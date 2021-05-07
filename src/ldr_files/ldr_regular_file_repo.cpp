#include <spdlog/spdlog.h>
#include "ldr_regular_file_repo.h"
#include "../db.h"

namespace ldr_file_repo {

    bool LdrRegularFileRepo::isValidBasePath(const std::filesystem::path& basePath) {
        if (!std::filesystem::exists(basePath)
            || !std::filesystem::is_directory(basePath)) {
            spdlog::warn("{} not found or not a directory", basePath.string());
            return false;
        }
        if (!std::filesystem::exists(basePath / "LDConfig.ldr")) {
            spdlog::warn("LDConfig.ldr not found in {}, therefore it's not a valid ldraw library directory", basePath.string());
            return false;
        }
        return true;
    }

    std::vector<std::string> LdrRegularFileRepo::listAllFileNames(float *progress) {
        std::vector<std::string> files;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(basePath)) {
            auto path = util::withoutBasePath(entry.path(), basePath).string();
            auto pathWithForwardSlash = util::replaceChar(path, '\\', '/');

            if (shouldFileBeSavedInList(pathWithForwardSlash)) {
                files.push_back(pathWithForwardSlash);
                *progress = std::min(1.0f, 0.5f * files.size() / ESTIMATE_PART_LIBRARY_FILE_COUNT);
            }
        }
        return files;
    }

    std::string LdrRegularFileRepo::getLibraryFileContent(LdrFileType type, std::string name) {
        return util::readFileToString(basePath / getPathRelativeToBase(type, name));
    }

    std::string LdrRegularFileRepo::getLibraryFileContent(std::string nameRelativeToRoot) {
        return util::readFileToString(basePath / nameRelativeToRoot);
    }

    LdrRegularFileRepo::~LdrRegularFileRepo() = default;

    LdrRegularFileRepo::LdrRegularFileRepo(const std::filesystem::path &basePath) : LdrFileRepo(basePath) {
        if (!isValidBasePath(basePath)) {
            throw std::invalid_argument("invalid basePath: "+basePath.string());
        }
    }
}
