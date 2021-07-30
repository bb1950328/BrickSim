#pragma once
#include "files.h"

namespace bricksim::ldr {
    std::map<std::string, std::shared_ptr<File>> readComplexFile(const std::string& name, const std::string& content, FileType mainFileType);
    std::shared_ptr<File> readSimpleFile(const std::string& name, const std::string& content, FileType type);
}