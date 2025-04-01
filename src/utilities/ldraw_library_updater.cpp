#include "ldraw_library_updater.h"
#include "../helpers/util.h"
#include "../ldr/config.h"
#include "../ldr/file_repo.h"
#include "../ldr/zip_file_repo.h"
#include "pugixml.hpp"
#include <fstream>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <zip.h>

namespace bricksim::ldraw_library_updater {
    namespace {
        std::unique_ptr<UpdateState> state;

        static constexpr std::size_t ESTIMATED_UPDATE_FILE_SIZE = 45000;

        int updatesListDownloadProgress([[maybe_unused]] void* ptr, [[maybe_unused]] long downTotal, long downNow, [[maybe_unused]] long upTotal, [[maybe_unused]] long upNow) {
            if (state != nullptr) {
                state->initializingProgress = .1f + std::max(.9f, .9f * downNow / ESTIMATED_UPDATE_FILE_SIZE);
            }
            return 0;
        }
        Distribution parseDistribution(const pugi::xml_node& distNode) {
            const auto* releaseDate = distNode.child("release_date").child_value();
            const auto releaseDateParsed = std::strlen(releaseDate) == 10
                                                   ? stringutil::parseYYYY_MM_DD(std::string_view(releaseDate))
                                                   : std::chrono::year_month_day();
            const auto* releaseId = distNode.child("release_id").child_value();
            const auto* url = distNode.child("url").child_value();
            std::size_t size;
            sscanf(distNode.child("size").child_value(), "%zu", &size);
            const auto* md5hex = distNode.child("md5_fingerprint").child_value();
            std::array<std::byte, 16> md5{static_cast<std::byte>(0)};
            for (int i = 0; i < 16; ++i) {
                for (int j = 0; j < 2; ++j) {
                    const auto c = md5hex[i * 2 + j];
                    const std::byte nibble = static_cast<std::byte>(c <= '9' ? c - '0' : c - 'a' + 10);
                    md5[i] = (md5[i] << 4) | nibble;
                }
            }
            return {releaseId, releaseDateParsed, url, size, md5};
        }
    }

    UpdateState& getState() {
        if (state == nullptr) {
            state = std::make_unique<UpdateState>();
        }
        return *state;
    }
    void resetState() {
        state = nullptr;
    }

    UpdateFailedException::UpdateFailedException(const std::string& message, const std::source_location location) :
        TaskFailedException(message, location) {}

