#pragma once

#include "file_repo.h"

namespace bricksim::ldr::file_repo {
    class RegularFileRepo : public FileRepo {
    public:
        explicit RegularFileRepo(const std::filesystem::path& basePath);
        static bool isValidBasePath(const std::filesystem::path& basePath);
        std::vector<std::string> listAllFileNames(std::function<void(float)> progress) override;
        ~RegularFileRepo() override;
        std::string getLibraryLdrFileContent(ldr::FileType type, const std::string& name) override;
        std::string getLibraryLdrFileContent(const std::string& nameRelativeToRoot) override;
        std::shared_ptr<BinaryFile> getLibraryBinaryFileContent(const std::string& nameRelativeToRoot) override;
        bool replaceLibraryFilesDirectlyFromZip() override;

    protected:
        void updateLibraryFilesImpl(const std::filesystem::path& updatedFileDirectory, std::function<void(int)> progress) override;
        void replaceLibraryFilesImpl(const std::filesystem::path& replacementFileOrDirectory, std::function<void(int)> progress) override;
    };
}
