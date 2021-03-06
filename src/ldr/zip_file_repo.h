#pragma once

#include "file_repo.h"
#include <mutex>
#include <zip.h>

namespace bricksim::ldr::file_repo {
    class ZipFileRepo : public FileRepo {
    public:
        explicit ZipFileRepo(const std::filesystem::path& basePath);
        static bool isValidBasePath(const std::filesystem::path& basePath);
        std::vector<std::string> listAllFileNames(float* progress) override;
        virtual ~ZipFileRepo();
        std::string getLibraryFileContent(ldr::FileType type, std::string name) override;
        std::string getLibraryFileContent(std::string nameRelativeToRoot) override;

    private:
        struct zip* zipArchive;
        std::string rootFolderName;
        std::mutex libzipLock;
        static std::string getZipRootFolder(zip_t* archive);//including / at the end
    };
}
