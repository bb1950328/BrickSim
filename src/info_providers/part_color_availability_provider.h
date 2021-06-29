#pragma once

#include "../ldr/colors.h"
#include "../ldr/files.h"
#include <memory>
#include <optional>
#include <set>

namespace bricksim::info_providers::part_color_availability {
    std::optional<std::set<ldr::ColorReference>> getAvailableColorsForPart(const std::shared_ptr<ldr::File>& part);
}
