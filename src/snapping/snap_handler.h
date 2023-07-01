#pragma once

#include "glm/glm.hpp"
#include "snap_linear.h"
#include "snap_rotational.h"
#include <string>
#include <vector>

namespace bricksim::snap {
    class Handler {
        LinearHandler linear;
        RotationalHandler rotational;
        bool enabled = true;

    public:
        [[nodiscard]] LinearHandler& getLinear();
        [[nodiscard]] RotationalHandler& getRotational();
        [[nodiscard]] const LinearHandler& getLinear() const;
        [[nodiscard]] const RotationalHandler& getRotational() const;
        [[nodiscard]] bool isEnabled() const;
        [[nodiscard]] bool* isEnabledPtr();

        void init();
        void cleanup();

        Handler();
        Handler(const Handler& other) = delete;
        Handler& operator=(const Handler& other) = delete;
    };
}
