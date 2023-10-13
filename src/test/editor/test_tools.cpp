#include "../../editor/tools.h"
#include "../testing_tools.h"

namespace bricksim::tools {
    TEST_CASE("tools::getData") {
        const auto tool = GENERATE(enumGenerator<Tool>());
        const auto& data = getData(tool);
        CHECK(data.tool == tool);
        CHECK(data.nameWithIcon.find(data.name) != std::string::npos);
    }
}
