// ldr_file_repository.cpp
// Created by bb1950328 on 06.10.20.
//

#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
#include "ldr_file_repository.h"
#include "config.h"
#include "helpers/zip_buffer.h"
#include "db.h"

namespace ldr_file_repo {
    namespace {
        std::map<std::string, std::pair<LdrFileType, LdrFile *>> files;
        std::filesystem::path ldrawDirectory;
        std::filesystem::path partsDirectory;
        std::filesystem::path subpartsDirectory;
        std::filesystem::path primitivesDirectory;
        std::filesystem::path modelsDirectory;

        zip_buffer::BufferedZip* zipLibrary;
        std::map<std::filesystem::path, std::string> otherFileCache;
        std::map<const std::string*, bool> otherFileCacheLock;
        std::mutex otherFileCacheLockMutex;

        bool isZipLibrary;

        //key is name lowercase, value is name in original case
        std::map<std::string, std::string> primitiveNames;
        std::map<std::string, std::string> subpartNames;
        std::map<std::string, std::string> partNames;
        std::map<std::string, std::string> modelNames;

        std::map<std::string, std::set<LdrFile *>> partsByCategory;

        const std::string PSEUDO_CATEGORY_SUBPART = "__SUBPART";
        const std::string PSEUDO_CATEGORY_PRIMITIVE = "__PRIMITIVE";
        const std::string PSEUDO_CATEGORY_MODEL = "__MODEL";
        const std::string PSEUDO_CATEGORY_HIDDEN_PART = "__HIDDEN_PART";
        const std::vector<std::string> PSEUDO_CATEGORIES{PSEUDO_CATEGORY_SUBPART, PSEUDO_CATEGORY_PRIMITIVE, PSEUDO_CATEGORY_MODEL, PSEUDO_CATEGORY_HIDDEN_PART};

        std::pair<LdrFileType, std::string> convertLDrawDirPathToFilenameAndType(const std::string &ldrawDirPath);

        LdrFile *openFile(LdrFileType fileType, const std::string* content,const std::string& fileName) {
            LdrFile *file = LdrFile::parseFile(fileType, fileName, content);
            /*std::vector<std::string> fileNames = {fileName};
            while (util::startsWith(util::trim(file->metaInfo.title), "~Moved to ")) {
                fileName = util::trim(file->metaInfo.title).substr(10) + ".dat";
                auto typeNamePair = findAndReadFileContent(fileName);
                file = LdrFile::parseFile(typeNamePair.first, fileName, typeNamePair.second);
                fileNames.push_back(fileName);
            }
            auto filePair = std::make_pair(fileType, file);
            for (const auto &fname : fileNames) {
                files[fname] = filePair;
            }*///todo put that somewhere else

            files[fileName] = {fileType, file};
            return file;
        }

        std::stringstream readFileToStringstream(const std::filesystem::path& path) {
            std::stringstream result;
            std::ifstream fileStream(path);
            result << fileStream.rdbuf();
            return result;
        }

        std::string* readFileToString(const std::filesystem::path& path) {
            std::string *pointer;
            auto it = otherFileCache.find(path);
            if (it==otherFileCache.end()) {
                std::ostringstream sstr;
                sstr << std::ifstream(path).rdbuf();
                otherFileCache[path] = std::string(sstr.str());
                pointer = &otherFileCache[path];
            } else {
                pointer = &it->second;
            }
            {
                std::lock_guard<std::mutex> lg(otherFileCacheLockMutex);
                otherFileCacheLock[pointer] = true;
            }
            return pointer;
        }

        void unlockCachedFile(const std::string* fileContent) {
            std::lock_guard<std::mutex> lg(otherFileCacheLockMutex);
            auto it = otherFileCacheLock.find(fileContent);
            if (it != otherFileCacheLock.end()) {
                it->second = false;
            }
        }

        void cleanUpFileCache() {
            std::vector<std::filesystem::path> cacheEraseEntries;
            {
                std::lock_guard<std::mutex> lg(otherFileCacheLockMutex);
                for (const auto &item : otherFileCache) {
                    auto it = otherFileCacheLock.find(&item.second);
                    if (!it->second) {
                        cacheEraseEntries.push_back(item.first);
                        otherFileCacheLock.erase(it);
                    }
                }
            }
            for (const auto &entry : cacheEraseEntries) {
                otherFileCache.erase(entry);
            }
        }

        bool shouldFileBeSavedInList(const std::string &filename) {
            return (util::endsWith(filename, ".dat")
                    || util::endsWith(filename, ".ldr")
                    || util::endsWith(filename, ".mpd"))
                   &&
                   (util::startsWith(filename, "parts/s/")
                    || util::startsWith(filename, "parts/")
                    || util::startsWith(filename, "p/")
                    || util::startsWith(filename, "models/"));
        }

