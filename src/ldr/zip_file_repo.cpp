#include "zip_file_repo.h"
#include <spdlog/spdlog.h>

namespace bricksim::ldr::file_repo {
    bool ZipFileRepo::isValidBasePath(const std::filesystem::path& basePath) {
        if (!std::filesystem::exists(basePath)
            || !std::filesystem::is_regular_file(basePath)
            || ".zip" != basePath.filename().extension().string()) {
            spdlog::warn("{} is not a .zip file", basePath.string());
            return false;
        }
        int err;
        zip_t* za = zip_open(basePath.string().c_str(), 0, &err);
        bool valid;
        if (za == nullptr) {
            char errBuf[100];
            zip_error_to_str(errBuf, sizeof(errBuf), err, errno);
            spdlog::warn("cannot open .zip file: {}", errBuf);
            valid = false;
        } else if (zip_get_num_entries(za, 0) > 0) {
            const std::string rootFolder = getZipRootFolder(za);
            const auto ldConfigPath = rootFolder + "LDConfig.ldr";
            if (zip_name_locate(za, ldConfigPath.c_str(), ZIP_FL_ENC_GUESS) == -1) {
                spdlog::warn("LDConfig.ldr not in {}", basePath.string());
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
        std::lock_guard<std::mutex> lg(libzipLock);
        std::vector<std::string> result;
        struct zip_stat fileStat {};
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
        zipArchive = zip_open(basePath.string().c_str(), 0, &errorCode);

        rootFolderName = getZipRootFolder(zipArchive);

        if (zipArchive == nullptr) {
            char errorMessage[100];
            zip_error_to_str(errorMessage, sizeof(errorMessage), errorCode, errno);
            spdlog::error("can't open zip library with path {}: {} {}", basePath.string(), errorCode, errorMessage);
            return;
        }
    }

    ZipFileRepo::~ZipFileRepo() {
        zip_close(zipArchive);
    }

    std::string ZipFileRepo::getLibraryFileContent(ldr::FileType type, std::string name) {
        return getLibraryFileContent(getPathRelativeToBase(type, name));
    }

    std::string ZipFileRepo::getLibraryFileContent(std::string nameRelativeToRoot) {
        std::lock_guard<std::mutex> lg(libzipLock);
        struct zip_stat stat {};
        std::string entryName = rootFolderName + nameRelativeToRoot;

        //try to find it with case sensitive first because it's faster
        auto found = zip_stat(zipArchive, entryName.c_str(), 0, &stat);
        if (found == -1) {
            //if not found with exact case, try again with case insensitive
            found = zip_stat(zipArchive, entryName.c_str(), ZIP_FL_NOCASE, &stat);
        }
        if (found == -1) {
            spdlog::error("file {} not found in zip library", entryName);
        }
        auto file = zip_fopen_index(zipArchive, stat.index, 0);

        if (file == nullptr) {
            spdlog::error("failed to open file {} in zip library: {}", entryName, zip_error_strerror(zip_get_error(zipArchive)));
            return "";
        }

        std::string result;
        result.resize(stat.size);
        const auto readBytes = zip_fread(file, &result[0], stat.size);
        if (readBytes != stat.size) {
            spdlog::warn("file {} in zip library has reported size of {} bytes, but only {} bytes read", entryName, stat.size, readBytes);
            result.resize(std::max(static_cast<typeof(readBytes)>(0), readBytes));
        }

        zip_fclose(file);

        return result;
    }

    std::string ZipFileRepo::getZipRootFolder(zip_t* archive) {
        struct zip_stat stat;// NOLINT(cppcoreguidelines-pro-type-member-init)
        zip_stat_index(archive, 0, 0, &stat);
        const auto endPtr = std::strchr(stat.name, '/');
        return std::string(stat.name, (endPtr - stat.name + 1));
    }
}
