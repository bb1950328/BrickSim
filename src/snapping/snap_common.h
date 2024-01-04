#pragma once
#include "../gui/icons.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace bricksim::snap {
    class SnapStepPreset {
    public:
        std::string name;

        explicit SnapStepPreset(const std::string& name);

        [[nodiscard]] virtual std::optional<gui::icons::IconType> getIcon() const = 0;
        [[nodiscard]] std::string getNameWithIcon() const;
    };
}
