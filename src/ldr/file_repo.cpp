#include "file_repo.h"
#include "../config.h"
#include "../db.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include "file_reader.h"
#include "regular_file_repo.h"
#include "shadow_file_repo.h"
#include "zip_file_repo.h"
#include <palanteer.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <utility>
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

            std::string result;
            result.resize(fileStat.size + 1);
            zip_fread(modelFile, result.data(), fileStat.size);
            result[fileStat.size] = '\0';

            zip_fclose(modelFile);
            zip_close(zArchive);

            return result;
        }

        std::string getContentOfLdrFile(const std::filesystem::path& path) {
            return path.extension() == ".io"
                           ? getContentOfIoFile(path)
                           : util::readFileToString(path);
        }
    }

    bool isInitialized() {
        return currentRepo != nullptr;
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
            } else if (strPath.ends_with(".zip")) {
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

    std::shared_ptr<File> FileRepo::getFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name) {
        plFunction();
        auto nsFiles = ldrFiles[fileNamespace];
        auto it = nsFiles.find(stringutil::asLower(name));
        if (it != nsFiles.end()) {
            return it->second.second;
        }
        if (fileNamespace == nullptr) {
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
                        type = FileType::SUBPART;
                    } else if (entryOpt->category == PSEUDO_CATEGORY_PRIMITIVE) {
                        type = FileType::PRIMITIVE;
                    } else if (entryOpt->category == PSEUDO_CATEGORY_MODEL) {
                        type = FileType::MODEL;
                    } else {
                        type = FileType::PART;
                    }
                    const auto shadowContent = getShadowFileRepo().getContent(FileRepo::getPathRelativeToBase(type, entryOpt->name));
                    const auto realFileContent = getLibraryLdrFileContent(type, entryOpt->name);
                    return addLdrFileWithContent(nullptr, entryOpt->name, type, realFileContent, shadowContent);
                }
            }
            throw std::invalid_argument(fmt::format("no file named \"{}\" in the library namespace", name));
        }
        const auto extendedPath = util::extendHomeDirPath(name);
        if (extendedPath.is_absolute() && std::filesystem::exists(extendedPath)) {
            return addLdrFileWithContent(nullptr, name, FileType::MODEL, getContentOfLdrFile(extendedPath));
        }

        const auto finalPath = fileNamespace->searchPath / name;
        if (std::filesystem::exists(finalPath)) {
            return addLdrFileWithContent(fileNamespace, name, FileType::MODEL, getContentOfLdrFile(finalPath));
        }

        try {
            return getFile(nullptr, name);
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(fmt::format(R"(no file named "{}", neither in the namespace "{}" ({}) nor the library namespace)", name, fileNamespace->name, fileNamespace->searchPath.string()));
        }
    }

    std::shared_ptr<BinaryFile> FileRepo::getBinaryFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, const BinaryFileSearchPath searchPath) {
        auto nsFiles = binaryFiles[fileNamespace];
        auto it = nsFiles.find(name);
        if (it != nsFiles.end()) {
            return it->second;
        }

        if (fileNamespace == nullptr) {
            if (db::fileList::getSize() == 0) {
                spdlog::warn("FileRepo not initialized, but getFile() called. calling initialize now. this shouldn't happen.");
                float progress;
                initialize(&progress);
            }
            auto filenameWithForwardSlash = stringutil::replaceChar(name, '\\', '/');
            std::string nameInLibrary;
            const auto prePrefixes = searchPath == BinaryFileSearchPath::TEXMAP
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

            if (nameInLibrary.empty()) {
                throw std::invalid_argument(fmt::format("no binary file names \"{}\" in library", name));
            } else {
                return addBinaryFileWithContent(nullptr, nameInLibrary, getLibraryBinaryFileContent(nameInLibrary));
            }
        }

        const auto extendedPath = util::extendHomeDirPath(name);
        if (extendedPath.is_absolute() && std::filesystem::exists(extendedPath)) {
            return addBinaryFileWithContent(nullptr, name, std::make_shared<BinaryFile>(extendedPath));
        }

        const auto finalPath = fileNamespace->searchPath / name;
        if (std::filesystem::exists(finalPath)) {
            return addBinaryFileWithContent(fileNamespace, name, std::make_shared<BinaryFile>(finalPath));
        }

        try {
            return getBinaryFile(nullptr, name);
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(fmt::format(R"(no file named "{}", neither in the namespace "{}" ({}) nor the library namespace)", name, fileNamespace->name, fileNamespace->searchPath.string()));
        }
    }

    std::shared_ptr<File> FileRepo::addLdrFileWithContent(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, FileType type, const std::string& content, const std::optional<std::string>& shadowContent) {
        auto readResults = readComplexFile(fileNamespace, name, type, content, shadowContent);
        {
            plLockWait("FileRepo::ldrFilesMtx");
            std::scoped_lock<std::mutex> lg(ldrFilesMtx);
            plLockScopeState("FileRepo::ldrFilesMtx", true);
            for (const auto& newFile: readResults) {
                ldrFiles[fileNamespace].emplace(stringutil::asLower(newFile.first), std::make_pair(newFile.second->metaInfo.type, newFile.second));
            }
        }

        return readResults[name];
    }

    std::shared_ptr<BinaryFile> FileRepo::addBinaryFileWithContent(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, const std::shared_ptr<BinaryFile>& file) {
        plLockWait("FileRepo::binaryFilesMtx");
        std::scoped_lock<std::mutex> lg(binaryFilesMtx);
        plLockScopeState("FileRepo::binaryFilesMtx", true);
        return binaryFiles[fileNamespace].emplace(name, file).first->second;
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
            case FileType::MODEL: return "models/" + name;
            case FileType::MPD_SUBFILE: throw std::invalid_argument("mpd subfile usually not in ldraw directory");
            case FileType::PART:
            case FileType::SUBPART: return "parts/" + name;
            case FileType::PRIMITIVE: return "p/" + name;
            default: return name;
        }
    }

    std::pair<ldr::FileType, std::string> FileRepo::getTypeAndNameFromPathRelativeToBase(const std::string& pathRelativeToBase) {
        if (pathRelativeToBase.starts_with("parts/s/")) {
            return {ldr::FileType::SUBPART, pathRelativeToBase.substr(6)};//not 8 because "s/" should be kept
        } else if (pathRelativeToBase.starts_with("parts/")) {
            return {ldr::FileType::PART, pathRelativeToBase.substr(6)};
        } else if (pathRelativeToBase.starts_with("p/")) {
            return {ldr::FileType::PRIMITIVE, pathRelativeToBase.substr(2)};
        } else if (pathRelativeToBase.starts_with("models/")) {
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
                const long iStart = threadNum * filesPerThread;                                    //inclusive
                const long iEnd = (threadNum == numCores - 1) ? numFiles : iStart + filesPerThread;//exclusive
                threads.emplace_back([this, iStart, iEnd, &fileNames, progress]() {
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
                            auto ldrFile = addLdrFileWithContent(nullptr, name, type, getLibraryLdrFileContent(*fileName));

                            std::string category;
                            if (type == ldr::FileType::PART) {
                                const char& firstChar = ldrFile->metaInfo.title[0];
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
                            *progress = .4f * static_cast<float>(entries.size()) / static_cast<float>(iEnd) + .5f;
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
            auto durationMs = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(after - before).count()) / 1000.0;

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
                result.insert(getFile(nullptr, fileName));
            }
            partsByCategory.try_emplace(categoryName, result);
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
                if (!partsByCategory.contains(ca)) {
                    getAllFilesOfCategory(ca);
                }
            }
        }

        return partsByCategory;
    }

    omap_t<std::string, oset_t<std::shared_ptr<File>>> FileRepo::getLoadedPartsGroupedByCategory() const {
        return partsByCategory;
    }

    void FileRepo::cleanup() {
        //ldrFiles.clear();
        //partsByCategory.clear();
    }

    bool FileRepo::hasFileCached(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name) {
        auto nsFiles = ldrFiles[fileNamespace];
        return nsFiles.find(name) != nsFiles.end() || (fileNamespace != nullptr && hasFileCached(nullptr, name));
    }

    void FileRepo::changeFileName(const std::shared_ptr<FileNamespace>& fileNamespace, const std::shared_ptr<File>& file, const std::string& newName) {
        auto nsFiles = ldrFiles[fileNamespace];
        auto it = nsFiles.find(file->metaInfo.name);
        if (it == nsFiles.end()) {
            it = nsFiles.begin();
            while (it != nsFiles.end() && it->second.second != file) {
                ++it;
            }
        }
        it->second.second->metaInfo.name = newName;
        nsFiles.emplace(newName, it->second);
        nsFiles.erase(it);
    }

    std::shared_ptr<File> FileRepo::reloadFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name) {
        auto nsFiles = ldrFiles[fileNamespace];
        auto it = nsFiles.find(name);
        if (it != nsFiles.end()) {
            nsFiles.erase(it);
        } else {
            reloadFile(nullptr, name);
        }
        return getFile(fileNamespace, name);
    }
    std::shared_ptr<File> FileRepo::addLdrFileWithContent(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, FileType type, const std::string& content) {
        return addLdrFileWithContent(fileNamespace, name, type, content, {});
    }
    const uomap_t<std::shared_ptr<FileNamespace>, uomap_t<std::string, std::pair<FileType, std::shared_ptr<File>>>>& FileRepo::getAllFilesInMemory() const {
        return ldrFiles;
    }
    std::shared_ptr<FileNamespace> FileRepo::getNamespace(const std::string& name) {
        for (const auto& [key, map]: ldrFiles) {
            if (key->name == name) {
                return key;
            }
        }
        return nullptr;
    }

    FileRepo::~FileRepo() = default;

}
