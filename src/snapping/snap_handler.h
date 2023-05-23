#pragma once

#include <string>
#include <vector>
namespace bricksim::snap {
    struct LinearSnapStepPreset {
        std::string name;
        int stepXZ;
        int stepY;
        LinearSnapStepPreset(std::string name, int distanceXz, int distanceY);
        LinearSnapStepPreset(const LinearSnapStepPreset& other) = default;
        LinearSnapStepPreset& operator=(const LinearSnapStepPreset& other) = default;
        LinearSnapStepPreset(LinearSnapStepPreset&& other) = default;
        LinearSnapStepPreset& operator=(LinearSnapStepPreset&& other) = default;
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
    class Handler {
        LinearHandler linear;
        bool enabled = true;

    public:
        [[nodiscard]] LinearHandler& getLinear();
        [[nodiscard]] bool isEnabled() const;
        [[nodiscard]] bool* isEnabledPtr();

        void init();
        void cleanup();

        Handler();
        Handler(const Handler& other) = delete;
        Handler& operator=(const Handler& other) = delete;
    };
}
