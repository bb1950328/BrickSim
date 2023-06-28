#pragma once

#include <string>
namespace bricksim::snap {
    constexpr auto DEFAULT_LINEAR_SNAP_DISTANCE_PRESETS = "["
                                                          R"({"name":"Brick", "xz": 20, "y": 24},)"
                                                          R"({"name":"Technic", "xz": 20, "y": 20},)"
                                                          R"({"name":"Plate/Half Brick", "xz": 10, "y": 8},)"
                                                          R"({"name":"Half Technic", "xz": 10, "y": 10})"
                                                          "]";
    constexpr auto DEFAULT_ROTATIONAL_SNAP_PRESETS = "["
                                                     R"({"name": "1/4", "step": 90},)"
                                                     R"({"name": "1/6", "step": 60},)"
                                                     R"({"name": "1/8", "step": 45},)"
                                                     R"({"name": "1/16", "step": 22.5})"
                                                     "]";
}