    void UpdateState::initialize() {
        if (step != Step::INACTIVE) {
            throw std::invalid_argument("wrong time to call this method");
        }
        step = Step::INITIALIZING;
        initializingProgress = .05f;
        findCurrentRelease();
        initializingProgress = .1f;
        readUpdatesList();
        initializingProgress = 1.f;
        step = Step::CHOOSE_ACTION;
    }
    void UpdateState::findCurrentRelease() {
        currentReleaseDate = stringutil::parseYYYY_MM_DD(ldr::getConfig().getUpdateDate());
        currentReleaseId = ldr::file_repo::get().getVersion();
    }
    void UpdateState::readUpdatesList() {
        const auto [statusCode, xmlContent] = util::requestGET(constants::LDRAW_LIBRARY_UPDATES_XML_URL, false, 0, updatesListDownloadProgress);
        if (statusCode < 200 | statusCode >= 300) {
            throw UpdateFailedException(fmt::format("download of update info XML failed with status {}", statusCode));
        }
        pugi::xml_document doc;
        const auto parseResult = doc.load_buffer(xmlContent.c_str(), xmlContent.size());
        if (parseResult.status != pugi::status_ok) {
            throw UpdateFailedException(fmt::format("error while parsing update info XML at char {}. {}", parseResult.offset, parseResult.description()));
        }
        for (auto distNode = doc.root().first_child().child("distribution"); distNode; distNode = distNode.next_sibling("distribution")) {
            const auto* fileFormat = distNode.child("file_format").child_value();
            if (std::strcmp(fileFormat, "ZIP") == 0) {
                const auto* releaseType = distNode.child("release_type").child_value();
                const auto distribution = parseDistribution(distNode);

                if (std::strcmp(releaseType, "UPDATE") == 0) {
                    if (distribution.id > currentReleaseId) {
                        incrementalUpdates.push_back(distribution);
                    }
                } else if (std::strcmp(releaseType, "COMPLETE") == 0) {
                    completeDistribution = distribution;
                }
            }
        }
    }
    std::size_t UpdateState::getIncrementalUpdateTotalSize() const {
        return std::accumulate(incrementalUpdates.cbegin(),
                               incrementalUpdates.cend(),
                               std::size_t(0),
                               [](std::size_t x, const Distribution& dist) { return x + dist.size; });
    }
    void UpdateState::doIncrementalUpdate() {
        step = Step::UPDATE_INCREMENTAL;
        spdlog::info("starting incremental LDraw library update");
        const auto tmpDirectory = std::filesystem::temp_directory_path() / "BrickSimIncrementalUpdate";
        std::filesystem::create_directory(tmpDirectory);
        std::vector<std::filesystem::path> tmpZipFiles;
        for (int i = 0; i < incrementalUpdates.size(); ++i) {
            incrementalUpdateProgress.push_back(0);
            auto& currentStepProgress = incrementalUpdateProgress.back();
            currentStepProgress = .01f;

            const auto dist = incrementalUpdates[i];
            tmpZipFiles.push_back(tmpDirectory / (dist.id + ".zip"));
            const auto url = dist.url;
            const auto downloadResult = util::downloadFile(url, tmpZipFiles.back(), [&currentStepProgress](std::size_t dlTotal, std::size_t dlNow, std::size_t ulTotal, std::size_t ulNow) {
                currentStepProgress = dlTotal > 0 ? .5f * dlNow / dlTotal : .5f;
                return 0;
            });
            spdlog::debug("downloaded {} to {}", url, tmpZipFiles.back().string());
            if (downloadResult.httpCode < 200 || downloadResult.httpCode >= 300) {
                throw UpdateFailedException(fmt::format("cannot download {}. HTTP Code was {}", url, downloadResult.httpCode));
            }
            if (dist.size != downloadResult.contentLength) {
                throw UpdateFailedException(fmt::format("Content Length mismatch for {} (expected={}, actual={}", url, dist.size, downloadResult.contentLength));
            }
            const auto& expectedMD5 = dist.md5;
            if (!std::all_of(expectedMD5.cbegin(), expectedMD5.cend(), [](std::byte x) { return x == std::byte{0}; })) {
                const auto actualMD5 = util::md5(tmpZipFiles.back());
                if (expectedMD5 != actualMD5) {
                    throw UpdateFailedException(fmt::format("MD5 mismatch (expected={:02x}, actual={:02x}) for {}", fmt::join(expectedMD5, ""), fmt::join(actualMD5, ""), dist.url));
                }
            }
        }

        const auto mergedDirectory = tmpDirectory / "merged";
        std::filesystem::create_directory(mergedDirectory);
        uoset_t<std::string> extractedFiles;

        for (int i = incrementalUpdates.size() - 1; i >= 0; --i) {
            const auto tmpZipPath = tmpZipFiles[i];
            const auto distr = incrementalUpdates[i];
            auto& currentStepProgress = incrementalUpdateProgress[i];

            int err;
            const auto archive = std::unique_ptr<zip_t, decltype(&zip_close)>(zip_open(tmpZipPath.string().c_str(), ZIP_RDONLY, &err), zip_close);
            if (archive == nullptr) {
                zip_error_t zipError;
                zip_error_init_with_code(&zipError, err);
                auto msg = fmt::format("Cannot open .zip file: {} (downloaded from {})", zip_error_strerror(&zipError), distr.url);
                zip_error_fini(&zipError);
                throw UpdateFailedException(msg);
            }
            const auto numEntries = zip_get_num_entries(archive.get(), ZIP_FL_UNCHANGED);
            for (zip_int64_t j = 0; j < numEntries; ++j) {
                struct zip_stat stat;
                if (zip_stat_index(archive.get(), j, ZIP_FL_UNCHANGED, &stat) != 0) {
                    throw UpdateFailedException(fmt::format("Cannot stat {}-th file in .zip (downloaded from {})", j, distr.url));
                }
                const auto [_, notAlreadyExist] = extractedFiles.insert(stat.name);
                if (!notAlreadyExist) {
                    continue;
                }
                auto entryPath = mergedDirectory / stat.name;
                auto zfile = std::unique_ptr<zip_file_t, decltype(&zip_fclose)>(zip_fopen_index(archive.get(), j, ZIP_FL_UNCHANGED), zip_fclose);
                if (zfile == nullptr) {
                    throw UpdateFailedException(fmt::format("cannot open file {} inside .zip (downloaded from {})", stat.name, distr.url));
                }
                std::filesystem::create_directories(entryPath.parent_path());
                std::ofstream outfile(entryPath, std::ios::binary);
                if (!outfile) {
                    throw UpdateFailedException(fmt::format("cannot create temporary file {}", entryPath.string()));
                }

                std::array<char, 4096> buf;
                zip_int64_t bytesRead;
                while ((bytesRead = zip_fread(zfile.get(), buf.data(), buf.size())) > 0) {
                    outfile.write(buf.data(), bytesRead);
                }
            }
            spdlog::debug("extracted {} entries from {}", numEntries, tmpZipPath.string());
        }

        std::filesystem::path sourceDir;
        if (std::filesystem::is_regular_file(mergedDirectory / "ldraw" / constants::LDRAW_CONFIG_FILE_NAME)) {
            sourceDir = mergedDirectory / "ldraw";
        } else if (std::filesystem::is_regular_file(mergedDirectory / constants::LDRAW_CONFIG_FILE_NAME)) {
            sourceDir = mergedDirectory;
        } else {
            throw UpdateFailedException(fmt::format("cannot find {} in {} or {}", constants::LDRAW_CONFIG_FILE_NAME, (mergedDirectory / "ldraw").string(), mergedDirectory.string()));
        }

        auto progressFunc = [this](float progress) {
            const auto fraction = .5f * progress + .5f;
            std::fill(incrementalUpdateProgress.begin(), incrementalUpdateProgress.end(), fraction);
        };
        ldr::file_repo::get().updateLibraryFiles(sourceDir, progressFunc, extractedFiles.size());

        std::filesystem::remove_all(tmpDirectory);
        step = Step::FINISHED;
    }
    void UpdateState::doCompleteUpdate() {
        step = Step::UPDATE_COMPLETE;
        const auto tmpDirectory = std::filesystem::temp_directory_path() / "BrickSimCompleteUpdate";
        std::filesystem::create_directory(tmpDirectory);
        const auto tmpFile = tmpDirectory / "complete.zip";
        util::downloadFile(completeDistribution->url, tmpFile, [this](std::size_t dlTotal, std::size_t dlNow, std::size_t ulTotal, std::size_t ulNow) {
            completeUpdateProgress = dlTotal > 0 ? .5f * dlNow / dlTotal : 0.f;
            return 0;
        });

        const auto needsExtract = !ldr::file_repo::get().replaceLibraryFilesDirectlyFromZip();

        uint64_t numEntries;
        std::filesystem::path fileOrDirectoryForReplacement;
        {
            int err;
            const auto archive = std::unique_ptr<zip_t, decltype(&zip_close)>(zip_open(tmpFile.string().c_str(), ZIP_RDONLY, &err), zip_close);
            if (archive == nullptr) {
                zip_error_t zipError;
                zip_error_init_with_code(&zipError, err);
                auto msg = fmt::format("Cannot open .zip file: {} (downloaded from {})", zip_error_strerror(&zipError), completeDistribution->url);
                zip_error_fini(&zipError);
                throw UpdateFailedException(msg);
            }
            numEntries = zip_get_num_entries(archive.get(), 0);

            if (needsExtract) {
                const auto extractionDirectory = tmpDirectory / "extracted";
                for (zip_int64_t j = 0; j < numEntries; ++j) {
                    struct zip_stat stat;
                    if (zip_stat_index(archive.get(), j, ZIP_FL_UNCHANGED, &stat) != 0) {
                        throw UpdateFailedException(fmt::format("Cannot stat {}-th file in .zip (downloaded from {})", j, completeDistribution->url));
                    }
                    if (stat.size<=0) {
                        continue;
                    }
                    auto entryPath = extractionDirectory / stat.name;
                    auto zfile = std::unique_ptr<zip_file_t, decltype(&zip_fclose)>(zip_fopen_index(archive.get(), j, ZIP_FL_UNCHANGED), zip_fclose);
                    if (zfile == nullptr) {
                        throw UpdateFailedException(fmt::format("cannot open file {} inside .zip (downloaded from {})", stat.name, completeDistribution->url));
                    }
                    std::filesystem::create_directories(entryPath.parent_path());
                    std::ofstream outfile(entryPath, std::ios::binary);
                    if (!outfile) {
                        throw UpdateFailedException(fmt::format("cannot create temporary file {}", entryPath.string()));
                    }

                    std::array<char, 4096> buf;
                    zip_int64_t bytesRead;
                    while ((bytesRead = zip_fread(zfile.get(), buf.data(), buf.size())) > 0) {
                        outfile.write(buf.data(), bytesRead);
                    }
                }
                fileOrDirectoryForReplacement = std::filesystem::is_directory(extractionDirectory / "ldraw")
                                                        ? extractionDirectory / "ldraw"
                                                        : extractionDirectory;
            } else {
                fileOrDirectoryForReplacement = tmpFile;
            }
        }

        ldr::file_repo::get().replaceLibraryFiles(fileOrDirectoryForReplacement, [this](float progress){completeUpdateProgress = .5+.5*progress;}, numEntries);

        std::filesystem::remove_all(tmpDirectory);
        step = Step::FINISHED;
    }
}