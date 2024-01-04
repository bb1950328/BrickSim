#include "snap_rotational.h"

#include "../persistent_state.h"
#include "../config/read.h"
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <utility>

namespace bricksim::snap {
    std::optional<gui::icons::IconType> RotationalHandler::getIcon(const config::SnappingRotationalStepPreset& preset) {
        using gui::icons::IconType;
        constexpr std::array sizes = {90.f, 60.f, 45.f, 22.5f};
        constexpr std::array icons = {IconType::Pie4, IconType::Pie6, IconType::Pie8, IconType::Pie16};
        static_assert(sizes.size() == icons.size());
        const auto it = std::find_if(sizes.begin(), sizes.end(), [&](const auto& item) {
            return std::abs(item - preset.step) < .1f;
        });
        return it != sizes.end()
                   ? std::make_optional<IconType>(icons[std::distance(sizes.begin(), it)])
                   : std::nullopt;
    }

    const config::SnappingRotationalStepPreset& RotationalHandler::getTemporaryPreset() const {
        return temporaryPreset;
    }

    int RotationalHandler::getCurrentPresetIndex() const {
        return currentPresetIndex;
    }

    void RotationalHandler::setCurrentPresetIndex(int value) {
        currentPresetIndex = value;
    }

    void RotationalHandler::setTemporaryPreset(const config::SnappingRotationalStepPreset& value) {
        temporaryPreset = value;
    }

    void RotationalHandler::init() {
        const float configStep = persisted_state::get().snapping.rotationalStep;
        auto& presets = config::get().snapping.rotationalPresets;
        for (size_t i = 0; i < presets.size(); ++i) {
            if (std::fabs(presets[i].step - configStep) < .01f) {
                currentPresetIndex = static_cast<int>(i);
                break;
            }
        }
        if (currentPresetIndex == TEMPORARY_PRESET_INDEX) {
            setTemporaryPreset({"", configStep});
        }
    }

    void RotationalHandler::cleanup() const {
        persisted_state::get().snapping.rotationalStep = getCurrentPreset().step;
    }

    float RotationalHandler::getNearestValue(const float initialValueDeg, const float currentValueDeg) const {
        const auto step = getCurrentPreset().step;
        const auto delta = currentValueDeg - initialValueDeg;
        const auto factor = std::round(delta / step);
        return std::fmod(initialValueDeg + factor * step, 360.f);
    }

    const config::SnappingRotationalStepPreset& RotationalHandler::getCurrentPreset() const {
        return config::get().snapping.rotationalPresets[currentPresetIndex];
    }
}
