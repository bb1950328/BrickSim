#include "../../gui/icons.h"
#include "../testing_tools.h"
#include <utf8.h>

namespace bricksim::gui::icons {
    TEST_CASE("icons::") {
        const IconType type = GENERATE(enumGenerator<IconType>());
        const IconSize size = GENERATE(enumGenerator<IconSize>());

        SECTION("getRawImage") {
            const auto& rawImage = getRawImage(type, size);
            CHECK(size == rawImage.height);
            CHECK(size == rawImage.width);
            CHECK(4 == rawImage.channels);
            CHECK(static_cast<std::size_t>(rawImage.width * rawImage.height * rawImage.channels) == rawImage.data.size());
        }

        SECTION("getHotPoint") {
            const auto hotPoint = getHotPoint(type, size);
            CHECK(0 <= hotPoint.x);
            CHECK(0 <= hotPoint.y);
            CHECK(hotPoint.x < size);
            CHECK(hotPoint.y < size);
        }

        SECTION("getGlyphCodepoint") {
            const auto codepoint = getGlyphCodepoint(type, size);
            CHECK(GLYPH_RANGE_START <= codepoint);
            CHECK(codepoint <= GLYPH_RANGE_END);

            const auto glyph = getGlyph(type, size);
            std::u32string u32glyph;
            u32glyph.push_back(codepoint);
            const auto expectedGlyph = utf8::utf32to8(u32glyph);
            CHECK(expectedGlyph == glyph);

            const auto utf8Bytes = getGlyphUtf8Bytes(type, size);
            std::string bytesAsStr;
            bytesAsStr.push_back((utf8Bytes & 0xff0000) >> 16);
            bytesAsStr.push_back((utf8Bytes & 0x00ff00) >> 8);
            bytesAsStr.push_back((utf8Bytes & 0x0000ff));
            CHECK(expectedGlyph == bytesAsStr);
        }
    }
}
