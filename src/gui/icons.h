#pragma once
#include "../constant_data/resources.h"
#include "../helpers/util.h"
#include "imgui.h"
#include "magic_enum.hpp"
#include <span>

namespace bricksim::gui::icons {
    enum IconType {
        Brick1Side,
        Brick1x1Top,
        Brick2Side,
        Brick2x2Top,
        Pie4,
        Pie6,
        Pie8,
        Pie16,
        Plate1HalfTop,
        Plate1Side,
        Plate2Side,
        Select,
        SelectConnected,
        SelectStronglyConnected,
    };

    enum IconSize {
        Icon16 = 16,
        Icon36 = 36,
        Icon48 = 48,
    };

    constexpr uint16_t TYPE_COUNT = magic_enum::enum_count<IconType>();
    constexpr uint16_t SIZE_COUNT = magic_enum::enum_count<IconSize>();
    constexpr uint16_t GLYPH_COUNT = TYPE_COUNT * SIZE_COUNT;

    constexpr uint16_t GLYPH_RANGE_START = 0xf900;
    constexpr uint16_t GLYPH_RANGE_END = GLYPH_RANGE_START + GLYPH_COUNT;

    constexpr uint16_t getGlyphIndex(IconType iconType, IconSize iconSize) {
        return *magic_enum::enum_index(iconType) + TYPE_COUNT * *magic_enum::enum_index(iconSize);
    }

    constexpr uint16_t getGlyphCodepoint(IconType iconType, IconSize iconSize) {
        return GLYPH_RANGE_START + getGlyphIndex(iconType, iconSize);
    }

    constexpr int getGlyphUtf8Bytes(IconType type, IconSize size) {
        return 0xEFA480 + getGlyphIndex(type, size);
    }

    namespace {
        using glyph_string_t = std::array<std::array<char, 4>, GLYPH_COUNT>;

        constexpr glyph_string_t createGlyphStrings() {
            glyph_string_t result{};
            auto it = result.begin();
            for (const auto size: magic_enum::enum_values<IconSize>()) {
                for (const auto type: magic_enum::enum_values<IconType>()) {
                    //const auto value = getGlyphNumber(type);
                    auto utf8 = getGlyphUtf8Bytes(type, size);
                    assert(utf8 < 0xffffff);
                    it->data()[3] = '\0';
                    it->data()[2] = utf8 & 0xff;
                    utf8 >>= 8;
                    it->data()[1] = utf8 & 0xff;
                    utf8 >>= 8;
                    it->data()[0] = utf8 & 0xff;
                    ++it;
                }
            }
            return result;
        }

        constexpr glyph_string_t glyphStrings = createGlyphStrings();
    }

    constexpr auto PNG_FILES = std::to_array({
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::brick_1_side_png, resources::icons::_36x36::brick_1_side_png, resources::icons::_48x48::brick_1_side_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::brick_1x1_top_png, resources::icons::_36x36::brick_1x1_top_png, resources::icons::_48x48::brick_1x1_top_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::brick_2_side_png, resources::icons::_36x36::brick_2_side_png, resources::icons::_48x48::brick_2_side_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::brick_2x2_top_png, resources::icons::_36x36::brick_2x2_top_png, resources::icons::_48x48::brick_2x2_top_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::pie4_png, resources::icons::_36x36::pie4_png, resources::icons::_48x48::pie4_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::pie6_png, resources::icons::_36x36::pie6_png, resources::icons::_48x48::pie6_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::pie8_png, resources::icons::_36x36::pie8_png, resources::icons::_48x48::pie8_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::pie16_png, resources::icons::_36x36::pie16_png, resources::icons::_48x48::pie16_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::plate_1_half_top_png, resources::icons::_36x36::plate_1_half_top_png, resources::icons::_48x48::plate_1_half_top_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::plate_1_side_png, resources::icons::_36x36::plate_1_side_png, resources::icons::_48x48::plate_1_side_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::plate_2_side_png, resources::icons::_36x36::plate_2_side_png, resources::icons::_48x48::plate_2_side_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::select_png, resources::icons::_36x36::select_png, resources::icons::_48x48::select_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::select_connected_png, resources::icons::_36x36::select_connected_png, resources::icons::_48x48::select_connected_png}),
            std::to_array<std::span<const uint8_t>>({resources::icons::_16x16::select_strongly_connected_png, resources::icons::_36x36::select_strongly_connected_png, resources::icons::_48x48::select_strongly_connected_png}),
    });
    static_assert(PNG_FILES.size() == TYPE_COUNT);

    constexpr std::span<const uint8_t> getPNG(IconType icon, IconSize size) {
        return PNG_FILES[*magic_enum::enum_index(icon)][*magic_enum::enum_index(size)];
    }
    constexpr const char* getGlyph(IconType icon, IconSize size) {
        return glyphStrings[*magic_enum::enum_index(icon) + TYPE_COUNT * *magic_enum::enum_index(size)].data();
    }

    [[nodiscard]] util::RawImage getRawImage(IconType icon, IconSize size);
    [[nodiscard]] glm::vec2 getHotPoint(IconType icon);
    [[nodiscard]] glm::ivec2 getHotPoint(IconType icon, IconSize size);

    void initialize();
    void cleanup();
}
