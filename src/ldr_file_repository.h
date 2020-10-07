// ldr_file_repository.h
// Created by bb1950328 on 06.10.20.
//

#ifndef BRICKSIM_LDR_FILE_REPOSITORY_H
#define BRICKSIM_LDR_FILE_REPOSITORY_H

#include <map>
#include <filesystem>
#include "ldr_files.h"

class LdrFileRepository {
public:
    static LdrFile *get_file(const std::string &filename);

    static LdrFileType get_file_type(const std::string &filename);

    static std::pair<LdrFileType, std::filesystem::path> resolve_file(const std::string &filename);

    static void clear_cache();

    static std::map<std::string, std::pair<LdrFileType, LdrFile*>> files;

    static void add_file(const std::string &filename, LdrFile *file, LdrFileType type);

    static void initializeNames();
private:
    static std::filesystem::path ldrawDirectory;
    static std::filesystem::path partsDirectory;
    static std::filesystem::path subpartsDirectory;
    static std::filesystem::path primitivesDirectory;
    static std::filesystem::path modelsDirectory;
    static bool namesInitialized;

    //keys: name as lowercase values: name as original case
    static std::map<std::string, std::filesystem::path> primitiveNames;
    static std::map<std::string, std::filesystem::path> subpartNames;
    static std::map<std::string, std::filesystem::path> partNames;
    static std::map<std::string, std::filesystem::path> modelNames;
};
#endif //BRICKSIM_LDR_FILE_REPOSITORY_H
