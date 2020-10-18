//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_CONFIG_H
#define BRICKSIM_CONFIG_H

#include <map>

namespace config {
    struct Key {
        const std::string name;
    };
    
    extern std::map<std::string, std::string> strings;
    extern std::map<std::string, long> longs;
    extern std::map<std::string, double> doubles;
    
    [[nodiscard]] std::string get_string(const Key& key);
    [[nodiscard]] long get_long(const Key& key);
    [[nodiscard]] double get_double(const Key& key);
    
    void set_string(const Key& key, const std::string &value);
    void set_long(const Key& key, long value);
    void set_double(const Key& key, double value);
    
    bool save();
    void _ensure_settings_loaded();
    
    const Key LDRAW_PARTS_LIBRARY{"ldrawPartsLibrary"};
    const Key SCREEN_WIDTH{"screenWidth"};
    const Key SCREEN_HEIGHT{"screenHeight"};
    const Key INSTANCED_MIN_COMPLEXITY{"instancedMinComplexity"};
    const Key MSAA_SAMPLES{"msaaSamples"};
    const Key GUI_SCALE{"guiScale"};
    const Key GUI_STYLE{"guiStyle"};
    const Key BACKGROUND_COLOR{"backgroundColor"};
    const Key SHOW_NORMALS{"showNormals"};
}

#endif //BRICKSIM_CONFIG_H
