#include "catch2/catch.hpp"
#include "../../../gui/windows/windows.h"

TEST_CASE("windowDataCorrect") {
    const auto& data = gui::windows::getData();
    const auto idCount = magic_enum::enum_count<gui::windows::Id>();
    for (int i = 0; i < idCount; ++i) {
        const auto id = magic_enum::enum_value<gui::windows::Id>(i);
        CHECK(i == magic_enum::enum_integer(id));
        CHECK(data[i].id == id);
        CHECK(data[i].drawFunction != nullptr);
        CHECK(&data[i].visible == gui::windows::isVisible(id));
    }
}