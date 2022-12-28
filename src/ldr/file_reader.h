#pragma once
#include "files.h"

namespace bricksim::ldr {
    uomap_t<std::string, std::shared_ptr<File>> readComplexFile(const std::string& name,
                                                                FileType mainFileType,
                                                                const std::string& content,
                                                                const std::optional<std::string>& shadowContent);
    std::shared_ptr<File> readSimpleFile(std::string_view name,
                                         FileType type,
                                         const std::string& content,
                                         const std::optional<std::string>& shadowContent);
}
