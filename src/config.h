

#ifndef BRICKSIM_CONFIG_H
#define BRICKSIM_CONFIG_H

#include <map>
#include "helpers/util.h"
#include "helpers/color.h"

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
        const color::RGB defaultValue;
        ColorKey(const std::string &name, const color::RGB &defaultValue);
    };
    class BoolKey : public Key {
    public:
        const bool defaultValue;
        BoolKey(const std::string &name, bool defaultValue);
    };
    
    [[nodiscard]] std::string getString(const StringKey& key);
    [[nodiscard]] int getInt(const IntKey &key);
    [[nodiscard]] double getDouble(const DoubleKey &key);
    [[nodiscard]] color::RGB getColor(const ColorKey& key);
    [[nodiscard]] bool getBool(const BoolKey& key);

    void setString(const StringKey &key, const std::string &value);
    void setInt(const IntKey &key, int value);
    void setDouble(const DoubleKey &key, double value);
    void setColor(const ColorKey &key, color::RGB value);
    void setBool(const BoolKey &key, bool value);

    void exportToTxt();
    void importFromTxt();

    void resetAllToDefault();
    
    const StringKey LDRAW_PARTS_LIBRARY("ldrawPartsLibrary", "~/ldraw");
    const IntKey SCREEN_WIDTH("screenWidth", 1280);
    const IntKey SCREEN_HEIGHT("screenHeight", 720);
    const IntKey INSTANCED_MIN_COMPLEXITY("instancedMinComplexity", 6000);
    const IntKey MSAA_SAMPLES("msaaSamples", 16);
    const DoubleKey GUI_SCALE("guiScale", 1.5);
    const StringKey GUI_STYLE("guiStyle", "BrickSim");//or ImGuiLight, ImGuiClassic or ImGuiDark
    const ColorKey BACKGROUND_COLOR("backgroundColor", color::RGB(0x36, 0x36, 0x36));
    const BoolKey SHOW_NORMALS("showNormals", false);
    const BoolKey DISPLAY_SELECTION_BUFFER("displaySelectionBuffer", false);
    const IntKey JPG_SCREENSHOT_QUALITY("jpgScreenshotQuality", 90);
    const ColorKey COLOR_MULTI_PART_DOCUMENT("colorMultiPartDocument", color::RGB(0x0, 0xff, 0xff));
    const ColorKey COLOR_MPD_SUBFILE("colorMpdSubfile", color::RGB(0x0, 0xff, 0x0));
    const ColorKey COLOR_MPD_SUBFILE_INSTANCE("colorMpdSubfileInstance", color::RGB(0xff, 0xff, 0x0));
    const ColorKey COLOR_OFFICAL_PART("colorOfficalPart", color::RGB(0xff, 0xff, 0xff));
    const ColorKey COLOR_UNOFFICAL_PART("colorUnofficalPart", color::RGB(0xff, 0xff, 0xff));
    const IntKey THUMBNAIL_SIZE("thumbnailSize", 256);
    const IntKey THUMBNAIL_CACHE_SIZE_BYTES("thumbnailCacheSizeBytes", 1073741824);
    const BoolKey DRAW_MINIMAL_ENCLOSING_BALL_LINES("drawMinimalEnclosingBallLines", false);
    const StringKey BRICKLINK_CURRENCY_CODE("bricklinkCurrencyCode", "CHF");
    const BoolKey ENABLE_VIEWPORTS("enableViewports", false);
    const StringKey FONT("font", "Roboto");
    const IntKey NOT_IMPORTANT_LOG_MESSAGE_KEEP_COUNT("notImportantLogMessageKeepCount", 10);
    const BoolKey ENABLE_GL_DEBUG_OUTPUT("enableGlDebugOutput", false);
    const DoubleKey MOUSE_3DVIEW_ROTATE_SENSITIVITY("mouse3dViewRotateSensitivity", 1);
    const DoubleKey MOUSE_3DVIEW_PAN_SENSITIVITY("mouse3dViewPanSensitivity", 1);
    const DoubleKey MOUSE_3DVIEW_ZOOM_SENSITIVITY("mouse3dViewZoomSensitivity", 1);
    const BoolKey ENABLE_VSYNC("enableVsync", true);
    const DoubleKey TRANSFORM_GIZMO_SIZE("transformGizmoSize", 1.0);
}

#endif //BRICKSIM_CONFIG_H
