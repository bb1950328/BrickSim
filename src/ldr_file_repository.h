// ldr_file_repository.h
// Created by bb1950328 on 06.10.20.
//

#ifndef BRICKSIM_LDR_FILE_REPOSITORY_H
#define BRICKSIM_LDR_FILE_REPOSITORY_H

#include <map>
#include <filesystem>
#include "ldr_files.h"

namespace ldr_file_repo {
    LdrFile *getFile(const std::string& filename);
    LdrFile* getFile(const std::pair<LdrFileType, std::stringstream&> &resolvedPair, const std::string& filename);
    LdrFile* getFileInLdrawDirectory(const std::string& filename);
    LdrFile* getFile(const std::string& filename, LdrFileType fileType);
    void addFile(const std::string &filename, LdrFile *file, LdrFileType type);
    LdrFileType getFileType(const std::string &filename);
    std::pair<LdrFileType, const std::string *> findAndReadFileContent(const std::string &filename);
    const std::string* readFileFromLdrawDirectory(const std::string& filename);

    void clearCache();
    void initializeFileList();
    void initializeLdrawLibraryLocation();

    [[ nodiscard ]] bool areAllPartsLoaded();
    std::map<std::string, std::set<LdrFile *>>& getAllPartsGroupedByCategory();
    std::map<std::string, std::set<LdrFile *>>& getLoadedPartsGroupedByCategory();
    std::set<std::string> getAllCategories();
    std::set<LdrFile*> getAllFilesOfCategory(const std::string& categoryName);
};
#endif //BRICKSIM_LDR_FILE_REPOSITORY_H
