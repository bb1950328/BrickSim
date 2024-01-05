#pragma once
#include "magic_enum.hpp"

#include <json_dto/pub.hpp>
#include <string>

#include "../helpers/color.h"
#include "../helpers/json_helper.h"
#include "../gui/windows/windows.h"
#include "../user_actions.h"
#include "json_dto/validators.hpp"

namespace bricksim::config {
    using namespace json_helper;

    enum class GuiStyle {
        BrickSim,
        ImGuiLight,
        ImGuiClassic,
        ImGuiDark,
    };

    enum class GuiFont {
        Roboto,
        RobotoMono,
    };

    struct Gui {
        float scale;
        GuiStyle style;
        bool enableImGuiViewports;
        GuiFont font;

        Gui() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("scale", scale, 1.5f, json_dto::min_max_constraint(.0001f, 10000.f))
                    & json_dto::optional(json_helper::EnumRW<GuiStyle>(), "style", style, GuiStyle::BrickSim)
                    & json_dto::optional("enableImGuiViewports", enableImGuiViewports, false)
                    & json_dto::optional(json_helper::EnumRW<GuiFont>(), "font", font, GuiFont::Roboto);
        }

        friend bool operator==(const Gui& lhs, const Gui& rhs) {
            return lhs.scale == rhs.scale
                   && lhs.style == rhs.style
                   && lhs.enableImGuiViewports == rhs.enableImGuiViewports
                   && lhs.font == rhs.font;
        }

        friend bool operator!=(const Gui& lhs, const Gui& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const LDraw& lhs, const LDraw& rhs) {
            return lhs.libraryLocation == rhs.libraryLocation
                   && lhs.shadowLibraryLocation == rhs.shadowLibraryLocation
                   && lhs.enableTexmapSupport == rhs.enableTexmapSupport;
        }

        friend bool operator!=(const LDraw& lhs, const LDraw& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const GraphicsDebug& lhs, const GraphicsDebug& rhs) {
            return lhs.showNormals == rhs.showNormals
                   && lhs.displaySelectionBuffer == rhs.displaySelectionBuffer
                   && lhs.drawMinimalEnclosingBallLines == rhs.drawMinimalEnclosingBallLines
                   && lhs.displayConnectorDataIn3DView == rhs.displayConnectorDataIn3DView;
        }

