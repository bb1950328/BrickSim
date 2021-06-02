#include <array>
#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"

#include <imgui.h>
#include <vector>
#include <chrono>
#include <magic_enum.hpp>

#include "tools/window_gear_ratio_calculator.h"
#include "window_about.h"
#include "window_debug.h"
#include "window_element_properties.h"
#include "window_element_tree.h"
#include "window_log.h"
#include "window_mesh_inspector.h"
#include "window_orientation_cube.h"
#include "window_part_palette.h"
#include "window_settings.h"
#include "window_system_info.h"
#include "window_view3d.h"
#include "windows.h"
#include "../../metrics.h"

namespace gui::windows {
    namespace {
        void drawImGuiDemo(Data& data) {
            ImGui::ShowDemoWindow(&data.visible);
        }
    }
    std::array<Data, magic_enum::enum_count<Id>()> data{
            {
                    {Id::VIEW_3D, ICON_FA_CUBES" 3D View", true, view3d::draw},
                    {Id::ELEMENT_TREE, ICON_FA_LIST" Element Tree", true, element_tree::draw},
                    {Id::ELEMENT_PROPERTIES, ICON_FA_WRENCH" Element Properties", true, element_properties::draw},
                    {Id::PART_PALETTE, ICON_FA_TH" Part Palette", true, part_palette::draw},
                    {Id::SETTINGS, ICON_FA_SLIDERS_H" Settings", false, settings::draw},
                    {Id::ABOUT, ICON_FA_INFO_CIRCLE " About", false, about::draw},
                    {Id::SYSTEM_INFO, ICON_FA_MICROCHIP " System Info", false, system_info::draw},
                    {Id::DEBUG, ICON_FA_BUG" Debug", true, debug::draw},
                    {Id::MESH_INSPECTOR, ICON_FA_SHAPES" Mesh Inspector", false, mesh_inspector::draw},
                    {Id::IMGUI_DEMO, ICON_FA_IMAGE" ImGui Demo", false, drawImGuiDemo},
                    {Id::ORIENTATION_CUBE, ICON_FA_CUBE" Orientation Cube", true, orientation_cube::draw},
                    {Id::LOG, ICON_FA_LIST" Log", false, log::draw},
                    {Id::GEAR_RATIO_CALCULATOR, "Gear Ratio Calculator", false, tools::gear_ratio_calculator::draw},
            }};

    void drawAll() {
        std::vector<std::pair<std::string, float>> drawingTimesMicroseconds;
        for (auto &windowData : data) {
            if (windowData.visible) {
                auto before = std::chrono::high_resolution_clock::now();
                windowData.drawFunction(windowData);
                auto after = std::chrono::high_resolution_clock::now();
                drawingTimesMicroseconds.emplace_back(windowData.name, std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count() / 1000.0);
            }
        }
        metrics::lastWindowDrawingTimesUs = drawingTimesMicroseconds;
    }

    bool *isVisible(Id id) {
        return &data[magic_enum::enum_integer(id)].visible;
    }

    const std::array<Data, magic_enum::enum_count<Id>()> &getData() {
        return data;
    }

    const char *getName(Id id) {
        return data[magic_enum::enum_integer(id)].name;
    }
}