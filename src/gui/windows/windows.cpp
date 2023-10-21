#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include <array>

#include <chrono>
#include <imgui.h>
#include <magic_enum.hpp>
#include <vector>

#include "../../metrics.h"
#include "utilities/window_gear_ratio_calculator.h"
#include "window_about.h"
#include "window_connection_visualization.h"
#include "window_debug.h"
#include "window_editor_meta_info.h"
#include "window_element_properties.h"
#include "window_element_tree.h"
#include "window_ldraw_file_inspector.h"
#include "window_log.h"
#include "window_mesh_inspector.h"
#include "window_model_info.h"
#include "window_orientation_cube.h"
#include "window_part_palette.h"
#include "window_settings.h"
#include "window_system_info.h"
#include "window_toolbar.h"
#include "window_view3d.h"
#include "windows.h"

namespace bricksim::gui::windows {
    namespace {
        void drawImGuiDemo(Data& data) {
            ImGui::ShowDemoWindow(&data.visible);
        }
        void noCleanup() {
        }
    }
    std::array<Data, magic_enum::enum_count<Id>()> data{
            {
                    {Id::VIEW_3D, ICON_FA_CUBES " 3D View", true, view3d::draw, noCleanup},
                    {Id::ELEMENT_TREE, ICON_FA_LIST " Element Tree", true, element_tree::draw, noCleanup},
                    {Id::ELEMENT_PROPERTIES, ICON_FA_WRENCH " Element Properties", true, element_properties::draw, noCleanup},
                    {Id::PART_PALETTE, ICON_FA_TABLE_CELLS " Part Palette", false /*todo true*/, part_palette::draw, noCleanup},
                    {Id::SETTINGS, ICON_FA_SLIDERS " Settings", false, settings::draw, noCleanup},
                    {Id::ABOUT, ICON_FA_CIRCLE_INFO " About", false, about::draw, noCleanup},
                    {Id::SYSTEM_INFO, ICON_FA_MICROCHIP " System Info", false, system_info::draw, noCleanup},
                    {Id::DEBUG, ICON_FA_BUG " Debug", true, debug::draw, noCleanup},
                    {Id::MESH_INSPECTOR, ICON_FA_SHAPES " Mesh Inspector", false, mesh_inspector::draw, noCleanup},
                    {Id::IMGUI_DEMO, ICON_FA_IMAGE " ImGui Demo", false, drawImGuiDemo, noCleanup},
                    {Id::ORIENTATION_CUBE, ICON_FA_CUBE " Orientation Cube", true, orientation_cube::draw, noCleanup},
                    {Id::LOG, ICON_FA_LIST " Log", false, log::draw, noCleanup},
                    {Id::GEAR_RATIO_CALCULATOR, ICON_FA_GEARS " Gear Ratio Calculator", false, tools::gear_ratio_calculator::draw, noCleanup},
                    {Id::MODEL_INFO, ICON_FA_INFO " Model Info", false, model_info::draw, noCleanup},
                    {Id::EDITOR_META_INFO, ICON_FA_RECEIPT " Meta-Info", false, editor_meta_info::draw, noCleanup},
                    {Id::LDRAW_FILE_INSPECTOR, ICON_FA_EYE " LDraw File Inspector", false, ldraw_file_inspector::draw, noCleanup},
                    {Id::TOOLBAR, ICON_FA_SCREWDRIVER_WRENCH " Toolbar", true, toolbar::draw, noCleanup},
                    {Id::CONNECTION_VISUALIZATION, ICON_FA_SHARE_NODES " Connection visualization", false, connection_visualization::draw, connection_visualization::cleanup},
            }};

    void drawAll() {
        std::vector<std::pair<std::string, float>> drawingTimesMicroseconds;
        for (auto& windowData: data) {
            if (windowData.visible) {
                auto before = std::chrono::high_resolution_clock::now();
                windowData.drawFunction(windowData);
                auto after = std::chrono::high_resolution_clock::now();
                drawingTimesMicroseconds.emplace_back(windowData.name, std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count() / 1000.0);
            }
        }
        metrics::lastWindowDrawingTimesUs = drawingTimesMicroseconds;
    }

    void cleanup() {
        for (auto& windowData: data) {
            windowData.cleanupFunction();
        }
    }

    bool* isVisible(Id id) {
        return &data[magic_enum::enum_integer(id)].visible;
    }

    const std::array<Data, magic_enum::enum_count<Id>()>& getData() {
        return data;
    }
    const char* getName(Id id) {
        return data[magic_enum::enum_integer(id)].name;
    }
}
