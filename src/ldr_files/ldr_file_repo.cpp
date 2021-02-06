// ldr_base_file_repository.cpp.c
// Created by bab21 on 01.02.21.
//

#include "ldr_file_repo.h"
#include "../db.h"
#include "ldr_regular_file_repo.h"
#include "../config.h"
#include "ldr_zip_file_repo.h"

#include <utility>
#include <fstream>
#include <zip.h>
#include <spdlog/spdlog.h>

namespace ldr_file_repo {
    const char* PSEUDO_CATEGORY_SUBPART = "__SUBPART";
    const char* PSEUDO_CATEGORY_PRIMITIVE = "__PRIMITIVE";
    const char* PSEUDO_CATEGORY_MODEL = "__MODEL";
    const char* PSEUDO_CATEGORY_HIDDEN_PART = "__HIDDEN_PART";
    const char* const PSEUDO_CATEGORIES[] = {PSEUDO_CATEGORY_SUBPART, PSEUDO_CATEGORY_PRIMITIVE, PSEUDO_CATEGORY_MODEL, PSEUDO_CATEGORY_HIDDEN_PART};
    const char* const PART_SEARCH_PREFIXES[] = {"parts/", "p/", "models/", ""};
    namespace {
        std::unique_ptr<LdrFileRepo> currentRepo = nullptr;

        std::string getContentOfIoFile(const std::filesystem::path& path) {
            char pw[]{0x53, 0x4C, 0x42, 70, 0x30 - 0x20 - 12, 0x19 - 15, 30 - 0x20, 60 - 0x20 - 24, 123 - 0x7B};
            for (char i = 0; pw[i] != 0; ++i) {
                pw[i] += 0x20 + 3 * i;
            }

            int errCode;
            struct zip *zArchive = zip_open(path.string().c_str(), 0, &errCode);
            if (zArchive == nullptr) {
                char errMessageBuffer[100];
                zip_error_to_str(errMessageBuffer, sizeof(errMessageBuffer), errCode, errno);//todo this function is deprecated
                throw std::invalid_argument(std::string("can't open .io file! Error ") + std::to_string(errCode) + ": " + std::string(errMessageBuffer));
            }

            //zip_set_default_password(zArchive, pw);
            
            struct zip_stat fileStat{};
            zip_stat(zArchive, "model.ldr", ZIP_FL_NOCASE, &fileStat);
            auto modelFile = zip_fopen_index_encrypted(zArchive, fileStat.index, ZIP_FL_NOCASE, pw);

            char* result = new char[fileStat.size + 1];
            zip_fread(modelFile, result, fileStat.size);
            result[fileStat.size] = '\0';

            zip_fclose(modelFile);
            zip_close(zArchive);

            return result;
        }
    }
    LdrFileRepo& get() {
        if (currentRepo) {
            return *currentRepo;
        }
        throw std::invalid_argument("repo not initialized yet");
    }

    bool checkLdrawLibraryLocation() {
        static auto found = false;
        if (!found) {
            const auto &pathFromConfig = util::extendHomeDirPath(config::getString(config::LDRAW_PARTS_LIBRARY));
            auto strPath = pathFromConfig.string();
            if (tryToInitializeWithLibraryPath(pathFromConfig)) {
                found = true;
            } else if (util::endsWith(strPath, ".zip")) {
                auto zipEndingRemoved = strPath.substr(0, strPath.size() - 4);
                if (tryToInitializeWithLibraryPath(zipEndingRemoved)) {
                    config::setString(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(zipEndingRemoved));
                    found = true;
                }
            } else if (tryToInitializeWithLibraryPath(strPath + ".zip")) {
                config::setString(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(strPath + ".zip"));
                found = true;
            }
        }
        return found;
    }

