#pragma once

#include "db.h"
#include "helpers/color.h"
#include "helpers/util.h"
#include "snapping/snap_const.h"
#include <cstring>
#include <functional>
#include <map>
#include <mutex>
#include <utility>

namespace bricksim::config {
    namespace {
        template<typename T>
        class ValuesCache {
            //using const char* as key is ok here because there's only one instance of each key
            uomap_t<const char*, T> values;

            std::function<void(const char*, T)> writeFunction;
            std::function<std::optional<T>(const char*)> readFunction;

            std::mutex mutex;

        public:
            T get(const char* key, T defaultValue);

            void set(const char* key, T value);

            ValuesCache(std::function<void(const char*, T)> writeFunction,
                        std::function<std::optional<T>(const char*)> readFunction);

            void clear();
        };

        ValuesCache<std::string> stringValues(db::config::setString, db::config::getString);
        ValuesCache<int> intValues(db::config::setInt, db::config::getInt);
        ValuesCache<double> doubleValues(db::config::setDouble, db::config::getDouble);

        template<typename T>
        T ValuesCache<T>::get(const char* key, T defaultValue) {
            const auto it = values.find(key);
            if (it == values.end()) {
                std::scoped_lock<std::mutex> lg(mutex);
                std::optional<T> opt = readFunction(key);
                if (opt.has_value()) {
                    return values[key] = opt.value();
                } else {
                    writeFunction(key, defaultValue);
                    return values[key] = defaultValue;
                }
            }
            return it->second;
        }

        template<typename T>
        void ValuesCache<T>::set(const char* key, T value) {
            std::scoped_lock<std::mutex> lg(mutex);
            writeFunction(key, value);
            values[key] = value;
        }

        template<typename T>
        ValuesCache<T>::ValuesCache(
                const std::function<void(const char*, T)> writeFunction,
                const std::function<std::optional<T>(const char*)> readFunction) :
            writeFunction(std::move(writeFunction)),
            readFunction(std::move(readFunction)),
            mutex() {}

        template<typename T>
        void ValuesCache<T>::clear() {
            values.clear();
        }
    }

    template<typename T>
        requires(
                std::is_same_v<T, std::string>
                || std::is_convertible_v<T, int> || std::is_convertible_v<T, double> || std::is_convertible_v<T, color::RGB>)
    class Key {
    public:
        const char* const name;
        const T defaultValue;

        Key(const char* const name, const T defaultValue) :
            name(name), defaultValue(defaultValue) {}

        bool operator==(const Key& rhs) const {
            return std::strcmp(name, rhs.name) == 0;
        }
    };

    template<typename T>
    [[nodiscard]] T get(const Key<T>& key) = delete;

    template<>
    [[nodiscard]] std::string get<std::string>(const Key<std::string>& key);

    template<>
    [[nodiscard]] int get<int>(const Key<int>& key);

    template<>
    [[nodiscard]] double get<double>(const Key<double>& key);

    template<>
    [[nodiscard]] color::RGB get<color::RGB>(const Key<color::RGB>& key);

    template<>
    [[nodiscard]] float get<float>(const Key<float>& key);

    template<>
    [[nodiscard]] bool get<bool>(const Key<bool>& key);

    template<typename T, typename V>
        requires std::is_convertible_v<V, T>
    void set(const Key<T>& key, const V& value);

    template<typename V>
    void set(const Key<std::string>& key, const V& value) {
        stringValues.set(key.name, value);
    }

    template<typename V>
    void set(const Key<int>& key, const V& value) {
        intValues.set(key.name, value);
    }

    template<typename V>
    void set(const Key<double>& key, const V& value) {
        doubleValues.set(key.name, value);
    }

    template<typename V>
    void set(const Key<float>& key, const V& value) {
        doubleValues.set(key.name, value);
    }

    template<typename V>
    void set(const Key<bool>& key, const V& value) {
        intValues.set(key.name, value);
    }

    template<>
    void set(const Key<color::RGB>& key, const color::RGB& value);

    void resetAllToDefault();

