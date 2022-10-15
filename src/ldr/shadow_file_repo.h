#pragma once

#include "zip.h"
#include <filesystem>
#include <mutex>
#include <optional>

namespace bricksim::ldr::file_repo {
    class ShadowFileRepo {
    public:
        const std::filesystem::path basePath;
        explicit ShadowFileRepo(const std::filesystem::path& basePath);
        virtual std::optional<std::string> getContent(std::string pathRelativeToBase) = 0;
        virtual ~ShadowFileRepo();
    };

    class RegularShadowFileRepo : public ShadowFileRepo {
    public:
        explicit RegularShadowFileRepo(const std::filesystem::path& basePath);
        std::optional<std::string> getContent(std::string pathRelativeToBase) override;
    };

    class ZipShadowFileRepo : public ShadowFileRepo {
    public:
        static bool isValidZip(const std::filesystem::path& candidatePath);
        explicit ZipShadowFileRepo(const std::filesystem::path& basePath);
        std::optional<std::string> getContent(std::string pathRelativeToBase) override;
    protected:
        zip_t* archive;
        std::mutex libzipLock;
    };

    class EmptyShadowFileRepo : public ShadowFileRepo {
    public:
        explicit EmptyShadowFileRepo();
        std::optional<std::string> getContent(std::string pathRelativeToBase) override;
    };

    ShadowFileRepo& getShadowFileRepo();
    void initializeShadowFileRepo();
}