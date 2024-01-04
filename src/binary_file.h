#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <filesystem>

namespace bricksim {
    class BinaryFile {
    public:
        std::string name;
        std::vector<uint8_t> data;

        explicit BinaryFile(std::string name);
        explicit BinaryFile(const std::filesystem::path& path);
    };
}
