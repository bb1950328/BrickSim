// part_color_availability_provider.h
// Created by bab21 on 01.11.20.
//

#ifndef BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H
#define BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H

#include <set>
#include "../ldr_files/ldr_colors.h"
#include "../ldr_files/ldr_files.h"

namespace part_color_availability_provider {
    std::optional<std::set<LdrColorReference>> getAvailableColorsForPart(const std::shared_ptr<LdrFile>& part);
}

#endif //BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H
