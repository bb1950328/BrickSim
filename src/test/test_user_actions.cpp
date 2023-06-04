#include "../../user_actions.h"
#include "testing_tools.h"

namespace bricksim {
    TEST_CASE("ActionData") {
        const auto action = GENERATE(enumGenerator<user_actions::Action>());
        const auto data = user_actions::getData(action);
        CAPTURE(magic_enum::enum_name(action));
        CHECK(action == data.action);
    }
}
