#include "binary_file.h"
#include <spdlog/spdlog.h>

#include <utility>

namespace bricksim {
    BinaryFile::BinaryFile(std::string name) :
        name(std::move(name)) {
    }

    BinaryFile::BinaryFile(const std::filesystem::path& path) :
        name(path.string()) {
        FILE* f = fopen(path.string().c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            unsigned long length = ftell(f);
            fseek(f, 0, SEEK_SET);
            data.resize(length);
            size_t bytesRead = fread(&data[0], 1, length, f);
            if (bytesRead != length) {
                spdlog::warn("reading file {}: {} bytes read, but reported size is {} bytes.", path.string(), bytesRead, length);
                data.resize(bytesRead);
            }
            fclose(f);
        } else {
            spdlog::error("can't read file {}: ", path.string(), strerror(errno));
            throw std::invalid_argument(strerror(errno));
        }
    }
}
