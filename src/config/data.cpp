#include "data.h"

namespace bricksim::config {
    const std::vector<SnappingLinearStepPreset> Snapping::DEFAULT_LINEAR_PRESETS = {
            {"Brick", 20, 24},
            {"Technic", 20, 20},
            {"Plate/Half Brick", 10, 8},
            {"Half Technic", 10, 10},
    };
    const std::vector<SnappingRotationalStepPreset> Snapping::DEFAULT_ROTATIONAL_PRESETS = {
            {"1/4", 90},
            {"1/6", 60},
            {"1/8", 45},
            {"1/16", 22.5f},
    };
}
