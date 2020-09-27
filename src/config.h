//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_CONFIG_H
#define BRICKSIM_CONFIG_H

#include <map>

class Configuration {
private:
    static Configuration *instance;

private:
    std::map<std::string, std::string> strings;
    std::map<std::string, long> longs;
    std::map<std::string, double> doubles;
    Configuration();
public:
    static Configuration* getInstance();

    std::string get_string(const std::string &key) const;

    long get_long(const std::string &key) const;

    double get_double(const std::string &key) const;
};

namespace config {
    const std::string KEY_LDRAW_PARTS_LIBRARY = "ldrawPartsLibrary";
    const std::string KEY_SCREEN_WIDTH = "screenWidth";
    const std::string KEY_SCREEN_HEIGHT = "screenHeight";
    const std::string KEY_INSTANCED_MIN_COMPLEXITY = "instancedMinComplexity"
}

#endif //BRICKSIM_CONFIG_H