    bool tryToInitializeWithLibraryPath(const std::filesystem::path &path) {
        if (std::filesystem::is_regular_file(path) && LdrZipFileRepo::isValidBasePath(path)) {
            currentRepo = std::unique_ptr<LdrFileRepo>(dynamic_cast<LdrFileRepo *>(new LdrZipFileRepo(path)));
        } else if (std::filesystem::is_directory(path) && LdrRegularFileRepo::isValidBasePath(path)) {
            currentRepo = std::unique_ptr<LdrFileRepo>(dynamic_cast<LdrFileRepo *>(new LdrRegularFileRepo(path)));
        }
        return currentRepo != nullptr;
    }

    LibraryType getLibraryType(const std::filesystem::path &path) {
        if (std::filesystem::is_regular_file(path) && LdrZipFileRepo::isValidBasePath(path)) {
            return LibraryType::ZIP;
        } else if (std::filesystem::is_directory(path) && LdrRegularFileRepo::isValidBasePath(path)) {
            return LibraryType::DIRECTORY;
        }
        return LibraryType::INVALID;
    }

    LdrFileRepo::LdrFileRepo(std::filesystem::path basePath) : basePath(std::move(basePath)) {}

    std::shared_ptr<LdrFile> LdrFileRepo::getFile(const std::string& name) {
        auto it = files.find(util::asLower(name));
        if (it!=files.end()) {
            return it->second.second;
        } else {
            if (db::fileList::getSize() == 0) {
                spdlog::warn("LdrFileRepo not initialized, but getFile() called. calling initialize now. this shouldn't happen.");
                float progress;
                initialize(&progress);
            }
            auto filenameWithForwardSlash = util::replaceChar(name, '\\', '/');
            for (const auto &prefix : PART_SEARCH_PREFIXES) {
                auto entryOpt = db::fileList::findFile(prefix + filenameWithForwardSlash);
                if (entryOpt.has_value()) {
                    LdrFileType type;
                    if (entryOpt->category==PSEUDO_CATEGORY_SUBPART) {
                        type = SUBPART;
                    } else if (entryOpt->category==PSEUDO_CATEGORY_PRIMITIVE) {
                        type = PRIMITIVE;
                    } else if (entryOpt->category==PSEUDO_CATEGORY_MODEL) {
                        type = MODEL;
                    } else {
                        type = PART;
                    }
                    return addFileWithContent(entryOpt->name, type, getLibraryFileContent(type, entryOpt->name));
                }
            }

            //at this point the file must be outside of the parts library
            auto fullPath = util::extendHomeDirPath(name);
            if (fullPath.extension()==".io") {
                return addFileWithContent(name, MODEL, getContentOfIoFile(fullPath));
            }
            return addFileWithContent(name, MODEL, util::readFileToString(fullPath));
        }
    }

    std::string LdrFileRepo::readFileFromFilesystem(const std::filesystem::path& path) {
        return util::readFileToString(path);
    }

    std::shared_ptr<LdrFile> LdrFileRepo::addFileWithContent(const std::string &name, LdrFileType type, const std::string& content) {
        auto file = LdrFile::parseFile(type, name, content);
        files.emplace(util::asLower(name), std::make_pair(type, file));
        return file;
    }

    std::filesystem::path &LdrFileRepo::getBasePath() {
        return basePath;
    }

    bool LdrFileRepo::shouldFileBeSavedInList(const std::string &filename) {
        return (util::endsWith(filename, ".dat")
                || util::endsWith(filename, ".ldr")
                || util::endsWith(filename, ".mpd"))
               &&
               (util::startsWith(filename, "parts/s/")
                || util::startsWith(filename, "parts/")
                || util::startsWith(filename, "p/")
                || util::startsWith(filename, "models/"));
    }

    std::string LdrFileRepo::getPathRelativeToBase(LdrFileType type, const std::string &name) {
        switch (type) {
            case MODEL: return "models/"+name;
            case MPD_SUBFILE: throw std::invalid_argument("mpd subfile usually not in ldraw directory");
            case PART:
            case SUBPART:return "parts/"+name;
            case PRIMITIVE:return "p/"+name;
            default: return name;
        }
    }

