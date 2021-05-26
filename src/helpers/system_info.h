#ifndef BRICKSIM_SYSTEM_INFO_H
#define BRICKSIM_SYSTEM_INFO_H

#include <string>
#include <vector>

namespace system_info {
    std::vector<std::pair<std::string, std::string>> getSystemInfo();
}

#endif //BRICKSIM_SYSTEM_INFO_H
