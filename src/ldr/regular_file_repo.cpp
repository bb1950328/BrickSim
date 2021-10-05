#include "regular_file_repo.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include <spdlog/spdlog.h>

namespace bricksim::ldr::file_repo {

    bool RegularFileRepo::isValidBasePath(const std::filesystem::path& basePath) {
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

    std::vector<std::string> RegularFileRepo::listAllFileNames(float* progress) {
        std::vector<std::string> files;
        for (const auto& entry: std::filesystem::recursive_directory_iterator(basePath)) {
            auto path = util::withoutBasePath(entry.path(), basePath).string();
            auto pathWithForwardSlash = stringutil::replaceChar(path, '\\', '/');

            if (shouldFileBeSavedInList(pathWithForwardSlash)) {
                files.push_back(pathWithForwardSlash);
                *progress = std::min(1.0f, 0.5f * files.size() / ESTIMATE_PART_LIBRARY_FILE_COUNT);
            }
        }
        return files;
    }

    std::string RegularFileRepo::getLibraryLdrFileContent(ldr::FileType type, const std::string& name) {
        return util::readFileToString(basePath / getPathRelativeToBase(type, name));
    }

    std::string RegularFileRepo::getLibraryLdrFileContent(const std::string& nameRelativeToRoot) {
        return util::readFileToString(basePath / nameRelativeToRoot);
    }

    RegularFileRepo::~RegularFileRepo() = default;

    RegularFileRepo::RegularFileRepo(const std::filesystem::path& basePath) :
        FileRepo(basePath) {
        if (!isValidBasePath(basePath)) {
            throw std::invalid_argument("invalid basePath: " + basePath.string());
        }
    }

    std::shared_ptr<BinaryFile> RegularFileRepo::getLibraryBinaryFileContent(const std::string& nameRelativeToRoot) {
        return std::make_shared<BinaryFile>(basePath / nameRelativeToRoot);
    }
}