    std::pair<LdrFileType, std::string> LdrFileRepo::getTypeAndNameFromPathRelativeToBase(const std::string &pathRelativeToBase) {
        if (util::startsWith(pathRelativeToBase, "parts/s/")) {
            return {LdrFileType::SUBPART, pathRelativeToBase.substr(6)};//not 8 because "s/" should be kept
        } else if (util::startsWith(pathRelativeToBase, "parts/")) {
            return {LdrFileType::PART, pathRelativeToBase.substr(6)};
        } else if (util::startsWith(pathRelativeToBase, "p/")) {
            return {LdrFileType::PRIMITIVE, pathRelativeToBase.substr(2)};
        } else if (util::startsWith(pathRelativeToBase, "models/")) {
            return {LdrFileType::MODEL, pathRelativeToBase.substr(7)};
        }
        return {LdrFileType::MODEL, pathRelativeToBase};
    }

    void LdrFileRepo::initialize(float *progress) {
        if (db::fileList::getSize()==0) {
            spdlog::info("LdrFileRepo: file list in db is empty, going to fill it");
            auto before = std::chrono::high_resolution_clock::now();
            std::vector<db::fileList::Entry> entries;
            for (const auto &fileName : listAllFileNames(progress)) {
                LdrFileType type;
                std::string name;
                std::tie(type, name) = getTypeAndNameFromPathRelativeToBase(fileName);
                auto ldrFile = addFileWithContent(name, type, getLibraryFileContent(fileName));


                std::string category;
                if (type == LdrFileType::PART) {
                    char &firstChar = ldrFile->metaInfo.title[0];
                    if ((firstChar == '~' && ldrFile->metaInfo.title[1] != '|') || firstChar == '=' || firstChar == '_') {
                        category = PSEUDO_CATEGORY_HIDDEN_PART;
                    } else {
                        category = ldrFile->metaInfo.getCategory();
                    }
                } else if (type == LdrFileType::SUBPART) {
                    category = PSEUDO_CATEGORY_SUBPART;
                } else if (type == LdrFileType::PRIMITIVE) {
                    category = PSEUDO_CATEGORY_PRIMITIVE;
                } else if (type == LdrFileType::MODEL) {
                    category = PSEUDO_CATEGORY_MODEL;
                }
                entries.push_back({name, ldrFile->metaInfo.title, category});
            }
            db::fileList::put(entries);
            auto after = std::chrono::high_resolution_clock::now();
            auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0;
            spdlog::info("filled fileList in {} ms. Size: {}", durationMs, db::fileList::getSize());
        }
    }

    std::set<std::string> LdrFileRepo::getAllCategories() {
        static std::set<std::string> result;
        if (result.empty()) {
            result = db::fileList::getAllCategories();
            for (const auto &pseudoCategory : PSEUDO_CATEGORIES) {
                result.erase(pseudoCategory);
            }
        }
        return result;
    }

    std::set<std::shared_ptr<LdrFile>> LdrFileRepo::getAllFilesOfCategory(const std::string &categoryName) {
        auto it = partsByCategory.find(categoryName);
        if (it == partsByCategory.end()) {
            const auto &fileNames = db::fileList::getAllPartsForCategory(categoryName);
            std::set<std::shared_ptr<LdrFile>> result;
            for (const auto &fileName : fileNames) {
                result.insert(getFile(fileName));
            }
            partsByCategory.emplace(categoryName, result);
            return result;
        }
        return it->second;
    }

    bool LdrFileRepo::areAllPartsLoaded() {
        return getAllCategories().size() == partsByCategory.size();
    }

    std::map<std::string, std::set<std::shared_ptr<LdrFile>>> LdrFileRepo::getAllPartsGroupedByCategory() {
        if (!areAllPartsLoaded()) {
            for (const auto &ca : getAllCategories()) {
                if (partsByCategory.find(ca) == partsByCategory.end()) {
                    getAllFilesOfCategory(ca);
                }
            }
        }

        return partsByCategory;
    }

    std::map<std::string, std::set<std::shared_ptr<LdrFile>>> LdrFileRepo::getLoadedPartsGroupedByCategory() {
        return partsByCategory;
    }

    LdrFileRepo::~LdrFileRepo() = default;
}




