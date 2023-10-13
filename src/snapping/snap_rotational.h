#pragma once
#include "snap_common.h"
#include <string>
#include <vector>

namespace bricksim::snap {
    struct RotationalSnapStepPreset : public SnapStepPreset {
        float stepDeg;
        RotationalSnapStepPreset(std::string name, float stepDeg);
        std::optional<gui::icons::IconType> getIcon() const override;
    };

    class RotationalHandler {
        std::vector<RotationalSnapStepPreset> presets;
        RotationalSnapStepPreset temporaryPreset = {"", 1.f};
        int currentPresetIndex = TEMPORARY_PRESET_INDEX;

    public:
        static const int TEMPORARY_PRESET_INDEX = -1;
        [[nodiscard]] const std::vector<RotationalSnapStepPreset>& getPresets() const;
        [[nodiscard]] const RotationalSnapStepPreset& getTemporaryPreset() const;
        void setTemporaryPreset(const RotationalSnapStepPreset& value);
        [[nodiscard]] int getCurrentPresetIndex() const;
        void setCurrentPresetIndex(int value);
        [[nodiscard]] const RotationalSnapStepPreset& getCurrentPreset() const;

        void init();
        void cleanup();

        float getNearestValue(float initialValueDeg, float currentValueDeg) const;
    };
}
