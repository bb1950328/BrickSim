#include "../../../gui/windows/windows.h"
#include "catch2/catch_test_macros.hpp"

namespace bricksim {
    TEST_CASE("windowDataCorrect") {
        const auto& data = gui::windows::getData();
        const auto idCount = magic_enum::enum_count<gui::windows::Id>();
        for (size_t i = 0; i < idCount; ++i) {
            const auto id = magic_enum::enum_value<gui::windows::Id>(i);
            CHECK(static_cast<int>(i) == magic_enum::enum_integer(id));
            CHECK(data[i].id == id);
            CHECK(data[i].drawFunction != nullptr);
            CHECK(&data[i].visible == gui::windows::isVisible(id));
        }
    }
}
