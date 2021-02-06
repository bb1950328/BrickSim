// ldr_zip_file_repository.h
// Created by bab21 on 01.02.21.
//

#ifndef BRICKSIM_LDR_ZIP_FILE_REPO_H
#define BRICKSIM_LDR_ZIP_FILE_REPO_H

#include "ldr_file_repo.h"
#include <zip.h>

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
    };
}
#endif //BRICKSIM_LDR_ZIP_FILE_REPO_H
