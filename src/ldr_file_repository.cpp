// ldr_file_repository.cpp
// Created by bb1950328 on 06.10.20.
//

#include <iostream>
#include <fstream>
#include <thread>
#include "ldr_file_repository.h"
#include "helpers/util.h"
#include "config.h"

namespace ldr_file_repo {
    namespace {
        std::map<std::string, std::pair<LdrFileType, LdrFile *>> files;
        std::filesystem::path ldrawDirectory;
        std::filesystem::path partsDirectory;
        std::filesystem::path subpartsDirectory;
        std::filesystem::path primitivesDirectory;
        std::filesystem::path modelsDirectory;
        bool namesInitialized;
        std::map<std::string, std::filesystem::path> primitiveNames;
        std::map<std::string, std::filesystem::path> subpartNames;
        std::map<std::string, std::filesystem::path> partNames;
        std::map<std::string, std::filesystem::path> modelNames;
        
        std::map<std::string, std::set<LdrFile*>> partsByCategory;
        
        LdrFile* openFile(std::pair<LdrFileType, std::filesystem::path> typeNamePair, const std::string& fileName) {
            LdrFile *file = LdrFile::parseFile(typeNamePair.first, typeNamePair.second);
            std::vector<std::string> fileNames = {fileName};
            while (util::starts_with(util::trim(file->metaInfo.title), "~Moved to ")) {
                typeNamePair = resolve_file(util::trim(file->metaInfo.title).substr(10) + ".dat");
                file = LdrFile::parseFile(typeNamePair.first, typeNamePair.second);
                fileNames.push_back(typeNamePair.second.filename());
            }
            auto filePair = std::make_pair(typeNamePair.first, file);
            for (const auto &fname : fileNames) {
                files[fname] = filePair;
            }
            //file->preLoadSubfilesAndEstimateComplexity(); todo check if this is still useful
            return file;
        }
    }
    
    LdrFile *get_file(const std::pair<LdrFileType, std::filesystem::path> &resolvedPair, const std::string &filename) {
        auto iterator = files.find(filename);
        if (iterator == files.end()) {
            return openFile(resolvedPair, filename);
        }
        return (iterator->second.second);
    }

    LdrFile *get_file(const std::string &filename) {
        auto iterator = files.find(filename);
        if (iterator == files.end()) {
            std::pair<LdrFileType, std::filesystem::path> typeNamePair;
            typeNamePair = resolve_file(filename);
            return openFile(typeNamePair, filename);
        }
        return (iterator->second.second);
    }

    LdrFileType get_file_type(const std::string &filename) {
        auto iterator = files.find(filename);
        if (iterator == files.end()) {
            throw std::invalid_argument("this file is unknown!");
        }
        return iterator->second.first;
    }

    void add_file(const std::string &filename, LdrFile *file, LdrFileType type) {
        files[filename] = std::make_pair(type, file);
    }

    void clear_cache() {
        files.clear();
    }

