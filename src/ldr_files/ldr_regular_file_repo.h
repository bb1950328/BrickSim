#ifndef BRICKSIM_LDR_REGULAR_FILE_REPO_H
#define BRICKSIM_LDR_REGULAR_FILE_REPO_H

#include "ldr_file_repo.h"

namespace ldr_file_repo {
    class LdrRegularFileRepo: public LdrFileRepo {
    public:
        explicit LdrRegularFileRepo(const std::filesystem::path &basePath);
        static bool isValidBasePath(const std::filesystem::path& basePath);
        std::vector<std::string> listAllFileNames(float *progress) override;
        virtual ~LdrRegularFileRepo() override;
        std::string getLibraryFileContent(LdrFileType type, std::string name) override;
        std::string getLibraryFileContent(std::string nameRelativeToRoot) override;
    };
}
#endif //BRICKSIM_LDR_REGULAR_FILE_REPO_H
