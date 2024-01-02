#pragma once
#include "../gui/icons.h"
#include "snap_common.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace bricksim::snap {
    struct LinearSnapStepPreset : public SnapStepPreset {
        int stepXZ;
        int stepY;
        [[nodiscard]] glm::ivec3 stepXYZ() const;
        LinearSnapStepPreset(std::string name, int distanceXz, int distanceY);
        [[nodiscard]] std::optional<gui::icons::IconType> getIcon() const;
    };
    class LinearHandler {
        std::vector<LinearSnapStepPreset> presets;
        LinearSnapStepPreset temporaryPreset = {"", 1, 1};
        int currentPresetIndex = TEMPORARY_PRESET_INDEX;

    public:
        static const int TEMPORARY_PRESET_INDEX = -1;
        [[nodiscard]] const std::vector<LinearSnapStepPreset>& getPresets() const;
        [[nodiscard]] int getCurrentPresetIndex() const;
        void setCurrentPresetIndex(int newPreset);
        [[nodiscard]] const LinearSnapStepPreset& getCurrentPreset() const;
        [[nodiscard]] const LinearSnapStepPreset& getTemporaryPreset() const;
        void setTemporaryPreset(const LinearSnapStepPreset& newTmpPreset);

        void init();
        void cleanup();
    };

}
