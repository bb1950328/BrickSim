#ifndef BRICKSIM_PART_FINDER_H
#define BRICKSIM_PART_FINDER_H

#include "ldr_files/ldr_files.h"

namespace part_finder {
    class Predicate {
        //todo add more sophisticated search
    private:
        std::string expression;
    public:
        [[nodiscard]] bool matches(const LdrFile& part) const;
        explicit Predicate(std::string expression);
    };

    const Predicate& getPredicate(const std::string& expression);
}

#endif //BRICKSIM_PART_FINDER_H
