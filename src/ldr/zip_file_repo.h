#pragma once

#include "file_repo.h"
#include <mutex>
#include <zip.h>

namespace bricksim::ldr::file_repo {
    class ZipFileRepo : public FileRepo {
    public:
        explicit ZipFileRepo(const std::filesystem::path& basePath);
        static bool isValidBasePath(const std::filesystem::path& basePath);
        std::vector<std::string> listAllFileNames(std::function<void(float)> progress) override;
        virtual ~ZipFileRepo();
        std::string getLibraryLdrFileContent(ldr::FileType type, const std::string& name) override;
        std::string getLibraryLdrFileContent(const std::string& nameRelativeToRoot) override;
        std::shared_ptr<BinaryFile> getLibraryBinaryFileContent(const std::string& nameRelativeToRoot) override;
    protected:
        void updateLibraryFilesImpl(const std::filesystem::path& updatedFileDirectory, std::function<void(int)> progress) override;

    private:
        struct zip* zipArchive;
        std::string rootFolderName;
        std::mutex libzipLock;
        static std::string getZipRootFolder(zip_t* archive);//including / at the end
        std::pair<struct zip_stat, zip_file_t*> openFileByName(const std::string& nameRelativeToRoot);
    };
}
