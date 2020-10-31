// ldr_file_repository.h
// Created by bb1950328 on 06.10.20.
//

#ifndef BRICKSIM_LDR_FILE_REPOSITORY_H
#define BRICKSIM_LDR_FILE_REPOSITORY_H

#include <map>
#include <filesystem>
#include "ldr_files.h"

namespace ldr_file_repo {
    LdrFile *get_file(const std::string& filename);

    LdrFileType get_file_type(const std::string &filename);

    std::pair<LdrFileType, std::filesystem::path> resolve_file(const std::string &filename);

    void clear_cache();

    void add_file(const std::string &filename, LdrFile *file, LdrFileType type);

    bool initializeNames();
};
#endif //BRICKSIM_LDR_FILE_REPOSITORY_H
