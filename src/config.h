//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_CONFIG_H
#define BRICKSIM_CONFIG_H

#include <map>

namespace config {
    extern std::map<std::string, std::string> strings;
    extern std::map<std::string, long> longs;
    extern std::map<std::string, double> doubles;

    [[nodiscard]] std::string get_string(const std::string &key);
    [[nodiscard]] long get_long(const std::string &key);
    [[nodiscard]] double get_double(const std::string &key);

    void set_string(const std::string &key, const std::string &value);
    void set_long(const std::string &key, long value);
    void set_double(const std::string &key, double value);

    bool save();
    void _ensure_settings_loaded();

    const std::string KEY_LDRAW_PARTS_LIBRARY = "ldrawPartsLibrary";
    const std::string KEY_SCREEN_WIDTH = "screenWidth";
    const std::string KEY_SCREEN_HEIGHT = "screenHeight";
    const std::string KEY_INSTANCED_MIN_COMPLEXITY = "instancedMinComplexity";
    const std::string KEY_MSAA_SAMPLES = "msaaSamples";
    const std::string KEY_GUI_SCALE = "guiScale";
    const std::string KEY_GUI_STYLE = "guiStyle";
    const std::string KEY_BACKGROUND_COLOR = "backgroundColor";
}

#endif //BRICKSIM_CONFIG_H
