#pragma once
#include "../config/data.h"

namespace bricksim::snap {
    class RotationalHandler {
        config::SnappingRotationalStepPreset temporaryPreset = {"", 1.f};
        int currentPresetIndex = TEMPORARY_PRESET_INDEX;

    public:
        static constexpr int TEMPORARY_PRESET_INDEX = -1;
        static std::optional<gui::icons::IconType> getIcon(const config::SnappingRotationalStepPreset& preset);
        [[nodiscard]] const config::SnappingRotationalStepPreset& getTemporaryPreset() const;
        void setTemporaryPreset(const config::SnappingRotationalStepPreset& value);
        [[nodiscard]] int getCurrentPresetIndex() const;
        void setCurrentPresetIndex(int value);
        [[nodiscard]] const config::SnappingRotationalStepPreset& getCurrentPreset() const;

        void init();
        void cleanup() const;

        float getNearestValue(float initialValueDeg, float currentValueDeg) const;
    };
}
