// part_color_availability_provider.h
// Created by bab21 on 01.11.20.
//

#ifndef BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H
#define BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H

#include <set>
#include "../ldr_colors.h"
#include "../ldr_files.h"

namespace part_color_availability_provider {
    std::optional<std::set<const LdrColor *>> getAvailableColorsForPart(LdrFile *part);
}

#endif //BRICKSIM_PART_COLOR_AVAILABILITY_PROVIDER_H
