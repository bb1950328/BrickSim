#pragma once
#include "data.h"

namespace bricksim::config {
    void initialize();
    void save();

    [[nodiscard]] Config& getMutable();
}
