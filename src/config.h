//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_CONFIG_H
#define BRICKSIM_CONFIG_H

#include <map>
#include "helpers/util.h"

namespace config {
    struct Key {
        const std::string name;
        explicit Key(std::string name);
        bool operator==(const Key& other) const;
    };
    
    [[nodiscard]] std::string getString(const Key& key);
    [[nodiscard]] long getLong(const Key& key);
    [[nodiscard]] double getDouble(const Key& key);
    [[nodiscard]] util::RGBcolor getColor(const Key& key);
    [[nodiscard]] bool getBool(const Key& key);

    void setString(const Key& key, const std::string &value);
    void setLong(const Key& key, long value);
    void setDouble(const Key& key, double value);
    void setColor(const Key& key, util::RGBcolor value);
    void setBool(const Key& key, bool value);

    void exportToTxt();
    void importFromTxt();
    
    const Key LDRAW_PARTS_LIBRARY{"ldrawPartsLibrary"};
    const Key SCREEN_WIDTH{"screenWidth"};
    const Key SCREEN_HEIGHT{"screenHeight"};
    const Key INSTANCED_MIN_COMPLEXITY{"instancedMinComplexity"};
    const Key MSAA_SAMPLES{"msaaSamples"};
    const Key GUI_SCALE{"guiScale"};
    const Key GUI_STYLE{"guiStyle"};
    const Key BACKGROUND_COLOR{"backgroundColor"};
    const Key SHOW_NORMALS{"showNormals"};
    const Key DISPLAY_SELECTION_BUFFER{"displaySelectionBuffer"};
    const Key JPG_SCREENSHOT_QUALITY{"jpgScreenshotQuality"};
    const Key COLOR_MULTI_PART_DOCUMENT{"colorMultiPartDocument"};
    const Key COLOR_MPD_SUBFILE{"colorMpdSubfile"};
    const Key COLOR_MPD_SUBFILE_INSTANCE{"colorMpdSubfileInstance"};
    const Key COLOR_OFFICAL_PART{"colorOfficalPart"};
    const Key COLOR_UNOFFICAL_PART{"colorUnofficalPart"};
    const Key THUMBNAIL_SIZE{"thumbnailSize"};
    const Key THUMBNAIL_CACHE_SIZE_BYTES{"thumbnailCacheSizeBytes"};
    const Key DRAW_MINIMAL_ENCLOSING_BALL_LINES{"drawMinimalEnclosingBallLines"};
    const Key BRICKLINK_CURRENCY_CODE{"bricklinkCurrencyCode"};
}

#endif //BRICKSIM_CONFIG_H
