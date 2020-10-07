// util.h
// Created by bb1950328 on 20.09.20.
//

#ifndef BRICKSIM_UTIL_H
#define BRICKSIM_UTIL_H

#include <string>
#include <list>
#include <filesystem>

namespace util {
#if _WIN32
    const char *const USER_ENV_VAR = "USERPROFILE";
    const char PATH_SEPARATOR = '\\';
    const char PATH_SEPARATOR_FOREIGN = '/';
#else
    const char *const USER_ENV_VAR = "HOME";
    const char PATH_SEPARATOR = '/';
    const char PATH_SEPARATOR_FOREIGN = '\\';
#endif

    std::string extend_home_dir(const std::string &input);
    std::filesystem::path extend_home_dir_path(const std::string &input);
    std::string trim(const std::string& input);
    std::string pathjoin(const std::list<std::string>& parts);
    std::string as_lower(const std::string& string);
    bool ends_with(std::string const &fullString, std::string const &ending);
    bool starts_with(std::string const &fullString, std::string const &start);
    void cout_mat4(glm::mat4 mat);
    void replaceAll(std::string& str, const std::string& from, const std::string& to);
    std::string replaceChar(const std::string& str, char from, char to);
}
#endif //BRICKSIM_UTIL_H
