// part_finder.cpp
// Created by bab21 on 15.11.20.
//

#include "part_finder.h"

#include <utility>
#include <algorithm>

namespace part_finder {
    namespace {
        std::map<std::string, Predicate> predicates;
    }

    const Predicate &getPredicate(const std::string &expression) {
        auto it = predicates.find(expression);
        if (it==predicates.end()) {
            for (auto i = expression.length()-1; i > std::max(0lu, expression.length()-5); --i) {
                predicates.erase(expression.substr(0, i));
            }
            return predicates.emplace(expression, Predicate(expression)).first->second;
        }
        return it->second;
    }

    Predicate::Predicate(std::string expression) : expression(std::move(expression)) {}

    bool Predicate::matches(const LdrFile &part) const {
        return (util::containsIgnoreCase(part.metaInfo.title, expression)
                || util::containsIgnoreCase(part.metaInfo.name, expression)
                || util::containsIgnoreCase(part.metaInfo.theme, expression)
                || std::any_of(part.metaInfo.keywords.begin(), part.metaInfo.keywords.end(), [this](const auto &keyword) { return util::containsIgnoreCase(keyword, expression); }
                )
        );
    }
}

