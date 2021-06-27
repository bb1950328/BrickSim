#ifndef BRICKSIM_LDR_FILE_REPO_H
#define BRICKSIM_LDR_FILE_REPO_H


#include <filesystem>
#include <set>
#include <vector>
#include <map>
#include "ldr_files.h"

namespace bricksim::ldr::file_repo {

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

    class FileRepo {
    public:
        explicit FileRepo(std::filesystem::path basePath);
        FileRepo & operator=(FileRepo&) = delete;
        FileRepo(const FileRepo&) = delete;
        void initialize(float* progress);

        std::shared_ptr<File> getFile(const std::string& name);
        std::shared_ptr<File> addFileWithContent(const std::string& name, FileType type, const std::string& content);
        std::filesystem::path& getBasePath();
        std::set<std::string> getAllCategories();

        std::set<std::shared_ptr<File>> getAllFilesOfCategory(const std::string &categoryName);
        bool areAllPartsLoaded();
        void cleanup();

        /**
         * @param progress range from 0.0f to 0.5f
         * @return vector of file names relative to root of library
         */
        virtual std::vector<std::string> listAllFileNames(float *progress) = 0;
        virtual std::string getLibraryFileContent(FileType type, std::string name) = 0;
        virtual std::string getLibraryFileContent(std::string nameRelativeToRoot) = 0;
        virtual ~FileRepo();
        std::map<std::string, std::set<std::shared_ptr<File>>> getAllPartsGroupedByCategory();
        std::map<std::string, std::set<std::shared_ptr<File>>> getLoadedPartsGroupedByCategory();
    protected:
        static std::string readFileFromFilesystem(const std::filesystem::path& path);
        static bool shouldFileBeSavedInList(const std::string &filename);
        /**
         * @param type
         * @param name how it's referenced in line type 1. subparts for example are s\abc.dat
         * @return a path relative to the root of the LDraw parts library. example: parts/3001.dat
         */
        static std::string getPathRelativeToBase(FileType type, const std::string& name);
        /**
         * this is the reverse function of ldr::FileRepo::getPathRelativeToBase
         * @param pathRelativeToBase
         * @return
         */
        static std::pair<FileType, std::string> getTypeAndNameFromPathRelativeToBase(const std::string& pathRelativeToBase);
        std::filesystem::path basePath;

    private:
        std::map<std::string, std::pair<FileType, std::shared_ptr<File>>> files;
        std::map<std::string, std::set<std::shared_ptr<File>>> partsByCategory;
    };

    FileRepo& get();
    bool tryToInitializeWithLibraryPath(const std::filesystem::path &path);
    bool checkLdrawLibraryLocation();
    LibraryType getLibraryType(const std::filesystem::path& path);
}

#endif //BRICKSIM_LDR_FILE_REPO_H