        friend bool operator!=(const GraphicsDebug& lhs, const GraphicsDebug& rhs) { return !(lhs == rhs); }
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
                    & json_dto::optional("msaaSamples", msaaSamples, static_cast<uint16_t>(16), json_helper::power_of_two_validator_t<uint16_t, 1, 16>())
                    & json_dto::optional("background", background, color::RGB(0x36, 0x36, 0x36))
                    & json_dto::optional("jpgScreenshotQuality", jpgScreenshotQuality, 90, json_dto::min_max_constraint(1, 100))
                    & json_dto::optional("vsync", vsync, true)
                    & json_dto::optional("faceCulling", faceCulling, true)
                    & json_dto::optional("deleteVertexDataAfterUploading", deleteVertexDataAfterUploading, true)
                    & json_dto::optional("debug", debug, GraphicsDebug{});
        }

        friend bool operator==(const Graphics& lhs, const Graphics& rhs) {
            return lhs.msaaSamples == rhs.msaaSamples
                   && lhs.background == rhs.background
                   && lhs.jpgScreenshotQuality == rhs.jpgScreenshotQuality
                   && lhs.vsync == rhs.vsync
                   && lhs.faceCulling == rhs.faceCulling
                   && lhs.deleteVertexDataAfterUploading == rhs.deleteVertexDataAfterUploading
                   && lhs.debug == rhs.debug;
        }

        friend bool operator!=(const Graphics& lhs, const Graphics& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const NodeColors& lhs, const NodeColors& rhs) {
            return lhs.modelInstance == rhs.modelInstance
                   && lhs.part == rhs.part
                   && lhs.model == rhs.model;
        }

        friend bool operator!=(const NodeColors& lhs, const NodeColors& rhs) { return !(lhs == rhs); }
    };

    struct ElementTree {
        NodeColors nodeColors;

        ElementTree() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("nodeColors", nodeColors, NodeColors{});
        }

        friend bool operator==(const ElementTree& lhs, const ElementTree& rhs) { return lhs.nodeColors == rhs.nodeColors; }
        friend bool operator!=(const ElementTree& lhs, const ElementTree& rhs) { return !(lhs == rhs); }
    };

    struct PartCategoryTreeNode {
        uint64_t id;
        std::string name;
        std::string ldrawCategory;
        std::string nameFilter;
        std::vector<PartCategoryTreeNode> children;

        explicit PartCategoryTreeNode() {
            json_helper::defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("id", id, 0)
                    & json_dto::optional("name", name, "")
                    & json_dto::optional("ldrawCategory", ldrawCategory, "")
                    & json_dto::optional("nameFilter", nameFilter, "")
                    & json_dto::optional("children", children, decltype(children)());
        }

        friend bool operator==(const PartCategoryTreeNode& lhs, const PartCategoryTreeNode& rhs) {
            return lhs.id == rhs.id
                   && lhs.name == rhs.name
                   && lhs.ldrawCategory == rhs.ldrawCategory
                   && lhs.nameFilter == rhs.nameFilter;
        }

        friend bool operator!=(const PartCategoryTreeNode& lhs, const PartCategoryTreeNode& rhs) { return !(lhs == rhs); }
    };

    struct PartPalette {
        uint16_t thumbnailSize;
        std::vector<PartCategoryTreeNode> customTrees;

        PartPalette() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("thumbnailSize", thumbnailSize, 256, json_dto::min_max_constraint(4, 2048))
                    & json_dto::optional("customTrees", customTrees, decltype(customTrees)());
        }

        friend bool operator==(const PartPalette& lhs, const PartPalette& rhs) {
            return lhs.thumbnailSize == rhs.thumbnailSize
                   && lhs.customTrees == rhs.customTrees;
        }

        friend bool operator!=(const PartPalette& lhs, const PartPalette& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const BricklinkIntegration& lhs, const BricklinkIntegration& rhs) { return lhs.currencyCode == rhs.currencyCode; }
        friend bool operator!=(const BricklinkIntegration& lhs, const BricklinkIntegration& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const Log& lhs, const Log& rhs) { return lhs.notImportantLogMessageKeepCount == rhs.notImportantLogMessageKeepCount; }
        friend bool operator!=(const Log& lhs, const Log& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const System& lhs, const System& rhs) {
            return lhs.enableGlDebugOutput == rhs.enableGlDebugOutput
                   && lhs.enableThreading == rhs.enableThreading
                   && lhs.renderingTmpDirectory == rhs.renderingTmpDirectory
                   && lhs.clearRenderingTmpDirectoryOnExit == rhs.clearRenderingTmpDirectoryOnExit;
        }

        friend bool operator!=(const System& lhs, const System& rhs) { return !(lhs == rhs); }
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
                    & json_dto::optional("rotate", rotate, 1.f, json_dto::min_max_constraint(.01f, 100.f))
                    & json_dto::optional("pan", pan, 1.f, json_dto::min_max_constraint(.01f, 100.f))
                    & json_dto::optional("zoom", zoom, 1.f, json_dto::min_max_constraint(.01f, 100.f));
        }

        friend bool operator==(const View3DSensitivity& lhs, const View3DSensitivity& rhs) {
            return lhs.rotate == rhs.rotate
                   && lhs.pan == rhs.pan
                   && lhs.zoom == rhs.zoom;
        }

        friend bool operator!=(const View3DSensitivity& lhs, const View3DSensitivity& rhs) { return !(lhs == rhs); }
    };

    struct View3D {
        View3DSensitivity sensitivity;

        View3D() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("sensitivity", sensitivity, View3DSensitivity{});
        }

        friend bool operator==(const View3D& lhs, const View3D& rhs) { return lhs.sensitivity == rhs.sensitivity; }
        friend bool operator!=(const View3D& lhs, const View3D& rhs) { return !(lhs == rhs); }
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
                    & json_dto::optional(json_helper::EnumRW<AngleMode>(), "angleMode", angleMode, AngleMode::Euler);
        }

        friend bool operator==(const ElementProperties& lhs, const ElementProperties& rhs) { return lhs.angleMode == rhs.angleMode; }
        friend bool operator!=(const ElementProperties& lhs, const ElementProperties& rhs) { return !(lhs == rhs); }
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

        friend bool operator==(const Editor& lhs, const Editor& rhs) { return lhs.newFileLocation == rhs.newFileLocation; }
        friend bool operator!=(const Editor& lhs, const Editor& rhs) { return !(lhs == rhs); }
    };

    struct SnappingLinearStepPreset {
        std::string name;
        uint64_t stepXZ;
        uint64_t stepY;

        SnappingLinearStepPreset() = default;

        SnappingLinearStepPreset(const std::string& name, const uint64_t step_xz, const uint64_t step_y) :
            name(name),
            stepXZ(step_xz),
            stepY(step_y) {}

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::mandatory("name", name)
                    & json_dto::mandatory("stepXZ", stepXZ, json_dto::min_max_constraint<uint64_t>(1, 10000))
                    & json_dto::mandatory("stepY", stepY, json_dto::min_max_constraint<uint64_t>(1, 10000));
        }

        friend bool operator==(const SnappingLinearStepPreset& lhs, const SnappingLinearStepPreset& rhs) {
            return lhs.name == rhs.name
                   && lhs.stepXZ == rhs.stepXZ
                   && lhs.stepY == rhs.stepY;
        }

        friend bool operator!=(const SnappingLinearStepPreset& lhs, const SnappingLinearStepPreset& rhs) { return !(lhs == rhs); }
    };

    struct SnappingRotationalStepPreset {
        std::string name;
        /// in degrees
        float step;

        SnappingRotationalStepPreset() = default;

        SnappingRotationalStepPreset(const std::string& name, const float step) :
            name(name),
            step(step) {}

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::mandatory("name", name)
                    & json_dto::mandatory("step", step, json_dto::min_max_constraint(.01f, 360.f));
        }

        friend bool operator==(const SnappingRotationalStepPreset& lhs, const SnappingRotationalStepPreset& rhs) {
            return lhs.name == rhs.name
                   && lhs.step == rhs.step;
        }

        friend bool operator!=(const SnappingRotationalStepPreset& lhs, const SnappingRotationalStepPreset& rhs) { return !(lhs == rhs); }
    };


    struct Snapping {
        std::vector<SnappingLinearStepPreset> linearPresets;
        std::vector<SnappingRotationalStepPreset> rotationalPresets;

        const static std::vector<SnappingLinearStepPreset> DEFAULT_LINEAR_PRESETS;
        const static std::vector<SnappingRotationalStepPreset> DEFAULT_ROTATIONAL_PRESETS;

        Snapping() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("linearPresets", linearPresets, DEFAULT_LINEAR_PRESETS)
                    & json_dto::optional("rotationalPresets", rotationalPresets, DEFAULT_ROTATIONAL_PRESETS);
        }

        friend bool operator==(const Snapping& lhs, const Snapping& rhs) {
            return lhs.linearPresets == rhs.linearPresets
                   && lhs.rotationalPresets == rhs.rotationalPresets;
        }

        friend bool operator!=(const Snapping& lhs, const Snapping& rhs) { return !(lhs == rhs); }
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
                         const uoset_t<gui::windows::Id>& window_scope = {}) :
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
                    & json_dto::mandatory(json_helper::EnumRW<user_actions::Action>(), "action", action)
                    & json_dto::mandatory("key", key)
                    & json_dto::optional("modifiers", modifiers, 0)
                    & json_dto::optional(json_helper::EnumRW<KeyEvent>(), "event", event, KeyEvent::ON_PRESS)
                    & json_dto::optional(json_helper::EnumSetRW<gui::windows::Id>(), "windowScope", windowScope, uoset_t<gui::windows::Id>{});
        }

        friend bool operator==(const KeyboardShortcut& lhs, const KeyboardShortcut& rhs) {
            return lhs.action == rhs.action
                   && lhs.key == rhs.key
                   && lhs.modifiers == rhs.modifiers
                   && lhs.event == rhs.event
                   && lhs.windowScope == rhs.windowScope;
        }

        friend bool operator!=(const KeyboardShortcut& lhs, const KeyboardShortcut& rhs) { return !(lhs == rhs); }
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
                    & json_dto::optional("defaultCount", defaultCount, 0)
                    & json_dto::optional("shortcuts", shortcuts, std::vector<KeyboardShortcut>());
        }

        friend bool operator==(const KeyboardShortcuts& lhs, const KeyboardShortcuts& rhs) {
            return lhs.defaultCount == rhs.defaultCount
                   && lhs.shortcuts == rhs.shortcuts;
        }

        friend bool operator!=(const KeyboardShortcuts& lhs, const KeyboardShortcuts& rhs) { return !(lhs == rhs); }
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
                    & json_dto::optional("gui", gui, Gui{})
                    & json_dto::optional("ldraw", ldraw, LDraw{})
                    & json_dto::optional("graphics", graphics, Graphics{})
                    & json_dto::optional("elementTree", elementTree, ElementTree{})
                    & json_dto::optional("partPalette", partPalette, PartPalette{})
                    & json_dto::optional("bricklinkIntegration", bricklinkIntegration, BricklinkIntegration{})
                    & json_dto::optional("log", log, Log{})
                    & json_dto::optional("system", system, System{})
                    & json_dto::optional("view3d", view3d, View3D{})
                    & json_dto::optional("elementProperties", elementProperties, ElementProperties{})
                    & json_dto::optional("editor", editor, Editor{})
                    & json_dto::optional("snapping", snapping, Snapping{})
                    & json_dto::optional("keyboardShortcuts", keyboardShortcuts, KeyboardShortcuts{});
        }

        friend bool operator==(const Config& lhs, const Config& rhs) {
            return lhs.gui == rhs.gui
                   && lhs.ldraw == rhs.ldraw
                   && lhs.graphics == rhs.graphics
                   && lhs.elementTree == rhs.elementTree
                   && lhs.partPalette == rhs.partPalette
                   && lhs.bricklinkIntegration == rhs.bricklinkIntegration
                   && lhs.log == rhs.log
                   && lhs.system == rhs.system
                   && lhs.view3d == rhs.view3d
                   && lhs.elementProperties == rhs.elementProperties
                   && lhs.editor == rhs.editor
                   && lhs.snapping == rhs.snapping
                   && lhs.keyboardShortcuts == rhs.keyboardShortcuts;
        }

        friend bool operator!=(const Config& lhs, const Config& rhs) {
            return !(lhs == rhs);
        }
    };
}
