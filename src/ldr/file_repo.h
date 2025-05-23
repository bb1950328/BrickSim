#pragma once

#include "../binary_file.h"
#include "files.h"
#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <vector>

namespace bricksim::ldr::file_repo {
    enum class LibraryType {
        INVALID,
        DIRECTORY,
        ZIP,
    };

    enum class BinaryFileSearchPath {
        DEFAULT,
        TEXMAP,
    };

    extern const char* PSEUDO_CATEGORY_SUBPART;
    extern const char* PSEUDO_CATEGORY_PRIMITIVE;
    extern const char* PSEUDO_CATEGORY_MODEL;
    extern const char* PSEUDO_CATEGORY_HIDDEN_PART;
    extern const char* PSEUDO_CATEGORY_BINARY_FILE;
    extern const char* const PSEUDO_CATEGORIES[];

    extern const char* const PART_SEARCH_PREFIXES[];
    constexpr int ESTIMATE_PART_LIBRARY_FILE_COUNT = 19057;//counted on 2020-12-09

    class FileRepo {
    public:
        explicit FileRepo(std::filesystem::path basePath);
        FileRepo& operator=(FileRepo&) = delete;
        FileRepo(const FileRepo&) = delete;
        void initialize(float* progress);

        /**
         * @param fileNamespace the namespace or null
         * @param name filename (relative to namespace search path or library)
         * @return the file
         * @throws std::invalid_argument
         */
        std::shared_ptr<File> getFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name);
        /**
         * @param context the file where the name appeared
         * @param name relative to context file
         * @return the file
         * @throws std::invalid_argument
         */
        std::shared_ptr<File> getFile(const std::shared_ptr<File>& context, const std::string& name);
        /**
         * @param fileNamespace the namespace or null
         * @param name the name (relative to namespace search path or library)
         * @param contextRelativePath directory relative to namespace search path which is the basepath for name if the file is not in the library
         * @return
         */
        std::shared_ptr<File> getFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, std::optional<std::filesystem::path> contextRelativePath);
        std::shared_ptr<File> getFileOrNull(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name);
        std::shared_ptr<BinaryFile> getBinaryFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, BinaryFileSearchPath searchPath = BinaryFileSearchPath::DEFAULT);
        bool hasFileCached(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name);
        [[nodiscard]] const uomap_t<std::shared_ptr<FileNamespace>, uomap_t<std::string, std::pair<FileType, std::shared_ptr<File>>>>& getAllFilesInMemory() const;
        std::shared_ptr<File> reloadFile(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name);
        std::shared_ptr<File> addLdrFileWithContent(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, const std::filesystem::path& source, FileType type, const std::string& content);
        std::shared_ptr<File> addLdrFileWithContent(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, const std::filesystem::path& source, FileType type, const std::string& content, const std::optional<std::string>& shadowContent);
        std::shared_ptr<BinaryFile> addBinaryFileWithContent(const std::shared_ptr<FileNamespace>& fileNamespace, const std::string& name, const std::shared_ptr<BinaryFile>& file);
        std::filesystem::path& getBasePath();
        static oset_t<std::string> getAllCategories();
        std::shared_ptr<FileNamespace> getNamespace(const std::string& name);
        std::string getVersion() const;

        /**
         * @param type
         * @param name how it's referenced in line type 1. subparts for example are s\abc.dat
         * @return a path relative to the root of the LDraw parts library. example: parts/3001.dat
         */
        static std::string getPathRelativeToBase(FileType type, const std::string& name);

        oset_t<std::shared_ptr<File>> getAllFilesOfCategory(const std::string& categoryName);
        bool areAllPartsLoaded() const;
        void cleanup();

        /**
         * @param progress range from 0.0f to 0.5f
         * @return vector of file names relative to root of library
         */
        virtual std::vector<std::string> listAllFileNames(std::function<void(float)> progress) = 0;
        virtual std::string getLibraryLdrFileContent(FileType type, const std::string& name) = 0;
        virtual std::string getLibraryLdrFileContent(const std::string& nameRelativeToRoot) = 0;
        virtual std::shared_ptr<BinaryFile> getLibraryBinaryFileContent(const std::string& nameRelativeToRoot) = 0;
        virtual ~FileRepo();
        omap_t<std::string, oset_t<std::shared_ptr<File>>> getAllPartsGroupedByCategory();
        omap_t<std::string, oset_t<std::shared_ptr<File>>> getLoadedPartsGroupedByCategory() const;

        void changeFileName(const std::shared_ptr<FileNamespace>& oldNamespace,
                            const std::shared_ptr<File>& file,
                            const std::shared_ptr<FileNamespace>& newNamespace,
                            const std::string& newName);

        void updateLibraryFiles(const std::filesystem::path& updatedFileDirectory, std::function<void(float)> progress, uint64_t estimatedFileCount);
        virtual bool replaceLibraryFilesDirectlyFromZip() = 0;
        void replaceLibraryFiles(const std::filesystem::path& replacementFileOrDirectory, std::function<void(float)> progress, uint64_t estimatedFileCount);

    protected:
        static bool shouldFileBeSavedInList(const std::string& filename);
        /**
         * this is the reverse function of ldr::FileRepo::getPathRelativeToBase
         * @param pathRelativeToBase
         * @return
         */
        static std::pair<FileType, std::string> getTypeAndNameFromPathRelativeToBase(const std::string& pathRelativeToBase);
        virtual void updateLibraryFilesImpl(const std::filesystem::path& updatedFileDirectory, std::function<void(int)> progress) = 0;
        virtual void replaceLibraryFilesImpl(const std::filesystem::path& replacementFileOrDirectory, std::function<void(int)> progress) = 0;
        std::filesystem::path basePath;

        void fillFileList(std::function<void(float)> progress);
        void fillFileList(std::function<void(float)> progress, const std::string& currentLDConfigHash);
    private:
        uomap_t<std::shared_ptr<FileNamespace>, uomap_t<std::string, std::pair<FileType, std::shared_ptr<File>>>> ldrFiles;
        std::mutex ldrFilesMtx;

        uomap_t<std::shared_ptr<FileNamespace>, uomap_t<std::string, std::shared_ptr<BinaryFile>>> binaryFiles;
        std::mutex binaryFilesMtx;

        omap_t<std::string, oset_t<std::shared_ptr<File>>> partsByCategory;
        static bool isLdrFilename(const std::string& filename);
        static bool isBinaryFilename(const std::string& filename);
        std::string getLDConfigContentHash();
        void refreshAfterUpdateOrReplaceLibrary(const std::function<void(float)>& progress);
    };

    FileRepo& get();
    bool isInitialized();
    bool tryToInitializeWithLibraryPath(const std::filesystem::path& path);
    bool checkLdrawLibraryLocation();
    LibraryType getLibraryType(const std::filesystem::path& path);
}
