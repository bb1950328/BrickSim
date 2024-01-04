#include "snap_linear.h"

#include "../persistent_state.h"
#include "../config/read.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

namespace bricksim::snap {
    void LinearHandler::init() {
        const auto configXZ = persisted_state::get().snapping.linearStepXZ;;
        const auto configY = persisted_state::get().snapping.linearStepY;
        auto& presets = config::get().snapping.linearPresets;
        for (size_t i = 0; i < presets.size(); ++i) {
            if (presets[i].stepXZ == configXZ && presets[i].stepY == configY) {
                currentPresetIndex = i;
                break;
            }
        }
        if (currentPresetIndex == TEMPORARY_PRESET_INDEX) {
            setTemporaryPreset({"", configXZ, configY});
        }
    }

    int LinearHandler::getCurrentPresetIndex() const {
        return currentPresetIndex;
    }

    void LinearHandler::setCurrentPresetIndex(int newPreset) {
        currentPresetIndex = newPreset;
    }

    const config::SnappingLinearStepPreset& LinearHandler::getCurrentPreset() const {
        return currentPresetIndex < 0
                   ? temporaryPreset
                   : config::get().snapping.linearPresets[currentPresetIndex];
    }

    void LinearHandler::setTemporaryPreset(const config::SnappingLinearStepPreset& newTmpPreset) {
        temporaryPreset = newTmpPreset;
    }

    void LinearHandler::cleanup() const {
        persisted_state::get().snapping.linearStepXZ = getCurrentPreset().stepXZ;
        persisted_state::get().snapping.linearStepY = getCurrentPreset().stepY;
    }

    const config::SnappingLinearStepPreset& LinearHandler::getTemporaryPreset() const {
        return temporaryPreset;
    }

    glm::ivec3 LinearHandler::getStepXYZ(const config::SnappingLinearStepPreset& preset) {
        return {preset.stepXZ, preset.stepY, preset.stepXZ};
    }

    std::optional<gui::icons::IconType> LinearHandler::getIcon(const config::SnappingLinearStepPreset& preset) {
        using gui::icons::IconType;
        if (preset.stepXZ == 20) {
            if (preset.stepY == 24) {
                return IconType::Brick1Side;
            } else if (preset.stepY == 20) {
                return IconType::Brick1x1Top;
            }
        } else if (preset.stepXZ == 10) {
            if (preset.stepY == 8) {
                return IconType::Plate1Side;
            } else if (preset.stepY == 10) {
                return IconType::Plate1HalfTop;
            }
        }
        return std::nullopt;
    }
}
