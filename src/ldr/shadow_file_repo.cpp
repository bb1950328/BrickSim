#include "shadow_file_repo.h"
#include "../config/read.h"
#include "spdlog/spdlog.h"
#include "../helpers/util.h"

namespace bricksim::ldr::file_repo {
    namespace {
        std::unique_ptr<ShadowFileRepo> currentShadowRepo = nullptr;
    }
    ShadowFileRepo::ShadowFileRepo(const std::filesystem::path& basePath) :
        basePath(basePath) {
    }
    ShadowFileRepo::~ShadowFileRepo() = default;
    void initializeShadowFileRepo() {
        std::filesystem::path path = util::replaceSpecialPaths(config::get().ldraw.shadowLibraryLocation);
        if (std::filesystem::is_regular_file(path) && ZipShadowFileRepo::isValidZip(path)) {
            currentShadowRepo = std::make_unique<ZipShadowFileRepo>(path);
            spdlog::info("loaded shadow library from zip: {}", path.string());
        } else if (std::filesystem::is_directory(path)) {
            currentShadowRepo = std::make_unique<RegularShadowFileRepo>(path);
            spdlog::info("loaded shadow library from folder: {}", path.string());
        } else {
            spdlog::warn("shadow library path {} invalid. some features will not be available.", path.string());
            currentShadowRepo = std::make_unique<EmptyShadowFileRepo>();
        }
    }
    ShadowFileRepo& getShadowFileRepo() {
        if (currentShadowRepo) {
            return *currentShadowRepo;
        }
        throw std::invalid_argument("shadow file repo is not initialized yet");
    }
    RegularShadowFileRepo::RegularShadowFileRepo(const std::filesystem::path& basePath) :
        ShadowFileRepo(basePath) {
    }
    std::optional<std::string> RegularShadowFileRepo::getContent(const std::string& pathRelativeToBase) {
        const auto fullPath = basePath / pathRelativeToBase;
        if (std::filesystem::is_regular_file(fullPath)) {
            return util::readFileToString(fullPath);
        }
        return {};
    }
    ZipShadowFileRepo::ZipShadowFileRepo(const std::filesystem::path& basePath) :
        ShadowFileRepo(basePath) {
        int err;
        archive = zip_open(basePath.string().c_str(), ZIP_RDONLY, &err);
    }
    bool ZipShadowFileRepo::isValidZip(const std::filesystem::path& candidatePath) {
        int err;
        return zip_open(candidatePath.string().c_str(), ZIP_RDONLY, &err) != nullptr;
    }
    std::optional<std::string> ZipShadowFileRepo::getContent(const std::string& pathRelativeToBase) {
        std::scoped_lock<std::mutex> lg(libzipLock);
        struct zip_stat stat {};
        auto found = zip_stat(archive, pathRelativeToBase.c_str(), 0, &stat);
        if (found == -1) {
            found = zip_stat(archive, pathRelativeToBase.c_str(), ZIP_FL_NOCASE, &stat);
        }
        if (found == -1) {
            return {};
        }
        zip_file_t* file = zip_fopen_index(archive, stat.index, 0);
        std::string result;
        result.resize(stat.size);
        const uint64_t readBytes = zip_fread(file, &result[0], stat.size);
        if (readBytes != stat.size) {
            spdlog::warn("file {} in zip shadow library has reported size of {} bytes, but only {} bytes read.", pathRelativeToBase, stat.size, readBytes);
            result.resize(std::max(static_cast<decltype(readBytes)>(0), readBytes));
        }

        zip_fclose(file);
        return result;
    }

    EmptyShadowFileRepo::EmptyShadowFileRepo() :
        ShadowFileRepo("") {}
    std::optional<std::string> EmptyShadowFileRepo::getContent(const std::string& pathRelativeToBase) {
        return {};
    }
}
