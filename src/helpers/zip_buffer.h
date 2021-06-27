#pragma once

#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <optional>

namespace bricksim::zip_buffer {
    class BufferedZip {

    public:
        std::map<std::string, std::string> textFiles;
        std::map<std::string, std::vector<char>> binaryFiles;
        BufferedZip(const std::filesystem::path &path, const std::optional<std::string> &password={}, bool caseSensitive= true, const std::optional<std::string>& prefixToReplace={});
        std::stringstream getFileAsStream(const std::string& filename);

        const std::string * getFileAsString(const std::string &filename);
    private:
        bool caseSensitive;
        [[nodiscard]] std::string convertFilenameToKey(const std::string& filename) const;
    };

    BufferedZip* openZipFile(const std::filesystem::path& path, const std::optional<std::string>& password={}, bool caseSensitive= true, const std::optional<std::string>& prefixToReplace={});
    void closeZipFile(const std::filesystem::path&);
}

