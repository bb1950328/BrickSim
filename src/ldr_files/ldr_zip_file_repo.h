#ifndef BRICKSIM_LDR_ZIP_FILE_REPO_H
#define BRICKSIM_LDR_ZIP_FILE_REPO_H

#include "ldr_file_repo.h"
#include <zip.h>
#include <mutex>

namespace ldr_file_repo {
    class LdrZipFileRepo: public LdrFileRepo {
    public:
        explicit LdrZipFileRepo(const std::filesystem::path &basePath);
        static bool isValidBasePath(const std::filesystem::path& basePath);
        std::vector<std::string> listAllFileNames(float *progress) override;
        virtual ~LdrZipFileRepo();
        std::string getLibraryFileContent(LdrFileType type, std::string name) override;
        std::string getLibraryFileContent(std::string nameRelativeToRoot) override;
    private:
        struct zip* zipArchive;
        std::string rootFolderName;
        std::mutex libzipLock;
        static std::string getZipRootFolder(zip_t* archive);//including / at the end
    };
}
#endif //BRICKSIM_LDR_ZIP_FILE_REPO_H
