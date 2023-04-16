#pragma once

#include "../data.h"
#include <string>
namespace bricksim::connection::visualization {
    std::string generateGraphviz(const ConnectionGraph& graph);
}
