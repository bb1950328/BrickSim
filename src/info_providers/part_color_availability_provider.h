#ifndef BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H
#define BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H


#include <optional>
#include <set>
#include <memory>
#include "../ldr_files/ldr_colors.h"
#include "../ldr_files/ldr_files.h"

namespace bricksim::info_providers::part_color_availability {
    std::optional<std::set<ldr::ColorReference>> getAvailableColorsForPart(const std::shared_ptr<ldr::File>& part);
}

#endif //BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H
