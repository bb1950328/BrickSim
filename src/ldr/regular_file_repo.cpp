#include "regular_file_repo.h"
#include <spdlog/spdlog.h>

namespace bricksim::ldr::file_repo {
    bool RegularFileRepo::isValidBasePath(const std::filesystem::path& basePath) {
        if (!std::filesystem::exists(basePath)
            || !std::filesystem::is_directory(basePath)) {
            spdlog::warn("{} not found or not a directory", basePath.string());
            return false;
        }
        if (!std::filesystem::exists(basePath / constants::LDRAW_CONFIG_FILE_NAME)) {
            spdlog::warn("{} not found in {}, therefore it's not a valid ldraw library directory", constants::LDRAW_CONFIG_FILE_NAME, basePath.string());
            return false;
        }
        return true;
    }

    std::vector<std::string> RegularFileRepo::listAllFileNames(std::function<void(float)> progress) {
        std::vector<std::string> files;
        for (const auto& entry: std::filesystem::recursive_directory_iterator(basePath)) {
            auto path = util::withoutBasePath(entry.path(), basePath).string();
            auto pathWithForwardSlash = stringutil::replaceChar(path, '\\', '/');

            if (shouldFileBeSavedInList(pathWithForwardSlash)) {
                files.push_back(pathWithForwardSlash);
                progress(std::min(1.f, .5f * static_cast<float>(files.size()) / ESTIMATE_PART_LIBRARY_FILE_COUNT));
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
    void RegularFileRepo::updateLibraryFilesImpl(const std::filesystem::path& updatedFileDirectory, std::function<void(int)> progress) {
        std::filesystem::copy(updatedFileDirectory, basePath, std::filesystem::copy_options::update_existing|std::filesystem::copy_options::recursive);
    }
    bool RegularFileRepo::replaceLibraryFilesDirectlyFromZip() {
        return false;
    }
    void RegularFileRepo::replaceLibraryFilesImpl(const std::filesystem::path& replacementFileOrDirectory, std::function<void(int)> progress) {
        if (!std::filesystem::is_directory(replacementFileOrDirectory)) {
            throw std::invalid_argument("expected a directory as replacement path");
        }
        std::filesystem::path tmpPath = basePath.string()+"_old";
        std::filesystem::rename(basePath, tmpPath);
        std::filesystem::copy(replacementFileOrDirectory, basePath, std::filesystem::copy_options::recursive);
        std::filesystem::remove_all(tmpPath);
    }
}
