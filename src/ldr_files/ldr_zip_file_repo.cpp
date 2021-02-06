// ldr_zip_file_repository.cpp.c
// Created by bab21 on 01.02.21.
//

#include <spdlog/spdlog.h>
#include <zip.h>
#include "ldr_zip_file_repo.h"

namespace ldr_file_repo {
    bool LdrZipFileRepo::isValidBasePath(const std::filesystem::path& basePath) {
        if (!std::filesystem::exists(basePath)
            || !std::filesystem::is_regular_file(basePath)
            || ".zip" != basePath.filename().extension().string()) {
            spdlog::warn("{} is not a .zip file", basePath.string());
            return false;
        }
        int err;
        zip_t *za = zip_open(basePath.string().c_str(), 0, &err);
        bool valid;
        if (za == nullptr) {
            char errBuf[100];
            zip_error_to_str(errBuf, sizeof(errBuf), err, errno);
            spdlog::warn("cannot open .zip file: {}", errBuf);
            valid = false;
        } else if (zip_name_locate(za, "LDConfig.ldr", ZIP_FL_ENC_GUESS) == -1
                   && zip_name_locate(za, "ldraw/LDConfig.ldr", ZIP_FL_ENC_GUESS) == -1) {
            spdlog::warn("LDConfig.ldr not in {}", basePath.string());
            valid = false;
        } else {
            spdlog::debug("{} is a valid zip library.", basePath.string());
            valid = true;
        }
        zip_close(za);
        return valid;
    }

    std::vector<std::string> LdrZipFileRepo::listAllFileNames(float *progress) {
        std::vector<std::string> result;
        struct zip_stat fileStat{};
        for (zip_int64_t i = 0; i < zip_get_num_entries(zipArchive, 0); ++i) {
            zip_stat_index(zipArchive, i, 0, &fileStat);
            if (shouldFileBeSavedInList(fileStat.name)) {
                result.emplace_back(fileStat.name);
                *progress = std::min(1.0f, 0.99f * result.size() / ESTIMATE_PART_LIBRARY_FILE_COUNT);
            }
        }
        return result;
    }

    LdrZipFileRepo::LdrZipFileRepo(const std::filesystem::path &basePath) : LdrFileRepo(basePath) {
        if (!isValidBasePath(basePath)) {
            throw std::invalid_argument("invalid basePath: "+basePath.string());
        }
        int errorCode;
        zipArchive = zip_open(basePath.string().c_str(), 0, &errorCode);
        if (zipArchive== nullptr) {
            char errorMessage[100];
            zip_error_to_str(errorMessage, sizeof(errorMessage), errorCode, errno);
            spdlog::error("can't open zip library with path {}: {} {}", basePath.string(), errorCode, errorMessage);
            return;
        }
    }

    LdrZipFileRepo::~LdrZipFileRepo() {
        zip_close(zipArchive);
    }

    std::string LdrZipFileRepo::getLibraryFileContent(LdrFileType type, std::string name) {
        return getLibraryFileContent(getPathRelativeToBase(type, name));
    }

    std::string LdrZipFileRepo::getLibraryFileContent(std::string nameRelativeToRoot) {
        struct zip_stat stat{};
        zip_stat(zipArchive, nameRelativeToRoot.c_str(), ZIP_FL_NOCASE, &stat);
        auto file = zip_fopen_index(zipArchive, stat.index, 0);

        char* result = new char[stat.size + 1];
        zip_fread(file, result, stat.size);
        result[stat.size] = '\0';

        return result;
    }
}

