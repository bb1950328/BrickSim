#pragma once

#include "file_repo.h"

namespace bricksim::ldr::file_repo {
    class RegularFileRepo : public FileRepo {
    public:
        explicit RegularFileRepo(const std::filesystem::path& basePath);
        static bool isValidBasePath(const std::filesystem::path& basePath);
        std::vector<std::string> listAllFileNames(float* progress) override;
        virtual ~RegularFileRepo() override;
        std::string getLibraryFileContent(ldr::FileType type, std::string name) override;
        std::string getLibraryFileContent(std::string nameRelativeToRoot) override;
    };
}
