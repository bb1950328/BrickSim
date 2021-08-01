#include "file_reader.h"
#include "../helpers/util.h"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

namespace bricksim::ldr {
    std::map<std::string, std::shared_ptr<File>> readComplexFile(const std::string& name, const std::string& content, FileType mainFileType) {
        if (mainFileType != MODEL) {
            return {{name, readSimpleFile(name, content, mainFileType)}};
        }
        auto mainFile = std::make_shared<File>();
        mainFile->metaInfo.type = mainFileType;
        std::map<std::string, std::shared_ptr<File>> files = {{name, mainFile}};
        std::optional<std::shared_ptr<File>> currentFile = mainFile;

        size_t lineStart = 0;
        size_t lineEnd = 0;
        bool firstFile = true;
        bool hasMoreLines = true;
        while (hasMoreLines) {
            lineEnd = content.find_first_of("\r\n", lineStart);
            if (lineEnd == std::string::npos) {
                hasMoreLines = false;
                lineEnd = content.size();
            }
            ++lineEnd;
            if (content[lineEnd] == '\n') {
                lineEnd++;
            }
            std::string line = content.substr(lineStart, lineEnd - lineStart);

            if (util::startsWith(line, "0 FILE")) {
                const auto currentName = util::trim(line.substr(7));
                if (firstFile) {
                    firstFile = false;
                    files.emplace(currentName, currentFile.value());
                } else {
                    const auto it = files.find(currentName);
                    if (it != files.end()) {
                        currentFile = it->second;
                    } else {
                        currentFile = std::make_shared<File>();
                        currentFile.value()->metaInfo.type = MPD_SUBFILE;
                        files.emplace(currentName, currentFile.value());
                    }
                }
            } else if (util::startsWith(line, "0 NOFILE") || util::startsWith(line, "0 !DATA")) {
                //todo save !DATA somewhere instead of ignoring it
                currentFile = {};
            } else if (currentFile.has_value()) {
                currentFile.value()->addTextLine(line);
            }
            lineStart = lineEnd;
        }
        return files;
    }

    std::shared_ptr<File> readSimpleFile(const std::string& name, const std::string& content, FileType type) {
        auto file = std::make_shared<File>();
        file->metaInfo.type = type;
        size_t lineStart = 0;
        size_t lineEnd = 0;
        while (lineEnd < content.size()) {
            lineEnd = content.find_first_of("\r\n", lineStart);
            if (lineEnd == std::string::npos) {
                break;
            }
            ++lineEnd;
            if (content[lineEnd] == '\n') {
                lineEnd++;
            }
            file->addTextLine(content.substr(lineStart, lineEnd - lineStart));
            lineStart = lineEnd;
        }
        return file;
    }
}