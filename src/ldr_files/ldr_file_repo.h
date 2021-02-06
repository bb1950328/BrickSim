// ldr_base_file_repository.h
// Created by bab21 on 01.02.21.
//

#ifndef BRICKSIM_LDR_FILE_REPO_H
#define BRICKSIM_LDR_FILE_REPO_H

#include <filesystem>
#include "ldr_files.h"

namespace ldr_file_repo {

    enum class LibraryType {
        INVALID,
        DIRECTORY,
        ZIP,
    };

    extern const char* PSEUDO_CATEGORY_SUBPART;
    extern const char* PSEUDO_CATEGORY_PRIMITIVE;
    extern const char* PSEUDO_CATEGORY_MODEL;
    extern const char* PSEUDO_CATEGORY_HIDDEN_PART;
    extern const char* const PSEUDO_CATEGORIES[];
    extern const char* const PART_SEARCH_PREFIXES[];
    constexpr int ESTIMATE_PART_LIBRARY_FILE_COUNT = 19057;//counted on 2020-12-09

    class LdrFileRepo {
    public:
        explicit LdrFileRepo(std::filesystem::path basePath);
        void initialize(float* progress);

        std::shared_ptr<LdrFile> getFile(const std::string& name);
        std::shared_ptr<LdrFile> addFileWithContent(const std::string& name, LdrFileType type, const std::string& content);
        std::filesystem::path& getBasePath();
        std::set<std::string> getAllCategories();

        std::set<std::shared_ptr<LdrFile>> getAllFilesOfCategory(const std::string &categoryName);
        bool areAllPartsLoaded();

        /**
         * @return vector of file names relative to root of library
         */
        virtual std::vector<std::string> listAllFileNames(float *progress) = 0;
        virtual std::string getLibraryFileContent(LdrFileType type, std::string name) = 0;
        virtual std::string getLibraryFileContent(std::string nameRelativeToRoot) = 0;
        virtual ~LdrFileRepo();
        std::map<std::string, std::set<std::shared_ptr<LdrFile>>> getAllPartsGroupedByCategory();
        std::map<std::string, std::set<std::shared_ptr<LdrFile>>> getLoadedPartsGroupedByCategory();
    protected:
        static std::string readFileFromFilesystem(const std::filesystem::path& path);
        static bool shouldFileBeSavedInList(const std::string &filename);
        /**
         * @param type
         * @param name how it's referenced in line type 1. subparts for example are s\abc.dat
         * @return a path relative to the root of the LDraw parts library. example: parts/3001.dat
         */
        static std::string getPathRelativeToBase(LdrFileType type, const std::string& name);
        /**
         * this is the reverse function of LdrFileRepo::getPathRelativeToBase
         * @param pathRelativeToBase
         * @return
         */
        static std::pair<LdrFileType, std::string> getTypeAndNameFromPathRelativeToBase(const std::string& pathRelativeToBase);
        std::filesystem::path basePath;

    private:
        std::map<std::string, std::pair<LdrFileType, std::shared_ptr<LdrFile>>> files;
        std::map<std::string, std::set<std::shared_ptr<LdrFile>>> partsByCategory;
    };

    LdrFileRepo& get();
    bool tryToInitializeWithLibraryPath(const std::filesystem::path &path);
    bool checkLdrawLibraryLocation();
    LibraryType getLibraryType(const std::filesystem::path& path);
}

#endif //BRICKSIM_LDR_FILE_REPO_H
