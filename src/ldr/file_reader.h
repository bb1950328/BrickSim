#pragma once
#include "files.h"

namespace bricksim::ldr {
    uomap_t<std::string, std::shared_ptr<File>> readComplexFile(const std::shared_ptr<FileNamespace>& fileNamespace,
                                                                const std::string& name,
                                                                const std::filesystem::path& source,
                                                                FileType mainFileType,
                                                                const std::string& content,
                                                                const std::optional<std::string>& shadowContent);
    std::shared_ptr<File> readSimpleFile(const std::shared_ptr<FileNamespace>& fileNamespace,
                                         std::string_view name,
                                         const std::filesystem::path& source,
                                         FileType type,
                                         const std::string& content,
                                         const std::optional<std::string>& shadowContent);
}