    bool initializeNames() {
        if (!namesInitialized) {
            ldrawDirectory = util::extend_home_dir_path(config::get_string(config::LDRAW_PARTS_LIBRARY));
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
                            partNames[util::as_lower(fname.string())] = fname;
                        }
                    }
                    for (const auto &entry : std::filesystem::directory_iterator(subpartsDirectory)) {
                        if (entry.is_regular_file()) {
                            auto fname = entry.path().filename();
                            subpartNames[util::as_lower(std::string("s\\") + fname.string())] = fname;
                        }
                    }
                    for (const auto &entry : std::filesystem::recursive_directory_iterator(primitivesDirectory)) {
                        if (entry.is_regular_file()) {
                            const auto &fname = std::filesystem::relative(entry.path(), primitivesDirectory);
                            primitiveNames[util::as_lower(fname.string())] = fname;
                        }
                    }
                    for (const auto &entry : std::filesystem::recursive_directory_iterator(modelsDirectory)) {
                        if (entry.is_regular_file()) {
                            const auto &fname = std::filesystem::relative(entry.path(), modelsDirectory);
                            modelNames[util::as_lower(fname.string())] = fname;
                        }
                    }
                    namesInitialized = true;
                    auto after = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
                    std::cout << "Initialized name lists in " << duration << "ms. found:" << std::endl;
                    std::cout << "\t" << partNames.size() << " parts in " << partsDirectory.string() << "(first is "
                              << partNames.begin()->second << ")" << std::endl;
                    std::cout << "\t" << subpartNames.size() << " subparts in " << subpartsDirectory.string()
                              << "(first is " << subpartNames.begin()->second << ")" << std::endl;
                    std::cout << "\t" << primitiveNames.size() << " primitives in " << primitivesDirectory.string()
                              << "(first is " << primitiveNames.begin()->second << ")" << std::endl;
                    std::cout << "\t" << modelNames.size() << " files in " << modelsDirectory.string() << "(first is "
                              << modelNames.begin()->second << ")" << std::endl;
                }
            }
        }
        return namesInitialized;
    }

    std::pair<LdrFileType, std::filesystem::path> resolve_file(const std::string &filename) {
        initializeNames();
        auto filenameWithPlatformSeparators = util::replaceChar(filename, '\\',
                                                                std::filesystem::path::preferred_separator);
        auto itSubpart = subpartNames.find(util::as_lower(filename));
        if (itSubpart != subpartNames.end()) {
            auto fullPath = subpartsDirectory / itSubpart->second;
            return std::make_pair(LdrFileType::SUBPART, fullPath);
        }
        auto itPart = partNames.find(util::as_lower(filenameWithPlatformSeparators));
        if (partNames.end() != itPart) {
            auto fullPath = partsDirectory / itPart->second;
            return std::make_pair(LdrFileType::PART, fullPath);
        }
        auto itPrimitive = primitiveNames.find(util::as_lower(filenameWithPlatformSeparators));
        if (primitiveNames.end() != itPrimitive) {
            auto fullPath = primitivesDirectory / itPrimitive->second;
            return std::make_pair(LdrFileType::PRIMITIVE, fullPath);
        }
        auto itModel = modelNames.find(util::as_lower(filenameWithPlatformSeparators));
        if (modelNames.end() != itModel) {
            auto fullPath = modelsDirectory / itModel->second;
            return std::make_pair(LdrFileType::MODEL, fullPath);
        }
        return std::make_pair(LdrFileType::MODEL, util::extend_home_dir_path(filename));
    }

    void openAndReadFiles(std::map<std::string, std::filesystem::path>::const_iterator start, std::map<std::string, std::filesystem::path>::const_iterator end) {
        for (auto i=start;i!=end;++i) {
            auto filename = i->first;
            auto path = i->second;
            LdrFile* file = get_file(std::make_pair(LdrFileType::PART, partsDirectory / path), filename);
            partsByCategory[file->metaInfo.category].insert(file);
        }
    }

    std::map<std::string, std::set<LdrFile *>> getPartsGroupedByCategory() {
        if (partsByCategory.empty()) {
            auto before = std::chrono::high_resolution_clock::now();
            auto numCores = std::thread::hardware_concurrency();
            numCores = std::max(1u, numCores);
            if (numCores==1) {
                openAndReadFiles(partNames.begin(), partNames.end());
            } else {
                unsigned long chunkSize = partNames.size() / numCores;
                auto startIt = partNames.begin();
                std::vector<std::thread> threads;
                for (int i = 1; i <= numCores; ++i) {
                    std::map<std::string, std::filesystem::path>::iterator endIt;
                    if (i != numCores) {//last one has to do a little bit more work it the number of parts is not divisible by numCores
                        endIt = startIt;
                        std::advance(endIt, chunkSize);
                    } else {
                        endIt = partNames.end();
                    }
                    threads.emplace_back(openAndReadFiles, startIt, endIt);
                    startIt = endIt;
                }
                for (auto &t : threads) {
                    t.join();
                }
            }
            auto after = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds >(after - before).count()/1000.0;
            std::cout << "loaded remaining parts in " << duration << "ms using " << numCores << " threads." << std::endl;
        }
        return partsByCategory;
    }
}