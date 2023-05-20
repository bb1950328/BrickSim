#pragma once

#include "files.h"
#include <filesystem>

namespace bricksim::ldr {
    const std::string SUBFILE_SEPARATOR = stringutil::repeat(LDR_NEWLINE, 3);

    void writeFile(const std::shared_ptr<File>& file, const std::filesystem::path& path);
    void writeFiles(const std::shared_ptr<File>& mainFile, const std::vector<std::shared_ptr<File>>& files, const std::filesystem::path& path);
    void writeFile(const std::shared_ptr<File>& file, std::ostream& stream, const std::string& filename);
}