    /*ldraw.libraryLocation*/const Key<std::string> LDRAW_PARTS_LIBRARY("ldrawPartsLibrary", "~/ldraw");
    /*ldraw.shadowLibraryLocation*/const Key<std::string> SHADOW_LIBRARY_PATH("shadowLibraryPath", "~/LDCadShadowLibrary");
    /*PS.windowWidth*/const Key<int> SCREEN_WIDTH("screenWidth", 1280);
    /*PS.windowHeight*/const Key<int> SCREEN_HEIGHT("screenHeight", 720);
    /*<deleted>*/const Key<int> INSTANCED_MIN_COMPLEXITY("instancedMinComplexity", 6000);
    /*graphics.msaaSamples*/const Key<int> MSAA_SAMPLES("msaaSamples", 16);
    /*gui.scale*/const Key<float> GUI_SCALE("guiScale", 1.5);
    /*gui.style*/const Key<std::string> GUI_STYLE("guiStyle", "BrickSim");//or ImGuiLight, ImGuiClassic or ImGuiDark
    /*graphics.background*/const Key<color::RGB> BACKGROUND_COLOR("backgroundColor", color::RGB(0x36, 0x36, 0x36));
    /*graphics.showNormals*/const Key<bool> SHOW_NORMALS("showNormals", false);
    /*graphics.displaySelectionBuffer*/const Key<bool> DISPLAY_SELECTION_BUFFER("displaySelectionBuffer", false);
    /*graphics.jpgScreenshotQuality*/const Key<int> JPG_SCREENSHOT_QUALITY("jpgScreenshotQuality", 90);
    /*<deleted>*/const Key<color::RGB> COLOR_MULTI_PART_DOCUMENT("colorMultiPartDocument", color::RGB(0x0, 0xff, 0xff));
    /*<deleted>*/const Key<color::RGB> COLOR_MPD_SUBFILE("colorMpdSubfile", color::RGB(0x0, 0xff, 0x0));
    /*elementTree.nodeColors.modelInstance*/const Key<color::RGB> COLOR_MPD_SUBFILE_INSTANCE("colorMpdSubfileInstance", color::RGB(0xff, 0xff, 0x0));
    /*elementTree.nodeColors.part*/const Key<color::RGB> COLOR_OFFICAL_PART("colorOfficalPart", color::RGB(0xff, 0xff, 0xff));
    /*<deleted>*/const Key<color::RGB> COLOR_UNOFFICAL_PART("colorUnofficalPart", color::RGB(0xff, 0xff, 0xff));
    /*partPalette.thumbnailSize*/const Key<int> THUMBNAIL_SIZE("thumbnailSize", 256);
    /*<inlined>*/const Key<int> THUMBNAIL_CACHE_SIZE_BYTES("thumbnailCacheSizeBytes", 1073741824);
    /*graphics.drawMinimalEnclosingBallLines*/const Key<bool> DRAW_MINIMAL_ENCLOSING_BALL_LINES("drawMinimalEnclosingBallLines", false);
    /*bricklinkIntegration.currencyCode*/const Key<std::string> BRICKLINK_CURRENCY_CODE("bricklinkCurrencyCode", "CHF");
    /*gui.enableImGuiViewports*/const Key<bool> ENABLE_VIEWPORTS("enableViewports", false);
    /*gui.font*/const Key<std::string> FONT("font", "Roboto");
    /*log.notImportantLogMessageKeepCount*/const Key<int> NOT_IMPORTANT_LOG_MESSAGE_KEEP_COUNT("notImportantLogMessageKeepCount", 10);
    /*system.enableGlDebugOutput*/const Key<bool> ENABLE_GL_DEBUG_OUTPUT("enableGlDebugOutput", false);
    /*view3d.sensitivity.rotate*/const Key<float> MOUSE_3DVIEW_ROTATE_SENSITIVITY("mouse3dViewRotateSensitivity", 1);
    /*view3d.sensitivity.pan*/const Key<float> MOUSE_3DVIEW_PAN_SENSITIVITY("mouse3dViewPanSensitivity", 1);
    /*view3d.sensitivity.zoom*/const Key<float> MOUSE_3DVIEW_ZOOM_SENSITIVITY("mouse3dViewZoomSensitivity", 1);
    /*graphics.vsync*/const Key<bool> ENABLE_VSYNC("enableVsync", true);
    /*<deleted>*/const Key<float> TRANSFORM_GIZMO_SIZE("transformGizmoSize", 1.0);
    /*graphics.faceCulling*/const Key<bool> FACE_CULLING_ENABLED("faceCullingEnabled", true);
    /*system.enableThreading*/const Key<bool> THREADING_ENABLED("threadingEnabled", true);
    /*elementProperties.angleMode*/const Key<bool> USE_EULER_ANGLES("useEulerAngles", true);
    /*graphics.deleteVertexDataAfterUploading*/const Key<bool> DELETE_VERTEX_DATA_AFTER_UPLOADING("deleteVertexDataAfterUploading", true);
    /*ldraw.enableTexmapSupport*/const Key<bool> ENABLE_TEXMAP_SUPPORT("enableTexmapSupport", true);
    /*graphics.displayConnectorDataIn3DView*/const Key<bool> DISPLAY_CONNECTOR_DATA_IN_3D_VIEW("displayConnectorDataIn3DView", false);
    /*editor.newFileLocation*/const Key<std::string> NEW_FILE_LOCATION("newFileLocation", "~");
    /*PS.snapping.linearStepXZ*/const Key<int> LINEAR_SNAP_STEP_XZ("linearSnapStepXZ", 20);
    /*PS.snapping.linearStepY*/const Key<int> LINEAR_SNAP_STEP_Y("linearSnapStepY", 20);
    /*snapping.linearPresets*/const Key<std::string> LINEAR_SNAP_PRESETS("linearSnapPresets", snap::DEFAULT_LINEAR_SNAP_DISTANCE_PRESETS);
    /*PS.snapping.enabled*/const Key<bool> SNAP_ENABLED("snapEnabled", true);
    /*snapping.rotationalPresets*/const Key<std::string> ROTATIONAL_SNAP_PRESETS("rotationalSnapPresets", snap::DEFAULT_ROTATIONAL_SNAP_PRESETS);
    /*PS.snapping.rotationalStep*/const Key<float> ROTATIONAL_SNAP_STEP("rotationalSnapStep", 90.f);
    /*keyboardShortcuts.defaultCount*/const Key<int> DEFAULT_KEYBOARD_SHORTCUT_COUNT("defaultKeyboardShortcutCount", 0);
    /*system.renderingTmpDirectory*/const Key<std::string> RENDERING_TMP_DIRECTORY("renderingTmpDirectory", "{tmp}/BrickSimRenderingTmp");
    /*system.clearRenderingTmpDirectoryOnExit*/const Key<bool> CLEAR_RENDERING_TMP_DIRECTORY_ON_EXIT("clearRenderingTmpDirectoryOnExit", true);
}
