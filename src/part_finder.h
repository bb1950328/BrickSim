#pragma once

#include "ldr/files.h"

namespace bricksim::part_finder {
    class Predicate {
        //todo add more sophisticated search
    private:
        std::string expression;

    public:
        [[nodiscard]] bool matches(const ldr::File& part) const;
        explicit Predicate(std::string expression);
    };

    const Predicate& getPredicate(const std::string& expression);
}
