#include "snap_common.h"
#include "spdlog/fmt/bundled/format.h"

namespace bricksim::snap {
    std::string SnapStepPreset::getNameWithIcon() const {
        const auto type = getIcon();
        return type.has_value()
                   ? fmt::format("{} {}", gui::icons::getGlyph(*type, gui::icons::Icon36), name)
                   : name;
    }

    SnapStepPreset::SnapStepPreset(const std::string& name) :
        name(name) {}
}
