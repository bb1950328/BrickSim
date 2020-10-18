// ldr_file_repository.cpp
// Created by bb1950328 on 06.10.20.
//

#include <iostream>
#include "ldr_file_repository.h"
#include "util.h"
#include "config.h"

std::map<std::string, std::pair<LdrFileType, LdrFile*>> LdrFileRepository::files;
std::filesystem::path LdrFileRepository::ldrawDirectory;
std::filesystem::path LdrFileRepository::partsDirectory;
std::filesystem::path LdrFileRepository::subpartsDirectory;
std::filesystem::path LdrFileRepository::primitivesDirectory;
std::filesystem::path LdrFileRepository::modelsDirectory;
bool LdrFileRepository::namesInitialized;
std::map<std::string, std::filesystem::path> LdrFileRepository::primitiveNames;
std::map<std::string, std::filesystem::path> LdrFileRepository::subpartNames;
std::map<std::string, std::filesystem::path> LdrFileRepository::partNames;
std::map<std::string, std::filesystem::path> LdrFileRepository::modelNames;

LdrFile *LdrFileRepository::get_file(const std::string &filename) {
    auto iterator = files.find(filename);
    if (iterator == files.end()) {
        auto typeNamePair = resolve_file(filename);
        LdrFile* file = LdrFile::parseFile(typeNamePair.first, typeNamePair.second);
        files[filename] = std::make_pair(typeNamePair.first, file);
        file->preLoadSubfilesAndEstimateComplexity();
        return file;
    }
    return (iterator->second.second);
}

LdrFileType LdrFileRepository::get_file_type(const std::string &filename) {
    auto iterator = files.find(filename);
    if (iterator == files.end()) {
        throw std::invalid_argument("this file is unknown!");
    }
    return iterator->second.first;
}

void LdrFileRepository::add_file(const std::string &filename, LdrFile *file, LdrFileType type) {
    files[filename] = std::make_pair(type, file);
}
void LdrFileRepository::clear_cache(){
    files.clear();
}
void LdrFileRepository::initializeNames() {
    if (!namesInitialized) {
        auto before = std::chrono::high_resolution_clock::now();
        ldrawDirectory = util::extend_home_dir_path(config::get_string(config::LDRAW_PARTS_LIBRARY));
        partsDirectory = ldrawDirectory / std::filesystem::path("parts");
        subpartsDirectory = partsDirectory / std::filesystem::path("s");
        primitivesDirectory = ldrawDirectory / std::filesystem::path("p");
        modelsDirectory = ldrawDirectory / std::filesystem::path("models");
        std::cout << "ldraw dir: " << ldrawDirectory << std::endl;
        //todo code duplication
        for (const auto & entry : std::filesystem::directory_iterator(partsDirectory)) {
            if (entry.is_regular_file()) {
                auto fname = entry.path().filename();
                partNames[util::as_lower(fname.string())] = fname;
            }
        }
        for (const auto & entry : std::filesystem::directory_iterator(subpartsDirectory)) {
            if (entry.is_regular_file()) {
                auto fname = entry.path().filename();
                subpartNames[util::as_lower(std::string("s\\")+fname.string())] = fname;
            }
        }
        for (const auto & entry : std::filesystem::recursive_directory_iterator(primitivesDirectory)) {
            if (entry.is_regular_file()) {
                const auto& fname = std::filesystem::relative(entry.path(), primitivesDirectory);
                primitiveNames[util::as_lower(fname.string())] = fname;
            }
        }
        for (const auto & entry : std::filesystem::recursive_directory_iterator(modelsDirectory)) {
            if (entry.is_regular_file()) {
                const auto& fname = std::filesystem::relative(entry.path(), modelsDirectory);
                modelNames[util::as_lower(fname.string())] = fname;
            }
        }
        namesInitialized = true;
        auto after = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after-before).count();
        std::cout << "Initialized name lists in " << duration << "ms. found:" << std::endl;
        std::cout << "\t" << partNames.size() << " parts in " << partsDirectory.string() << "(first is " << partNames.begin()->second << ")" << std::endl;
        std::cout << "\t" << subpartNames.size() << " subparts in " << subpartsDirectory.string()  << "(first is " << subpartNames.begin()->second << ")"<< std::endl;
        std::cout << "\t" << primitiveNames.size() << " primitives in " << primitivesDirectory.string()  << "(first is " << primitiveNames.begin()->second << ")"<< std::endl;
        std::cout << "\t" << modelNames.size() << " files in " << modelsDirectory.string()  << "(first is " << modelNames.begin()->second << ")"<< std::endl;
    }
}
std::pair<LdrFileType, std::filesystem::path> LdrFileRepository::resolve_file(const std::string & filename) {
    initializeNames();
    auto filenameWithPlatformSeparators = util::replaceChar(filename, '\\', std::filesystem::path::preferred_separator);
    auto itSubpart = subpartNames.find(util::as_lower(filename));
    if (itSubpart != subpartNames.end()) {
        auto fullPath = subpartsDirectory / itSubpart->second;
        return std::make_pair(LdrFileType::SUBPART, fullPath);
    }
    auto itPart = partNames.find(util::as_lower(filenameWithPlatformSeparators));
    if (partNames.end()!=itPart) {
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