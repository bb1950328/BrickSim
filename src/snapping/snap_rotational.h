#pragma once
#include <string>
#include <vector>

namespace bricksim::snap {
    struct RotationalSnapStepPreset {
        std::string name;
        float stepDeg;
        RotationalSnapStepPreset(std::string name, float stepDeg);
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
