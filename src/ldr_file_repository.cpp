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

        bool namesInitialized;
        bool isZipLibrary;

        //key is name lowercase, value is name in original case
        std::map<std::string, std::string> primitiveNames;
        std::map<std::string, std::string> subpartNames;
        std::map<std::string, std::string> partNames;
        std::map<std::string, std::string> modelNames;

        std::map<std::string, std::set<LdrFile *>> partsByCategory;

        LdrFile *openFile(LdrFileType fileType, const std::string* content,std::string fileName) {
            LdrFile *file = LdrFile::parseFile(fileType, fileName, content);
            std::vector<std::string> fileNames = {fileName};
            while (util::startsWith(util::trim(file->metaInfo.title), "~Moved to ")) {
                fileName = util::trim(file->metaInfo.title).substr(10) + ".dat";
                auto typeNamePair = resolveFile(fileName);
                file = LdrFile::parseFile(typeNamePair.first, fileName, typeNamePair.second);
                fileNames.push_back(fileName);
            }
            auto filePair = std::make_pair(fileType, file);
            for (const auto &fname : fileNames) {
                files[fname] = filePair;
            }
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
            auto typeNamePair = resolveFile(filename);
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

    bool initializeNames() {
        if (!namesInitialized) {
            const auto &partsLibraryPath = util::extendHomeDirPath(config::getString(config::LDRAW_PARTS_LIBRARY));//todo add some search (add .zip, remove .zip) and update config
            if (std::filesystem::is_regular_file(partsLibraryPath)) {
                if (partsLibraryPath.filename().extension() != ".zip") {
                    throw std::invalid_argument(partsLibraryPath.string()+" is a file, but not a .zip! (it's a "+partsLibraryPath.filename().extension().string()+")");
                }

                auto before = std::chrono::high_resolution_clock::now();
                isZipLibrary = true;
                zipLibrary = zip_buffer::openZipFile(partsLibraryPath);
                for (const auto &file : zipLibrary->textFiles) {
                    if (util::startsWith(file.first, "parts/s/")) {
                        auto fname = file.first.substr(6);//not 8 because "s/" should be kept
                        subpartNames[util::asLower(fname)] = fname;
                    } else if (util::startsWith(file.first, "parts/")) {
                        auto fname = file.first.substr(6);
                        partNames[util::asLower(fname)] = fname;
                    } else if (util::startsWith(file.first, "p/")) {
                        auto fname = file.first.substr(2);
                        primitiveNames[util::asLower(fname)] = fname;
                    } else if (util::startsWith(file.first, "models/")) {
                        auto fname = file.first.substr(7);
                        modelNames[util::asLower(fname)] = fname;
                    }
                }

                namesInitialized = true;
                auto after = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
                std::cout << "initialized name lists in " << duration/1000.0 << "ms. found:" << std::endl;
                std::cout << "\t" << partNames.size() << " parts (first is " << partNames.begin()->second << ")" << std::endl;
                std::cout << "\t" << subpartNames.size() << " subparts (first is " << subpartNames.begin()->second << ")" << std::endl;
                std::cout << "\t" << primitiveNames.size() << " primitives (first is " << primitiveNames.begin()->second << ")" << std::endl;
                std::cout << "\t" << modelNames.size() << " files (first is " << modelNames.begin()->second << ")" << std::endl;
            } else {
                isZipLibrary = false;
                ldrawDirectory = partsLibraryPath;
                std::cout << "ldraw dir: " << ldrawDirectory << std::endl;
                if (std::filesystem::exists(ldrawDirectory) && std::filesystem::is_directory(ldrawDirectory)) {
                    auto before = std::chrono::high_resolution_clock::now();
                    partsDirectory = ldrawDirectory / std::filesystem::path("parts");
                    subpartsDirectory = partsDirectory / std::filesystem::path("s");
                    primitivesDirectory = ldrawDirectory / std::filesystem::path("p");
                    modelsDirectory = ldrawDirectory / std::filesystem::path("models");

                    if (std::filesystem::exists(partsDirectory)
                        && std::filesystem::exists(subpartsDirectory)
                        && std::filesystem::exists(primitivesDirectory)
                        && std::filesystem::exists(modelsDirectory)) {
                        //todo code duplication
                        for (const auto &entry : std::filesystem::directory_iterator(partsDirectory)) {
                            if (entry.is_regular_file()) {
                                auto fname = entry.path().filename();
                                partNames[util::asLower(fname.string())] = fname.string();
                            }
                        }
                        for (const auto &entry : std::filesystem::directory_iterator(subpartsDirectory)) {
                            if (entry.is_regular_file()) {
                                auto fname = entry.path().filename();
                                subpartNames[util::asLower(std::string("s\\") + fname.string())] = fname.string();
                            }
                        }
                        for (const auto &entry : std::filesystem::recursive_directory_iterator(primitivesDirectory)) {
                            if (entry.is_regular_file()) {
                                const auto &fname = std::filesystem::relative(entry.path(), primitivesDirectory);
                                primitiveNames[util::asLower(fname.string())] = fname.string();
                            }
                        }
                        for (const auto &entry : std::filesystem::recursive_directory_iterator(modelsDirectory)) {
                            if (entry.is_regular_file()) {
                                const auto &fname = std::filesystem::relative(entry.path(), modelsDirectory);
                                modelNames[util::asLower(fname.string())] = fname.string();
                        }
                    }
                    namesInitialized = true;
                    auto after = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
                    std::cout << "Initialized name lists in " << duration << "ms. found:" << std::endl;
                    std::cout << "\t" << partNames.size() << " parts in " << partsDirectory.string() << "(first is " << partNames.begin()->second << ")" << std::endl;
                    std::cout << "\t" << subpartNames.size() << " subparts in " << subpartsDirectory.string() << "(first is " << subpartNames.begin()->second << ")" << std::endl;
                    std::cout << "\t" << primitiveNames.size() << " primitives in " << primitivesDirectory.string() << "(first is " << primitiveNames.begin()->second << ")" << std::endl;
                    std::cout << "\t" << modelNames.size() << " files in " << modelsDirectory.string() << "(first is " << modelNames.begin()->second << ")" << std::endl;}
                }
            }
        }
        return namesInitialized;
    }

    std::pair<LdrFileType, const std::string*> resolveFile(const std::string &filename) {
        initializeNames();
        if (isZipLibrary) {
            auto filenameWithForwardSlash = util::replaceChar(filename, '\\', '/');
            auto itSubpart = subpartNames.find(util::asLower(filenameWithForwardSlash));
            if (itSubpart != subpartNames.end()) {
                return std::make_pair(LdrFileType::SUBPART, zipLibrary->getFileAsString(std::string("parts/")+itSubpart->second));
            }
            auto itPart = partNames.find(util::asLower(filenameWithForwardSlash));
            if (partNames.end() != itPart) {
                return std::make_pair(LdrFileType::PART, zipLibrary->getFileAsString(std::string("parts/")+itPart->second));
            }
            auto itPrimitive = primitiveNames.find(util::asLower(filenameWithForwardSlash));
            if (primitiveNames.end() != itPrimitive) {
                return std::make_pair(LdrFileType::PRIMITIVE, zipLibrary->getFileAsString(std::string("p/")+itPrimitive->second));
            }
            auto itModel = modelNames.find(util::asLower(filenameWithForwardSlash));
            if (modelNames.end() != itModel) {
                return std::make_pair(LdrFileType::MODEL, zipLibrary->getFileAsString(std::string("models/")+itModel->second));
            }
            auto itZip = zipLibrary->textFiles.find(filenameWithForwardSlash);
            if (itZip != zipLibrary->textFiles.end()) {
                return std::make_pair(LdrFileType::MODEL, zipLibrary->getFileAsString(filenameWithForwardSlash));
            }
        } else {
            auto filenameWithPlatformSeparators = util::replaceChar(filename, '\\', std::filesystem::path::preferred_separator);
            auto itSubpart = subpartNames.find(util::asLower(filename));
            if (itSubpart != subpartNames.end()) {
                auto fullPath = subpartsDirectory / itSubpart->second;
                return std::make_pair(LdrFileType::SUBPART, readFileToString(fullPath));
            }
            auto itPart = partNames.find(util::asLower(filenameWithPlatformSeparators));
            if (partNames.end() != itPart) {
                auto fullPath = partsDirectory / itPart->second;
                return std::make_pair(LdrFileType::PART, readFileToString(fullPath));
            }
            auto itPrimitive = primitiveNames.find(util::asLower(filenameWithPlatformSeparators));
            if (primitiveNames.end() != itPrimitive) {
                auto fullPath = primitivesDirectory / itPrimitive->second;
                return std::make_pair(LdrFileType::PRIMITIVE, readFileToString(fullPath));
            }
            auto itModel = modelNames.find(util::asLower(filenameWithPlatformSeparators));
            if (modelNames.end() != itModel) {
                auto fullPath = modelsDirectory / itModel->second;
                return std::make_pair(LdrFileType::MODEL, readFileToString(fullPath));
            }
            if (std::filesystem::is_regular_file(ldrawDirectory / filename)) {
                return std::make_pair(LdrFileType::MODEL, readFileToString(ldrawDirectory / filename));
            }
        }
        return std::make_pair(LdrFileType::MODEL, readFileToString(util::extendHomeDirPath(filename)));
    }

    void openAndReadFiles(std::map<std::string, std::string>::const_iterator start, std::map<std::string, std::string>::const_iterator end) {
        for (auto i = start; i != end; ++i) {
            auto filename = i->first;
            auto path = i->second;
            LdrFile* file = getFile(filename);
            const std::string &category = file->metaInfo.getCategory();
            if (file->metaInfo.title[0] != '~') {
                partsByCategory[category].insert(file);
            }
        }
    }

    std::map<std::string, std::set<LdrFile *>> getPartsGroupedByCategory() {
        if (partsByCategory.empty()) {
            std::cout << "started loading remaining parts" << std::endl;
            auto before = std::chrono::high_resolution_clock::now();
            auto numCores = std::thread::hardware_concurrency();
            numCores = std::max(1u, numCores);
            if (numCores == 1) {
                openAndReadFiles(partNames.begin(), partNames.end());
            } else {
                unsigned long chunkSize = partNames.size() / numCores;
                auto startIt = partNames.begin();
                std::vector<std::thread> threads;
                for (int i = 1; i <= numCores; ++i) {
                    std::map<std::string, std::string>::iterator endIt;
                    if (i != numCores) {
                        endIt = startIt;
                        std::advance(endIt, chunkSize);
                    } else {
                        endIt = partNames.end();//last one has to do a little bit more work it the number of parts is not divisible by numCores
                    }
                    threads.emplace_back(openAndReadFiles, startIt, endIt);
                    startIt = endIt;
                }
                for (auto &t : threads) {
                    t.join();
                }
            }
            auto after = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0;
            cleanUpFileCache();
            std::cout << "loaded remaining parts in " << duration << "ms using " << numCores << " threads." << std::endl;
        }
        return partsByCategory;
    }
}