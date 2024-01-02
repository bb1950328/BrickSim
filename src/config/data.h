#pragma once
#include "magic_enum.hpp"

#include <json_dto/pub.hpp>
#include <string>

#include "../helpers/color.h"

namespace bricksim::gui::windows {
    enum class Id;
}

namespace bricksim::user_actions {
    enum Action : uint32_t;
}

namespace bricksim::config {
    namespace {
        template<typename T>
        void init(T* this_, const char* const json) {
            *this_ = json_dto::from_json<T>(json);
        }

        template<typename T>
        void defaultInit(T* this_) {
            init<T>(this_, "{}");
        }
    }

    enum class GuiStyle {
        BrickSim,
        ImGuiLight,
        ImGuiClassic,
        ImGuiDark,
    };

    struct Gui {
        float scale;
        GuiStyle style = GuiStyle::BrickSim;
        bool enableImGuiViewports;
        std::string font;

        Gui() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("scale", scale, 1.5f)
                    & json_dto::optional("style", style, GuiStyle::BrickSim)
                    & json_dto::optional("enableImGuiViewports", enableImGuiViewports, false)
                    & json_dto::optional("font", font, "Roboto");
        }
    };

    struct LDraw {
        std::string libraryLocation;
        std::string shadowLibraryLocation;
        bool enableTexmapSupport;

        LDraw() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io & json_dto::optional("libraryLocation", libraryLocation, "~/ldraw")
                    & json_dto::optional("shadowLibraryLocation", shadowLibraryLocation, "~/LDCadShadowLibrary")
                    & json_dto::optional("enableTexmapSupport", enableTexmapSupport, true);
        }
    };

    struct GraphicsDebug {
        bool showNormals;
        bool displaySelectionBuffer;
        bool drawMinimalEnclosingBallLines;
        bool displayConnectorDataIn3DView;

        GraphicsDebug() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("showNormals", showNormals, false)
                    & json_dto::optional("displaySelectionBuffer", displaySelectionBuffer, false)
                    & json_dto::optional("drawMinimalEnclosingBallLines", drawMinimalEnclosingBallLines, false)
                    & json_dto::optional("displayConnectorDataIn3DView", displayConnectorDataIn3DView, false);
        }
    };

    struct Graphics {
        uint16_t msaaSamples;
        color::RGB background;
        int jpgScreenshotQuality;
        bool vsync;
        bool faceCulling;
        bool deleteVertexDataAfterUploading;
        GraphicsDebug debug;

        Graphics() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("msaaSamples", msaaSamples, 16)
                    & json_dto::optional("background", background, color::RGB(0x36, 0x36, 0x36))
                    & json_dto::optional("jpgScreenshotQuality", jpgScreenshotQuality, 90)
                    & json_dto::optional("vsync", vsync, true)
                    & json_dto::optional("faceCulling", faceCulling, true)
                    & json_dto::optional("deleteVertexDataAfterUploading", deleteVertexDataAfterUploading, true)
                    & json_dto::optional("debug", debug, {});
        }
    };

    struct NodeColors {
        color::RGB modelInstance;
        color::RGB part;
        color::RGB model;

        NodeColors() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("modelInstance", modelInstance, color::YELLOW)
                    & json_dto::optional("part", part, color::WHITE)
                    & json_dto::optional("model", model, color::WHITE);
        }
    };

    struct ElementTree {
        NodeColors nodeColors;

        ElementTree() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("nodeColors", nodeColors, {});
        }
    };

    struct PartPalette {
        uint16_t thumbnailSize;

        PartPalette() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("thumbnailSize", thumbnailSize, 256);
        }
    };

    struct BricklinkIntegration {
        std::string currencyCode;

        BricklinkIntegration() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("currencyCode", currencyCode, "CHF");
        }
    };

    struct Log {
        uint64_t notImportantLogMessageKeepCount;

        Log() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("notImportantLogMessageKeepCount", notImportantLogMessageKeepCount, 100);
        }
    };

    struct System {
        bool enableGlDebugOutput;
        bool enableThreading;
        std::string renderingTmpDirectory;
        bool clearRenderingTmpDirectoryOnExit;

        System() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("enableGlDebugOutput", enableGlDebugOutput, false)
                    & json_dto::optional("enableThreading", enableThreading, true)
                    & json_dto::optional("renderingTmpDirectory", renderingTmpDirectory, "{tmp}/BrickSimRenderingTmp")
                    & json_dto::optional("clearRenderingTmpDirectoryOnExit", clearRenderingTmpDirectoryOnExit, true);
        }
    };

    struct View3DSensitivity {
        float rotate;
        float pan;
        float zoom;

        View3DSensitivity() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("rotate", rotate, 1.f)
                    & json_dto::optional("pan", pan, 1.f)
                    & json_dto::optional("zoom", zoom, 1.f);
        }
    };

    struct View3D {
        View3DSensitivity sensitivity;

        View3D() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("sensitivity", sensitivity, false);
        }
    };

    enum class AngleMode {
        Euler,
        Quaternion,
    };

    struct ElementProperties {
        AngleMode angleMode;

        ElementProperties() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("angleMode", angleMode, AngleMode::Euler);
        }
    };

    struct Editor {
        std::string newFileLocation;

        Editor() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("newFileLocation", newFileLocation, "~");
        }
    };

    struct SnappingLinearStepPreset {
        std::string name;
        uint64_t stepXZ;
        uint64_t stepY;

        SnappingLinearStepPreset(const std::string& name, const uint64_t step_xz, const uint64_t step_y) :
            name(name),
            stepXZ(step_xz),
            stepY(step_y) {}

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::mandatory("name", name)
                    & json_dto::mandatory("stepXZ", stepXZ)
                    & json_dto::mandatory("stepY", stepY);
        }
    };

    struct SnappingRotationalStepPreset {
        std::string name;
        float step;

        SnappingRotationalStepPreset(const std::string& name, const float step) :
            name(name),
            step(step) {}

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::mandatory("name", name)
                    & json_dto::mandatory("step", step);
        }
    };


    struct Snapping {
        std::vector<SnappingLinearStepPreset> linearPresets;
        std::vector<SnappingRotationalStepPreset> rotationalPresets;

        constexpr static std::vector<SnappingLinearStepPreset> DEFAULT_LINEAR_PRESETS = {{
                {"Brick", 20, 24},
                {"Technic", 20, 20},
                {"Plate/Half Brick", 10, 8},
                {"Half Technic", 10, 10},
        }};
        constexpr static std::vector<SnappingRotationalStepPreset> DEFAULT_ROTATIONAL_PRESETS = {{
                {"1/4", 90},
                {"1/6", 60},
                {"1/8", 45},
                {"1/16", 22.5f},
        }};

        Snapping() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("linearPresets", linearPresets, DEFAULT_LINEAR_PRESETS)
                    & json_dto::optional("rotationalPresets", rotationalPresets, DEFAULT_ROTATIONAL_PRESETS);
        }
    };

    enum class KeyEvent : uint8_t {
        ON_RELEASE = 0 /*GLFW_RELEASE*/,
        ON_PRESS = 1 /*GLFW_PRESS*/,
        ON_REPEAT = 2 /*GLFW_REPEAT*/,
    };

    constexpr std::array<const char* const, magic_enum::enum_count<KeyEvent>()> KEY_EVENT_DISPLAY_NAMES = {"On Release", "On Press", "On Repeat"};

    class KeyboardShortcut {
    public:
        user_actions::Action action;
        uint64_t key;
        uint8_t modifiers;
        KeyEvent event;
        uoset_t<gui::windows::Id> windowScope;

        KeyboardShortcut() {
            init(this, R"JSON({"action": "DO_NOTHING", "key": 0})JSON");
        }

        KeyboardShortcut(const user_actions::Action action,
                         const uint64_t key,
                         const uint8_t modifiers,
                         const KeyEvent event,
                         const uoset_t<gui::windows::Id>& window_scope) :
            action(action),
            key(key),
            modifiers(modifiers),
            event(event),
            windowScope(window_scope) {}

        KeyboardShortcut(const KeyboardShortcut& other) = default;
        KeyboardShortcut(KeyboardShortcut&& other) = default;
        KeyboardShortcut& operator=(const KeyboardShortcut& other) = default;
        KeyboardShortcut& operator=(KeyboardShortcut&& other) = default;

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::mandatory("action", action)
                    & json_dto::mandatory("key", key)
                    & json_dto::optional("modifiers", modifiers, 0)
                    & json_dto::optional("event", event, KeyEvent::ON_PRESS)
                    & json_dto::optional("windowScope", windowScope, {});
        }
    };

    struct KeyboardShortcuts {
        uint64_t defaultCount;
        std::vector<KeyboardShortcut> shortcuts;

        KeyboardShortcuts() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("defaultCount", defaultCount, 0);
        }
    };


    struct Config {
        Gui gui;
        LDraw ldraw;
        Graphics graphics;
        ElementTree elementTree;
        PartPalette partPalette;
        BricklinkIntegration bricklinkIntegration;
        Log log;
        System system;
        View3D view3d;
        ElementProperties elementProperties;
        Editor editor;
        Snapping snapping;
        KeyboardShortcuts keyboardShortcuts;

        Config() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("gui", gui, {})
                    & json_dto::optional("ldraw", ldraw, {})
                    & json_dto::optional("graphics", graphics, {})
                    & json_dto::optional("elementTree", elementTree, {})
                    & json_dto::optional("partPalette", partPalette, {})
                    & json_dto::optional("bricklinkIntegration", bricklinkIntegration, {})
                    & json_dto::optional("log", log, {})
                    & json_dto::optional("system", system, {})
                    & json_dto::optional("view3d", view3d, {})
                    & json_dto::optional("elementProperties", elementProperties, {})
                    & json_dto::optional("editor", editor, {})
                    & json_dto::optional("snapping", snapping, {})
                    & json_dto::optional("keyboardShortcuts", keyboardShortcuts, {});
        }
    };

    struct SnappingState {
        bool enabled;
        uint64_t linearStepXZ;
        uint64_t linearStepY;
        float rotationalStep;

        SnappingState() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("enabled", enabled, true)
                    & json_dto::optional("linearStepXZ", linearStepXZ, 20)
                    & json_dto::optional("linearStepY", linearStepY, 20)
                    & json_dto::optional("rotationalStep", rotationalStep, 90.f);
        }
    };

    struct PersistedState {
        uint16_t windowWidth;
        uint16_t windowHeight;
        SnappingState snapping;

        PersistedState() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("windowWidth", windowWidth, 1280)
                    & json_dto::optional("windowHeight", windowHeight, 720)
                    & json_dto::optional("snapping", snapping, {});
        }
    };
}
