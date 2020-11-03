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
    LdrFile* get_file(const std::pair<LdrFileType, std::stringstream&> &resolvedPair, const std::string& filename);

    LdrFileType get_file_type(const std::string &filename);

    std::pair<LdrFileType, const std::string *> resolve_file(const std::string &filename);

    void clear_cache();

    void add_file(const std::string &filename, LdrFile *file, LdrFileType type);

    bool initializeNames();

    std::map<std::string, std::set<LdrFile *>> getPartsGroupedByCategory();
};
#endif //BRICKSIM_LDR_FILE_REPOSITORY_H
