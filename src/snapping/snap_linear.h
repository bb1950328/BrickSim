#pragma once
#include "../config/data.h"
#include <vector>

namespace bricksim::snap {
    class LinearHandler {
        config::SnappingLinearStepPreset temporaryPreset = {"", 1, 1};
        int currentPresetIndex = TEMPORARY_PRESET_INDEX;

    public:
        static constexpr int TEMPORARY_PRESET_INDEX = -1;
        [[nodiscard]] int getCurrentPresetIndex() const;
        void setCurrentPresetIndex(int newPreset);
        [[nodiscard]] const config::SnappingLinearStepPreset& getCurrentPreset() const;
        [[nodiscard]] const config::SnappingLinearStepPreset& getTemporaryPreset() const;
        void setTemporaryPreset(const config::SnappingLinearStepPreset& newTmpPreset);

        static glm::ivec3 getStepXYZ(const config::SnappingLinearStepPreset& preset);
        static std::optional<gui::icons::IconType> getIcon(const config::SnappingLinearStepPreset& preset);

        void init();
        void cleanup() const;
    };
}