        void fillFileListFromZip() {
            std::vector<db::fileList::Entry> entries;
            for (const auto &file : zipLibrary->textFiles) {
                if (shouldFileBeSavedInList(file.first)) {
                    auto ldrFile = getFileInLdrawDirectory(file.first);
                    entries.push_back({ldrFile->metaInfo.name, ldrFile->metaInfo.title, ldrFile->metaInfo.getCategory()});
                }
            }
            db::fileList::put(entries);
        }
        
        void fillFileListFromDirectory() {
            std::vector<db::fileList::Entry> entries;
            for (const auto &entry : std::filesystem::recursive_directory_iterator(ldrawDirectory)) {
                auto path = util::withoutBasePath(entry.path(), ldrawDirectory).string();
                auto pathWithForwardSlash = util::replaceChar(path, '\\', '/');

                if (shouldFileBeSavedInList(pathWithForwardSlash)) {
                    auto ldrFile = getFileInLdrawDirectory(path);
                    auto type = convertLDrawDirPathToFilenameAndType(pathWithForwardSlash).first;

                    std::string category;
                    if (type==LdrFileType::PART) {
                        char &firstChar = ldrFile->metaInfo.title[0];
                        if ((firstChar == '~' && ldrFile->metaInfo.title[1] != '|') || firstChar=='=' || firstChar=='_') {
                            category = PSEUDO_CATEGORY_HIDDEN_PART;
                        } else {
                            category = ldrFile->metaInfo.getCategory();
                        }
                    } else if (type==LdrFileType::SUBPART) {
                        category = PSEUDO_CATEGORY_SUBPART;
                    } else if (type==LdrFileType::PRIMITIVE) {
                        category = PSEUDO_CATEGORY_PRIMITIVE;
                    } else if (type==LdrFileType::MODEL) {
                        category = PSEUDO_CATEGORY_MODEL;
                    }
                    entries.push_back({pathWithForwardSlash, ldrFile->metaInfo.title, category});
                }
            }
            db::fileList::put(entries);
        }
        
        /**
         * @param ldrawDirPath a relative path from the ldraw directory, for example parts/3001.dat 
         * @return the LdrFileType and the fileName like its referenced in ldr files, for example 3001.dat or s/xyz123.dat
         */
        std::pair<LdrFileType, std::string> convertLDrawDirPathToFilenameAndType(const std::string &ldrawDirPath) {
            if (util::startsWith(ldrawDirPath, "parts/s/")) {
                return {LdrFileType::SUBPART, ldrawDirPath.substr(6)};//not 8 because "s/" should be kept
            } else if (util::startsWith(ldrawDirPath, "parts/")) {
                return {LdrFileType::PART, ldrawDirPath.substr(6)};
            } else if (util::startsWith(ldrawDirPath, "p/")) {
                return {LdrFileType::PRIMITIVE, ldrawDirPath.substr(2)};
            } else if (util::startsWith(ldrawDirPath, "models/")) {
                return {LdrFileType::MODEL, ldrawDirPath.substr(7)};
            }
            return {LdrFileType::MODEL, ldrawDirPath};
        }
    }

    LdrFile *get_file(const std::pair<LdrFileType, const std::string*> &resolvedPair, const std::string &filename) {
        auto iterator = files.find(filename);
        if (iterator == files.end()) {
            LdrFile *file = openFile(resolvedPair.first, resolvedPair.second, filename);
            unlockCachedFile(resolvedPair.second);
            return file;
        }
        return (iterator->second.second);
    }

    LdrFile *getFile(const std::string &filename) {
        auto iterator = files.find(filename);
        if (iterator == files.end()) {
            auto typeNamePair = findAndReadFileContent(filename);
            LdrFile *file = openFile(typeNamePair.first, typeNamePair.second, filename);
            unlockCachedFile(typeNamePair.second);
            return file;
        }
        return (iterator->second.second);
    }

    LdrFileType getFileType(const std::string &filename) {
        auto iterator = files.find(filename);
        if (iterator == files.end()) {
            throw std::invalid_argument("this file is unknown!");
        }
        return iterator->second.first;
    }

    void addFile(const std::string &filename, LdrFile *file, LdrFileType type) {
        files[filename] = std::make_pair(type, file);
    }

    void clearCache() {
        files.clear();
    }

    void initializeLdrawLibraryLocation() {
        static auto done = false;
        if (!done) {
            const auto &partsLibraryPath = util::extendHomeDirPath(config::getString(config::LDRAW_PARTS_LIBRARY));//todo add some search (add .zip, remove .zip) and update config
            if (std::filesystem::is_regular_file(partsLibraryPath)) {
                if (partsLibraryPath.filename().extension() != ".zip") {
                    throw std::invalid_argument(partsLibraryPath.string()+" is a file, but not a .zip! (it's a "+partsLibraryPath.filename().extension().string()+")");
                }
                isZipLibrary = true;
                zipLibrary = zip_buffer::openZipFile(partsLibraryPath);
            } else {
                isZipLibrary = false;
                ldrawDirectory = partsLibraryPath;
            }
            done = true;
        }
    }

