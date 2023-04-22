#pragma once

#include <array>
#include <filesystem>
namespace bricksim::graphviz_wrapper {
    constexpr std::array<char const*, 4> OUTPUT_FILE_FILTER_PATTERNS = {"*.svg", "*.png", "*.pdf", "*.bmp"};
    bool isAvailable();
    void renderDot(const std::filesystem::path& outputPath, std::string_view dot);
}
