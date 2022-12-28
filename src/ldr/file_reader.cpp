#include "file_reader.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include <magic_enum.hpp>
#include <palanteer.h>
#include <spdlog/spdlog.h>

namespace bricksim::ldr {
    uomap_t<std::string, std::shared_ptr<File>> readComplexFile(const std::string& name,
                                                                FileType mainFileType,
                                                                const std::string& content,
                                                                const std::optional<std::string>& shadowContent) {
        //plFunction();
        if (mainFileType != FileType::MODEL) {
            return {{name, readSimpleFile(name, mainFileType, content, shadowContent)}};
        }
        auto mainFile = std::make_shared<File>();
        mainFile->metaInfo.type = mainFileType;
        uomap_t<std::string, std::shared_ptr<File>> files = {{name, mainFile}};
        std::optional<std::shared_ptr<File>> currentFile = mainFile;

        if (shadowContent.has_value()) {
            mainFile->addShadowContent(*shadowContent);
        }

        size_t lineStart = 0;
        size_t lineEnd = 0;
        bool firstFile = true;
        bool hasMoreLines = true;
        while (hasMoreLines) {
            lineEnd = content.find_first_of("\r\n", lineStart);
            if (lineEnd == std::string::npos) {
                hasMoreLines = false;
                lineEnd = content.size();
            } else {
                ++lineEnd;
            }
            std::string line = content.substr(lineStart, lineEnd - lineStart);

            if (line.starts_with("0 FILE")) {
                const auto currentName = stringutil::trim(line.substr(7));
                if (firstFile) {
                    firstFile = false;
                    files.emplace(currentName, currentFile.value());
                } else {
                    const auto it = files.find(currentName);
                    if (it != files.end()) {
                        currentFile = it->second;
                    } else {
                        currentFile = std::make_shared<File>();
                        currentFile.value()->metaInfo.type = FileType::MPD_SUBFILE;
                        files.emplace(currentName, currentFile.value());
                    }
                }
            } else if (line.starts_with("0 NOFILE") || line.starts_with("0 !DATA")) {
                //todo save !DATA somewhere instead of ignoring it
                currentFile = {};
            } else if (currentFile.has_value()) {
                currentFile.value()->addTextLine(line);
            }
            lineStart = lineEnd;
        }
        return files;
    }

    std::shared_ptr<File> readSimpleFile(std::string_view name,
                                         FileType type,
                                         const std::string& content,
                                         const std::optional<std::string>& shadowContent) {
        plFunction();
        auto file = std::make_shared<File>();
        file->metaInfo.type = type;
        file->metaInfo.name = name;
        size_t lineStart = 0;
        size_t lineEnd = 0;
        std::string_view contentSV = content;
        while (lineEnd < contentSV.size()) {
            lineEnd = contentSV.find_first_of("\r\n", lineStart);
            if (lineEnd == std::string::npos) {
                lineEnd = contentSV.size();
            } else {
                ++lineEnd;
            }
            
            file->addTextLine(contentSV.substr(lineStart, lineEnd - lineStart));
            lineStart = lineEnd;
        }

        if (shadowContent.has_value()) {
            file->addShadowContent(*shadowContent);
        }

        return file;
    }
}
