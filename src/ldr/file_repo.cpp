#include "file_repo.h"
#include "../config.h"
#include "../db.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include "file_reader.h"
#include "regular_file_repo.h"
#include "zip_file_repo.h"
#include <palanteer.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <zip.h>

namespace bricksim::ldr::file_repo {
    const char* PSEUDO_CATEGORY_SUBPART = "__SUBPART";
    const char* PSEUDO_CATEGORY_PRIMITIVE = "__PRIMITIVE";
    const char* PSEUDO_CATEGORY_MODEL = "__MODEL";
    const char* PSEUDO_CATEGORY_HIDDEN_PART = "__HIDDEN_PART";
    const char* PSEUDO_CATEGORY_BINARY_FILE = "__BINARY_FILE";
    const char* const PSEUDO_CATEGORIES[] = {PSEUDO_CATEGORY_SUBPART, PSEUDO_CATEGORY_PRIMITIVE, PSEUDO_CATEGORY_MODEL, PSEUDO_CATEGORY_HIDDEN_PART, PSEUDO_CATEGORY_BINARY_FILE};

    const char* const PART_SEARCH_PREFIXES[] = {"parts/", "p/", "models/", ""};

    namespace {
        std::unique_ptr<FileRepo> currentRepo = nullptr;

        std::string getContentOfIoFile(const std::filesystem::path& path) {
            char pw[]{0x53, 0x4C, 0x42, 70, 0x30 - 0x20 - 12, 0x19 - 15, 30 - 0x20, 60 - 0x20 - 24, 123 - 0x7B};
            for (char i = 0; pw[i] != 0; ++i) {
                pw[i] += 0x20 + 3 * i;
            }

            int errCode;
            struct zip* zArchive = zip_open(path.string().c_str(), 0, &errCode);
            if (zArchive == nullptr) {
                char errMessageBuffer[100];
                zip_error_to_str(errMessageBuffer, sizeof(errMessageBuffer), errCode, errno);//todo this function is deprecated
                throw std::invalid_argument(std::string("can't open .io file! Error ") + std::to_string(errCode) + ": " + std::string(errMessageBuffer));
            }

            struct zip_stat fileStat {};
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
    FileRepo& get() {
        if (currentRepo) {
            return *currentRepo;
        }
        throw std::invalid_argument("repo not initialized yet");
    }

    bool checkLdrawLibraryLocation() {
        static auto found = false;
        if (!found) {
            const auto& pathFromConfig = util::extendHomeDirPath(config::get(config::LDRAW_PARTS_LIBRARY));
            auto strPath = pathFromConfig.string();
            if (tryToInitializeWithLibraryPath(pathFromConfig)) {
                found = true;
            } else if (stringutil::endsWith(strPath, ".zip")) {
                auto zipEndingRemoved = strPath.substr(0, strPath.size() - 4);
                if (tryToInitializeWithLibraryPath(zipEndingRemoved)) {
                    config::set(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(zipEndingRemoved));
                    found = true;
                }
            } else if (tryToInitializeWithLibraryPath(strPath + ".zip")) {
                config::set(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(strPath + ".zip"));
                found = true;
            }
        }
        return found;
    }

    bool tryToInitializeWithLibraryPath(const std::filesystem::path& path) {
        if (std::filesystem::is_regular_file(path) && ZipFileRepo::isValidBasePath(path)) {
            currentRepo = std::unique_ptr<FileRepo>(dynamic_cast<FileRepo*>(new ZipFileRepo(path)));
        } else if (std::filesystem::is_directory(path) && RegularFileRepo::isValidBasePath(path)) {
            currentRepo = std::unique_ptr<FileRepo>(dynamic_cast<FileRepo*>(new RegularFileRepo(path)));
        }
        return currentRepo != nullptr;
    }

    LibraryType getLibraryType(const std::filesystem::path& path) {
        if (std::filesystem::is_regular_file(path) && ZipFileRepo::isValidBasePath(path)) {
            return LibraryType::ZIP;
        } else if (std::filesystem::is_directory(path) && RegularFileRepo::isValidBasePath(path)) {
            return LibraryType::DIRECTORY;
        }
        return LibraryType::INVALID;
    }

    FileRepo::FileRepo(std::filesystem::path basePath) :
        basePath(std::move(basePath)) {}

    std::shared_ptr<File> FileRepo::getFile(const std::string& name) {
        plFunction();
        auto it = ldrFiles.find(stringutil::asLower(name));
        if (it != ldrFiles.end()) {
            return it->second.second;
        } else {
            if (db::fileList::getSize() == 0) {
                spdlog::warn("FileRepo not initialized, but getFile() called. calling initialize now. this shouldn't happen.");
                float progress;
                initialize(&progress);
            }
            auto filenameWithForwardSlash = stringutil::replaceChar(name, '\\', '/');
            for (const auto& prefix: PART_SEARCH_PREFIXES) {
                auto entryOpt = db::fileList::findFile(prefix + filenameWithForwardSlash);
                if (entryOpt.has_value()) {
                    ldr::FileType type;
                    if (entryOpt->category == PSEUDO_CATEGORY_SUBPART) {
                        type = SUBPART;
                    } else if (entryOpt->category == PSEUDO_CATEGORY_PRIMITIVE) {
                        type = PRIMITIVE;
                    } else if (entryOpt->category == PSEUDO_CATEGORY_MODEL) {
                        type = MODEL;
                    } else {
                        type = PART;
                    }
                    return addLdrFileWithContent(entryOpt->name, type, getLibraryLdrFileContent(type, entryOpt->name));
                }
            }

            //at this point the file must be outside of the parts library
            auto fullPath = util::extendHomeDirPath(name);
            if (fullPath.extension() == ".io") {
                return addLdrFileWithContent(name, MODEL, getContentOfIoFile(fullPath));
            }
            return addLdrFileWithContent(name, MODEL, util::readFileToString(fullPath));
        }
    }

    std::shared_ptr<BinaryFile> FileRepo::getBinaryFile(const std::string& name, const BinaryFileSearchPath searchPath) {
        auto it = binaryFiles.find(name);
        if (it != binaryFiles.end()) {
            return it->second;
        }
        if (db::fileList::getSize() == 0) {
            spdlog::warn("FileRepo not initialized, but getFile() called. calling initialize now. this shouldn't happen.");
            float progress;
            initialize(&progress);
        }
        auto filenameWithForwardSlash = stringutil::replaceChar(name, '\\', '/');
        std::string nameInLibrary;
        const auto prePrefixes = searchPath == TEXMAP
                                         ? std::vector<std::string>({"textures/", ""})
                                         : std::vector<std::string>({""});
        for (const auto& prePrefix: prePrefixes) {
            for (const auto& prefix: PART_SEARCH_PREFIXES) {
                auto fullName = prefix + prePrefix;
                fullName += filenameWithForwardSlash;
                auto entryOpt = db::fileList::findFile(fullName);
                if (entryOpt.has_value()) {
                    nameInLibrary = entryOpt->name;
                    break;
                }
            }
            if (!nameInLibrary.empty()) {
                break;
            }
        }

        if (!nameInLibrary.empty()) {
            return addBinaryFileWithContent(nameInLibrary, getLibraryBinaryFileContent(nameInLibrary));
        }

        auto nameAsPath = std::filesystem::path(name);
        if (std::filesystem::exists(nameAsPath)) {
            return addBinaryFileWithContent(name, std::make_shared<BinaryFile>(nameAsPath));
        }

        return nullptr;
    }

    std::string FileRepo::readFileFromFilesystem(const std::filesystem::path& path) {
        return util::readFileToString(path);
    }

    std::shared_ptr<File> FileRepo::addLdrFileWithContent(const std::string& name, ldr::FileType type, const std::string& content) {
        auto readResults = readComplexFile(name, content, type);
        {
            plLockWait("FileRepo::ldrFilesMtx");
            std::lock_guard<std::mutex> lg(ldrFilesMtx);
            plLockScopeState("FileRepo::ldrFilesMtx", true);
            for (const auto& newFile: readResults) {
                ldrFiles.emplace(stringutil::asLower(newFile.first), std::make_pair(newFile.second->metaInfo.type, newFile.second));
            }
        }

        return readResults[name];
    }

    std::shared_ptr<BinaryFile> FileRepo::addBinaryFileWithContent(const std::string& name, const std::shared_ptr<BinaryFile>& file) {
        plLockWait("FileRepo::binaryFilesMtx");
        std::lock_guard<std::mutex> lg(binaryFilesMtx);
        plLockScopeState("FileRepo::binaryFilesMtx", true);
        return binaryFiles.emplace(name, file).first->second;
    }

    std::filesystem::path& FileRepo::getBasePath() {
        return basePath;
    }

    bool FileRepo::shouldFileBeSavedInList(const std::string& filename) {
        return (isLdrFilename(filename) || isBinaryFilename(filename))
               && (filename.starts_with("parts/")
                   || filename.starts_with("p/")
                   || filename.starts_with("models/"));
    }

    bool FileRepo::isLdrFilename(const std::string& filename) {
        return filename.ends_with(".dat")
               || filename.ends_with(".ldr")
               || filename.ends_with(".mpd");
    }

    bool FileRepo::isBinaryFilename(const std::string& filename) {
        return filename.ends_with(".png")
               || filename.ends_with(".jpg");
    }

    std::string FileRepo::getPathRelativeToBase(ldr::FileType type, const std::string& name) {
        switch (type) {
            case MODEL: return "models/" + name;
            case MPD_SUBFILE: throw std::invalid_argument("mpd subfile usually not in ldraw directory");
            case PART:
            case SUBPART: return "parts/" + name;
            case PRIMITIVE: return "p/" + name;
            default: return name;
        }
    }

    std::pair<ldr::FileType, std::string> FileRepo::getTypeAndNameFromPathRelativeToBase(const std::string& pathRelativeToBase) {
        if (stringutil::startsWith(pathRelativeToBase, "parts/s/")) {
            return {ldr::FileType::SUBPART, pathRelativeToBase.substr(6)};//not 8 because "s/" should be kept
        } else if (stringutil::startsWith(pathRelativeToBase, "parts/")) {
            return {ldr::FileType::PART, pathRelativeToBase.substr(6)};
        } else if (stringutil::startsWith(pathRelativeToBase, "p/")) {
            return {ldr::FileType::PRIMITIVE, pathRelativeToBase.substr(2)};
        } else if (stringutil::startsWith(pathRelativeToBase, "models/")) {
            return {ldr::FileType::MODEL, pathRelativeToBase.substr(7)};
        }
        return {ldr::FileType::MODEL, pathRelativeToBase};
    }

    void FileRepo::initialize(float* progress) {
        if (db::fileList::getSize() == 0) {
            spdlog::info("FileRepo: file list in db is empty, going to fill it");
            auto before = std::chrono::high_resolution_clock::now();

            auto fileNames = listAllFileNames(progress);
            const auto numFiles = fileNames.size();
            const auto numCores = std::thread::hardware_concurrency();
            const auto filesPerThread = numFiles / numCores;
            std::vector<std::thread> threads;
            for (int threadNum = 0; threadNum < numCores; ++threadNum) {
                const auto iStart = threadNum * filesPerThread;                                    //inclusive
                const auto iEnd = (threadNum == numCores - 1) ? numFiles : iStart + filesPerThread;//exclusive
                threads.emplace_back([this, iStart, iEnd, &fileNames, progress, threadNum]() {
#ifdef USE_PL
                    std::string threadName = "FileList filler #" + std::to_string(threadNum);
                    plDeclareThreadDyn(threadName.c_str());
#endif
                    std::vector<db::fileList::Entry> entries;
                    for (auto fileName = fileNames.cbegin() + iStart; fileName < fileNames.cbegin() + iEnd; ++fileName) {
                        auto [type, name] = getTypeAndNameFromPathRelativeToBase(*fileName);
                        if (isBinaryFilename(name)) {
                            entries.push_back({*fileName, name, PSEUDO_CATEGORY_BINARY_FILE});
                        } else {
                            auto ldrFile = addLdrFileWithContent(name, type, getLibraryLdrFileContent(*fileName));

                            std::string category;
                            if (type == ldr::FileType::PART) {
                                char& firstChar = ldrFile->metaInfo.title[0];
                                if ((firstChar == '~' && ldrFile->metaInfo.title[1] != '|') || firstChar == '=' || firstChar == '_') {
                                    category = PSEUDO_CATEGORY_HIDDEN_PART;
                                } else {
                                    category = ldrFile->metaInfo.getCategory();
                                }
                            } else if (type == ldr::FileType::SUBPART) {
                                category = PSEUDO_CATEGORY_SUBPART;
                            } else if (type == ldr::FileType::PRIMITIVE) {
                                category = PSEUDO_CATEGORY_PRIMITIVE;
                            } else if (type == ldr::FileType::MODEL) {
                                category = PSEUDO_CATEGORY_MODEL;
                            }
                            entries.push_back({name, ldrFile->metaInfo.title, category});
                        }
                        if (iStart == 0) {
                            *progress = 0.4f * entries.size() / iEnd + 0.5f;
                        }
                    }
                    db::fileList::put(entries);
                    if (iStart == 0) {
                        *progress = 1.0f;
                    }
                });
            }

            for (auto& t: threads) {
                t.join();
            }

            auto after = std::chrono::high_resolution_clock::now();
            auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0;

            if (numFiles != db::fileList::getSize()) {
                spdlog::error("had {} fileNames, but only {} are in db", numFiles, db::fileList::getSize());
            }
            spdlog::info("filled fileList in {} ms using {} threads. Size: {}", durationMs, numCores, numFiles);
        }
    }

    oset_t<std::string> FileRepo::getAllCategories() {
        static oset_t<std::string> result;
        if (result.empty()) {
            result = db::fileList::getAllCategories();
            for (const auto& pseudoCategory: PSEUDO_CATEGORIES) {
                result.erase(pseudoCategory);
            }
        }
        return result;
    }

    oset_t<std::shared_ptr<File>> FileRepo::getAllFilesOfCategory(const std::string& categoryName) {
        auto it = partsByCategory.find(categoryName);
        if (it == partsByCategory.end()) {
            const auto& fileNames = db::fileList::getAllPartsForCategory(categoryName);
            oset_t<std::shared_ptr<File>> result;
            for (const auto& fileName: fileNames) {
                result.insert(getFile(fileName));
            }
            partsByCategory.emplace(categoryName, result);
            return result;
        }
        return it->second;
    }

    bool FileRepo::areAllPartsLoaded() {
        return getAllCategories().size() == partsByCategory.size();
    }

    omap_t<std::string, oset_t<std::shared_ptr<File>>> FileRepo::getAllPartsGroupedByCategory() {
        if (!areAllPartsLoaded()) {
            for (const auto& ca: getAllCategories()) {
                if (partsByCategory.find(ca) == partsByCategory.end()) {
                    getAllFilesOfCategory(ca);
                }
            }
        }

        return partsByCategory;
    }

    omap_t<std::string, oset_t<std::shared_ptr<File>>> FileRepo::getLoadedPartsGroupedByCategory() {
        return partsByCategory;
    }

    void FileRepo::cleanup() {
        //ldrFiles.clear();
        //partsByCategory.clear();
    }

    bool FileRepo::hasFileCached(const std::string& name) {
        return ldrFiles.find(name) != ldrFiles.end();
    }

    void FileRepo::changeFileName(std::shared_ptr<File>& file, const std::string& newName) {
        auto it = ldrFiles.find(file->metaInfo.name);
        if (it == ldrFiles.end()) {
            it = ldrFiles.begin();
            while (it != ldrFiles.end() && it->second.second != file) {
                ++it;
            }
        }
        it->second.second->metaInfo.name = newName;
        ldrFiles.emplace(newName, it->second);
        ldrFiles.erase(it);
    }

    FileRepo::~FileRepo() = default;
}
