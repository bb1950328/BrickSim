// util.cpp
// Created by bab21 on 20.09.20.
//

#include <algorithm>
#include "util.h"

namespace util {
    std::string extend_home_dir(const std::string &input) {
        std::string output = std::string(input);
        auto home_pos = output.find('~');
        if (home_pos != std::string::npos) {

            output.replace(home_pos, 1, getenv(USER_ENV_VAR));
        }
        return output;
    }

    std::string trim(const std::string &input) {
        auto wsbefore = std::find_if_not(input.begin(), input.end(), [](int c) { return std::isspace(c); });
        auto wsafter = std::find_if_not(input.rbegin(), input.rend(), [](int c) { return std::isspace(c); }).base();
        return (wsafter <= wsbefore ? std::string() : std::string(wsbefore, wsafter));
    }

    std::string pathjoin(const std::list<std::string> &parts) {
        auto result = std::string();
        for (const auto &part : parts) {
            result.append(part);
            if (result.back() != PATH_SEPARATOR) {
                result.push_back(PATH_SEPARATOR);
            }
        }
        result.pop_back();//remove last separator
        std::replace(result.begin(), result.end(), PATH_SEPARATOR_FOREIGN, PATH_SEPARATOR);
        return result;
    }

    std::string as_lower(const std::string &string) {
        auto result = std::string();
        for (const auto &ch: string) {
            result.push_back(std::tolower(ch));
        }
        return result;
    }

    bool ends_with(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    bool starts_with(std::string const &fullString, std::string const &start) {
        return fullString.rfind(start, 0) == 0;
    }
}