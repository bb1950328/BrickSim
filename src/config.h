//
// Created by bb1950328 on 19.09.20.
//

#ifndef BRICKSIM_CONFIG_H
#define BRICKSIM_CONFIG_H

#include <map>
#include "helpers/util.h"

namespace config {
    class Key {
    public:
        const std::string name;
        bool operator==(const Key& other) const;
    protected:
        explicit Key(std::string name);
    };
    class StringKey : public Key {
    public:
        const std::string defaultValue;
        StringKey(const std::string &name, std::string defaultValue);
    };
    class IntKey : public Key {
    public:
        const int defaultValue;
        IntKey(const std::string &name, int defaultValue);
    };
    class DoubleKey : public Key {
    public:
        const double defaultValue;
        DoubleKey(const std::string &name, double defaultValue);
    };
    class ColorKey : public StringKey {
    public:
        const util::RGBcolor defaultValue;
        ColorKey(const std::string &name, const util::RGBcolor &defaultValue);
    };
    class BoolKey : public Key {
    public:
        const bool defaultValue;
        BoolKey(const std::string &name, bool defaultValue);
    };
    
    [[nodiscard]] std::string getString(const StringKey& key);
    [[nodiscard]] int getInt(const IntKey &key);
    [[nodiscard]] double getDouble(const DoubleKey &key);
    [[nodiscard]] util::RGBcolor getColor(const ColorKey& key);
    [[nodiscard]] bool getBool(const BoolKey& key);

    void setString(const StringKey &key, const std::string &value);
    void setInt(const IntKey &key, int value);
    void setDouble(const DoubleKey &key, double value);
    void setColor(const ColorKey &key, util::RGBcolor value);
    void setBool(const BoolKey &key, bool value);

    void exportToTxt();
    void importFromTxt();
    
    const StringKey LDRAW_PARTS_LIBRARY("ldrawPartsLibrary", "~/ldraw");
    const IntKey SCREEN_WIDTH("screenWidth", 1280);
    const IntKey SCREEN_HEIGHT("screenHeight", 720);
    const IntKey INSTANCED_MIN_COMPLEXITY("instancedMinComplexity", 6000);
    const IntKey MSAA_SAMPLES("msaaSamples", 16);
    const DoubleKey GUI_SCALE("guiScale", 1.5);
    const StringKey GUI_STYLE("guiStyle", "BrickSim");//or ImGuiLight, ImGuiClassic or ImGuiDark
    const ColorKey BACKGROUND_COLOR("backgroundColor", util::RGBcolor(0x36, 0x36, 0x36));
    const BoolKey SHOW_NORMALS("showNormals", false);
    const BoolKey DISPLAY_SELECTION_BUFFER("displaySelectionBuffer", false);
    const IntKey JPG_SCREENSHOT_QUALITY("jpgScreenshotQuality", 90);
    const ColorKey COLOR_MULTI_PART_DOCUMENT("colorMultiPartDocument", util::RGBcolor(0x0, 0xff, 0xff));
    const ColorKey COLOR_MPD_SUBFILE("colorMpdSubfile", util::RGBcolor(0x0, 0xff, 0x0));
    const ColorKey COLOR_MPD_SUBFILE_INSTANCE("colorMpdSubfileInstance", util::RGBcolor(0xff, 0xff, 0x0));
    const ColorKey COLOR_OFFICAL_PART("colorOfficalPart", util::RGBcolor(0xff, 0xff, 0xff));
    const ColorKey COLOR_UNOFFICAL_PART("colorUnofficalPart", util::RGBcolor(0xff, 0xff, 0xff));
    const IntKey THUMBNAIL_SIZE("thumbnailSize", 256);
    const IntKey THUMBNAIL_CACHE_SIZE_BYTES("thumbnailCacheSizeBytes", 1073741824);
    const BoolKey DRAW_MINIMAL_ENCLOSING_BALL_LINES("drawMinimalEnclosingBallLines", false);
    const StringKey BRICKLINK_CURRENCY_CODE("bricklinkCurrencyCode", "CHF");
    const BoolKey ENABLE_VIEWPORTS("enableViewports", false);
}

#endif //BRICKSIM_CONFIG_H
