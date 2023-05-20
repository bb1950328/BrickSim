#include "file_writer.h"
#include "palanteer.h"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

namespace bricksim::ldr {
    void writeFile(const std::shared_ptr<File>& file, const std::filesystem::path& path) {
        writeFiles(file, {}, path);
    }

    void writeFiles(const std::shared_ptr<File>& mainFile, const std::vector<std::shared_ptr<File>>& files, const std::filesystem::path& path) {
        std::ofstream stream(path, std::ios::out | std::ios::binary);//not really a binary file, but we don't want the OS to mess with newlines and stuff
        writeFile(mainFile, stream, path.filename().string());
        for (const auto& file: files) {
            stream << SUBFILE_SEPARATOR;
            writeFile(file, stream, file->metaInfo.name);
        }
    }

    void writeFile(const std::shared_ptr<File>& file, std::ostream& stream, const std::string& filename) {
        plScope("ldr::writeFile");
        stream << "0 FILE " << filename << LDR_NEWLINE;

        auto nameBackup = file->metaInfo.name;
        file->metaInfo.name = filename;
        stream << file->metaInfo;
        file->metaInfo.name = nameBackup;

        stream << LDR_NEWLINE;

        for (const auto& element: file->elements) {
            stream << element->getLdrLine() << LDR_NEWLINE;
        }
    }
}
