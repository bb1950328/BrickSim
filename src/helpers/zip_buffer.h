//
// Created by Bader on 02.11.2020.
//

#ifndef BRICKSIM_ZIP_BUFFER_H
#define BRICKSIM_ZIP_BUFFER_H

#include <filesystem>
#include <map>
#include <vector>

namespace zip_buffer {
    class BufferedZip {

    public:
        std::map<std::string, std::string> textFiles;
        std::map<std::string, std::vector<char>> binaryFiles;
        BufferedZip(const std::filesystem::path &path, const std::optional<std::string> &password);
        std::stringstream getFileAsStream(const std::string& filename);

        const std::string * getFileAsString(const std::string &filename);
    };

    BufferedZip* openZipFile(const std::filesystem::path& path, const std::optional<std::string>& password={});
    void closeZipFile(const std::filesystem::path&);
}

#endif //BRICKSIM_ZIP_BUFFER_H
