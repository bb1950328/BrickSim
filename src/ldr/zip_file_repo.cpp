#include "zip_file_repo.h"
#include <spdlog/spdlog.h>
#include <zipint.h>

namespace bricksim::ldr::file_repo {
    bool ZipFileRepo::isValidBasePath(const std::filesystem::path& basePath) {
        if (!std::filesystem::exists(basePath)
            || !std::filesystem::is_regular_file(basePath)
            || ".zip" != basePath.filename().extension().string()) {
            spdlog::warn("{} is not a .zip file", basePath.string());
            return false;
        }
        int err;
        zip_t* za = zip_open(basePath.string().c_str(), ZIP_RDONLY, &err);
        bool valid;
        if (za == nullptr) {
            zip_error_t zipError;
            zip_error_init_with_code(&zipError, err);
            spdlog::warn("cannot open .zip file: {}", zip_error_strerror(&zipError));
            zip_error_fini(&zipError);
            valid = false;
        } else if (zip_get_num_entries(za, 0) > 0) {
            const std::string rootFolder = getZipRootFolder(za);
            const auto ldConfigPath = rootFolder + constants::LDRAW_CONFIG_FILE_NAME;
            if (zip_name_locate(za, ldConfigPath.c_str(), ZIP_FL_ENC_GUESS) == -1) {
                spdlog::warn("{} not in {}", constants::LDRAW_CONFIG_FILE_NAME, basePath.string());
                valid = false;
            } else {
                spdlog::debug("{} is a valid zip library.", basePath.string());
                valid = true;
            }
        } else {
            spdlog::warn("{} is an empty zip file", basePath.string());
            valid = false;
        }
        zip_close(za);
        return valid;
    }

    std::vector<std::string> ZipFileRepo::listAllFileNames(float* progress) {
        std::scoped_lock<std::mutex> lg(libzipLock);
        std::vector<std::string> result;
        struct zip_stat fileStat{};
        int nameCutOff = rootFolderName.size();
        auto numEntries = zip_get_num_entries(zipArchive, 0);
        for (zip_int64_t i = 0; i < numEntries; ++i) {
            zip_stat_index(zipArchive, i, 0, &fileStat);
            const std::string nameString(fileStat.name + nameCutOff);
            if (shouldFileBeSavedInList(nameString)) {
                result.emplace_back(nameString);
                *progress = std::min(1.0f, 0.5f * i / numEntries);
            }
        }
        return result;
    }

    ZipFileRepo::ZipFileRepo(const std::filesystem::path& basePath) :
        FileRepo(basePath) {
        if (!isValidBasePath(basePath)) {
            throw std::invalid_argument("invalid basePath: " + basePath.string());
        }
        int errorCode;
        zipArchive = zip_open(basePath.string().c_str(), ZIP_RDONLY, &errorCode);

        rootFolderName = getZipRootFolder(zipArchive);

        if (zipArchive == nullptr) {
            zip_error_t zipError;
            zip_error_init_with_code(&zipError, errorCode);
            spdlog::error("can't open zip library with path {}: {} {}", basePath.string(), errorCode, zip_error_strerror(&zipError));
            zip_error_fini(&zipError);
            return;
        }
    }

    ZipFileRepo::~ZipFileRepo() {
        zip_close(zipArchive);
    }

    std::string ZipFileRepo::getLibraryLdrFileContent(ldr::FileType type, const std::string& name) {
        return getLibraryLdrFileContent(getPathRelativeToBase(type, name));
    }

    std::string ZipFileRepo::getLibraryLdrFileContent(const std::string& nameRelativeToRoot) {
        std::scoped_lock<std::mutex> lg(libzipLock);
        auto [stat, file] = openFileByName(nameRelativeToRoot);

        if (file == nullptr) {
            return "";
        }

        std::string result;
        result.resize(stat.size);
        const uint64_t readBytes = zip_fread(file, &result[0], stat.size);
        if (readBytes != stat.size) {
            spdlog::warn("file {} in zip library has reported size of {} bytes, but only {} bytes read. error={}", nameRelativeToRoot, stat.size, readBytes, zip_error_strerror(&file->error));
            result.resize(std::max(static_cast<decltype(readBytes)>(0), readBytes));
        }

        zip_fclose(file);

        return result;
    }

    std::pair<struct zip_stat, zip_file_t*> ZipFileRepo::openFileByName(const std::string& nameRelativeToRoot) {
        struct zip_stat stat{};
        std::string entryName = rootFolderName + nameRelativeToRoot;
        //try to find it with case-sensitive first because it's faster
        auto found = zip_stat(zipArchive, entryName.c_str(), 0, &stat);
        if (found == -1) {
            //if not found with exact case, try again with case-insensitive
            found = zip_stat(zipArchive, entryName.c_str(), ZIP_FL_NOCASE, &stat);
        }
        if (found == -1) {
            spdlog::error("file {} not found in zip library", entryName);
        }
        zip_file_t* file = zip_fopen_index(zipArchive, stat.index, 0);
        if (file == nullptr) {
            spdlog::error("failed to open file {} in zip library: {}", entryName, zip_error_strerror(zip_get_error(zipArchive)));
        }
        return std::make_pair(stat, file);
    }

    std::string ZipFileRepo::getZipRootFolder(zip_t* archive) {
        struct zip_stat stat;// NOLINT(cppcoreguidelines-pro-type-member-init)
        zip_stat_index(archive, 0, 0, &stat);
        const auto endPtr = std::strchr(stat.name, '/');
        return {stat.name, static_cast<size_t>((endPtr - stat.name + 1))};
    }

    std::shared_ptr<BinaryFile> ZipFileRepo::getLibraryBinaryFileContent(const std::string& nameRelativeToRoot) {
        std::scoped_lock<std::mutex> lg(libzipLock);
        auto [stat, file] = openFileByName(nameRelativeToRoot);
        if (file == nullptr) {
            return nullptr;
        }

        auto result = std::make_shared<BinaryFile>(nameRelativeToRoot);
        result->data.resize(stat.size);
        const uint64_t readBytes = zip_fread(file, &result->data[0], stat.size);
        if (readBytes != stat.size) {
            spdlog::warn("file {} in zip library has reported size of {} bytes, but only {} bytes read", nameRelativeToRoot, stat.size, readBytes);
            result->data.resize(std::max(static_cast<decltype(readBytes)>(0), readBytes));
        }

        zip_fclose(file);

        return result;
    }
}