    void initializeFileList() {
        initializeLdrawLibraryLocation();
        static auto done = false;
        if (!done) {
            if (db::fileList::getSize()==0) {
                std::cout << "starting to fill fileList" << std::endl;
                auto before = std::chrono::high_resolution_clock::now();
                if (isZipLibrary) {
                    fillFileListFromZip();//todo testing
                } else {
                    fillFileListFromDirectory();
                }
                auto after = std::chrono::high_resolution_clock::now();
                auto durationMs = std::chrono::duration_cast<std::chrono::microseconds >(after - before).count()/1000.0;
                std::cout << "INFO: filled fileList in " << durationMs << "ms. Size: " << db::fileList::getSize() << std::endl;
            }
            done = true;
        }
    }

    std::pair<LdrFileType, const std::string*> findAndReadFileContent(const std::string &filename) {
        if (std::filesystem::path(filename).is_absolute()) {
            return std::make_pair(LdrFileType::MODEL, readFileToString(filename));
        }
        
        auto filenameWithForwardSlash = util::replaceChar(filename, '\\', '/');

        auto partsFN = std::string("parts/") + filenameWithForwardSlash;
        auto partsResult = db::fileList::containsFile(partsFN);
        if (partsResult.has_value()) {
            LdrFileType type;
            type = util::startsWith(filenameWithForwardSlash, "s/") ? LdrFileType::SUBPART : LdrFileType::PART;
            return std::make_pair(type, readFileFromLdrawDirectory(partsFN));
        }

        auto primitiveFN = std::string("p/") + filenameWithForwardSlash;
        auto primitiveResult = db::fileList::containsFile(primitiveFN);
        if (primitiveResult.has_value()) {
            return std::make_pair(LdrFileType::PRIMITIVE, readFileFromLdrawDirectory(primitiveFN));
        }

        auto modelFN = std::string("models/") + filenameWithForwardSlash;
        auto modelResult = db::fileList::containsFile(modelFN);
        if (modelResult.has_value()) {
            return std::make_pair(LdrFileType::MODEL, readFileFromLdrawDirectory(modelFN));
        }
        
        return std::make_pair(LdrFileType::MODEL, readFileToString(util::extendHomeDirPath(filename)));
    }

    std::map<std::string, std::set<LdrFile *>>& getAllPartsGroupedByCategory() {
        if (!areAllPartsLoaded()) {
            for (const auto &ca : getAllCategories()) {
                if (partsByCategory.find(ca)==partsByCategory.end()) {
                    getAllFilesOfCategory(ca);
                }
            }
        }

        return partsByCategory;
    }

    std::map<std::string, std::set<LdrFile *>> &getLoadedPartsGroupedByCategory() {
        return partsByCategory;
    }

    std::set<std::string> getAllCategories() {
        static std::set<std::string> result;
        if (result.empty()) {
            result = db::fileList::getAllCategories();
            for (const auto &pseudoCategory : PSEUDO_CATEGORIES) {
                result.erase(pseudoCategory);
            }
        }
        return result;
    }

    std::set<LdrFile *> getAllFilesOfCategory(const std::string &categoryName) {
        auto it = partsByCategory.find(categoryName);
        if (it == partsByCategory.end()) {
            const auto &fileNames = db::fileList::getAllPartsForCategory(categoryName);
            std::set<LdrFile*> result;
            for (const auto &fileName : fileNames) {
                result.insert(getFileInLdrawDirectory(fileName));
            }
            partsByCategory.emplace(categoryName, result);
            return result;
        }
        return it->second;
    }

    /**
     * @param filename path relative to the ldraw directory
     * @return opened LdrFile
     */
    LdrFile *getFileInLdrawDirectory(const std::string &filename) {
        initializeLdrawLibraryLocation();
        auto typeNamePair = convertLDrawDirPathToFilenameAndType(filename);
        auto it = files.find(typeNamePair.second);
        if (it == files.end()) {
            const auto *content = readFileFromLdrawDirectory(filename);
            LdrFile *file = openFile(typeNamePair.first, content, typeNamePair.second);
            unlockCachedFile(content);
            return file;
        } else {
            return it->second.second;
        }
    }

    const std::string *readFileFromLdrawDirectory(const std::string &filename) {
        initializeLdrawLibraryLocation();
        if (isZipLibrary) {
            return zipLibrary->getFileAsString(filename);
        } else {
            return readFileToString(ldrawDirectory / filename);
        }
    }

    bool areAllPartsLoaded() {
        return getAllCategories().size() == partsByCategory.size();
    }
}