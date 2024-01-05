#include "../../config/read.h"
#include "../../config/write.h"
#include "../gui.h"
#include "imgui_stdlib.h"
#include "window_settings.h"

#include "TextEditor.h"
#include "imgui_internal.h"
#include "../../info_providers/bricklink_constants_provider.h"

#include "tinyfiledialogs.h"
#include "../../keyboard_shortcut_manager.h"
#include "../../lib/magic_enum/test/3rdparty/Catch2/include/catch2/catch.hpp"

namespace bricksim::gui::windows::settings {
    void drawPathInputWithSpecialPaths(const char* label, std::string& path, const bool isDirectory) {
        ImGui::PushID(label);
        const float promptButtonSize = ImGui::GetFrameHeight();
        const float inputWidth = ImGui::CalcItemWidth() - promptButtonSize - ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::PushItemWidth(inputWidth);
        ImGui::InputText("##input", &path);
        ImGui::PopItemWidth();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (ImGui::BeginTooltip()) {
                ImGui::Text("Special Paths:");
                if (ImGui::BeginTable("specialPaths", 2)) {
                    for (const auto& [name, path]: util::getSpecialPaths()) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", name.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", path.string().c_str());
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTooltip();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN, {promptButtonSize, promptButtonSize})) {
            char* selectedPath;
            const auto extendedPath = util::replaceSpecialPaths(path).string();
            if (isDirectory) {
                selectedPath = tinyfd_selectFolderDialog(label, extendedPath.c_str());
            } else {
                selectedPath = tinyfd_openFileDialog(label, extendedPath.c_str(), 0, nullptr, nullptr, 0);
            }
            if (selectedPath != nullptr) {
                path = selectedPath;
            }
        }

        ImGui::SameLine();
        ImGui::Text("%s", label);

        ImGui::PopID();
    }

    void drawColorPicker(const char* label, color::RGB& value) {
        auto background = value.asGlmVector();
        if (ImGui::ColorEdit3(label, &background[0], ImGuiColorEditFlags_Uint8)) {
            value = color::RGB(background);
        }
    }

    template<typename N, N base, typename = std::enable_if_t<std::is_integral_v<N>, bool>>
    void drawPowerSlider(const char* label, N& value, const N minExponent, const N maxExponent) {
        int currentExponent = std::log(value) / std::log(base);
        if (ImGui::SliderInt(label, &currentExponent, minExponent, maxExponent, std::to_string(value).c_str())) {
            value = std::pow(base, currentExponent);
        }
    }

    template<typename E>
    void drawEnumCombo(const char* label, E& value) {
        const auto preview = std::string(magic_enum::enum_name(value));
        if (ImGui::BeginCombo(label, preview.c_str())) {
            for (const auto& item: magic_enum::enum_values<E>()) {
                const auto itemName = std::string(magic_enum::enum_name(item));
                if (ImGui::Selectable(itemName.c_str(), value == item)) {
                    value = item;
                }
            }
            ImGui::EndCombo();
        }
    }

    void drawCurrencyCodeCombo(const char* label, std::string& value) {
        if (ImGui::BeginCombo(label, value.c_str())) {
            for (const auto& [index, code]: info_providers::bricklink_constants::getCurrencies()) {
                if (ImGui::Selectable(code.codeCurrency.c_str(), value == code.codeCurrency)) {
                    value = code.codeCurrency;
                }
            }
            ImGui::EndCombo();
        }
    }

    template<typename D>
    void drawSettings(D& data);

    template<>
    void drawSettings(config::Gui& data) {
        ImGui::InputFloat("Scale", &data.scale, .1f, .2f, "%.2f");
        drawEnumCombo("Style", data.style);
        ImGui::Checkbox("Enable ImGui Viewports", &data.enableImGuiViewports);
        drawEnumCombo("Font", data.font);
    }

    template<>
    void drawSettings(config::LDraw& data) {
        drawPathInputWithSpecialPaths("Library Location", data.libraryLocation, true);
        drawPathInputWithSpecialPaths("Shadow Library Location", data.shadowLibraryLocation, true);
        ImGui::Checkbox("Enable !TEXMAP Support", &data.enableTexmapSupport);
    }

    template<>
    void drawSettings(config::GraphicsDebug& data) {
        ImGui::Checkbox("Show Normals", &data.showNormals);
        ImGui::Checkbox("Display Selection Buffer", &data.displaySelectionBuffer);
        ImGui::Checkbox("Draw Minimal Enclosing Ball Lines", &data.drawMinimalEnclosingBallLines);
        ImGui::Checkbox("Display Connector Data In 3D View", &data.displayConnectorDataIn3DView);
    }

    template<>
    void drawSettings(config::Graphics& data) {
        drawPowerSlider<uint16_t, 2>("MSAA Samples", data.msaaSamples, 0, 4);
        drawColorPicker("Background Color", data.background);
        ImGui::SliderInt("JPG Screenshot Quality", &data.jpgScreenshotQuality, 1, 100, "%d%%");
        ImGui::Checkbox("VSync", &data.vsync);
        ImGui::Checkbox("Face Culling", &data.faceCulling);
        ImGui::Checkbox("Delete Vertex Data in RAM after Uploading to VRAM", &data.deleteVertexDataAfterUploading);
    }


    template<>
    void drawSettings(config::NodeColors& data) {
        drawColorPicker("Model Instance", data.modelInstance);
        drawColorPicker("Part", data.part);
        drawColorPicker("Model", data.model);
    }


    template<>
    void drawSettings(config::ElementTree& data) {}

    template<>
    void drawSettings(config::PartPalette& data) {
        int thumbnailSize = data.thumbnailSize;
        if (ImGui::InputInt("Thumbnail Image Size", &thumbnailSize, 16, 64)) {
            data.thumbnailSize = std::clamp(thumbnailSize, 4, 2048);
        }
        //todo implement editor for custom category trees
    }

    template<>
    void drawSettings(config::BricklinkIntegration& data) {
        drawCurrencyCodeCombo("Currency", data.currencyCode);
    }

    template<>
    void drawSettings(config::Log& data) {
        int notImportantLogMessageKeepCount = data.notImportantLogMessageKeepCount;
        if (ImGui::InputInt("Number of non-important log messages to keep", &notImportantLogMessageKeepCount, 10, 100)) {
            data.notImportantLogMessageKeepCount = std::max(notImportantLogMessageKeepCount, 0);
        }
    }

    template<>
    void drawSettings(config::System& data) {
        ImGui::Checkbox("OpenGL Debug Output", &data.enableGlDebugOutput);
        ImGui::Checkbox("Enable Multithreading", &data.enableThreading);
        drawPathInputWithSpecialPaths("Temporary Directory For Rendered Images", data.renderingTmpDirectory, true);
        ImGui::Checkbox("Clear Temporary Directory For Rendered Images On Exit", &data.clearRenderingTmpDirectoryOnExit);
    }

    template<>
    void drawSettings(config::View3DSensitivity& data) {
        ImGui::InputFloat("Rotate", &data.rotate, .1f, .2f, "%.2f");
        ImGui::InputFloat("Pan", &data.pan, .1f, .2f, "%.2f");
        ImGui::InputFloat("Zoom", &data.zoom, .1f, .2f, "%.2f");
    }

    template<>
    void drawSettings(config::View3D& data) {}

    template<>
    void drawSettings(config::ElementProperties& data) {
        drawEnumCombo("Angle Mode", data.angleMode);
    }

    template<>
    void drawSettings(config::Editor& data) {
        drawPathInputWithSpecialPaths("New File Location", data.newFileLocation, true);
    }

    template<>
    void drawSettings(config::Snapping& data) {
        //todo make presets configurable
    }

    template<>
    void drawSettings(config::KeyboardShortcuts& data) {
        static std::optional<std::reference_wrapper<config::KeyboardShortcut>> currentlyEditingShortcut;
        bool shouldOpenSelectActionModal = false;
        bool shouldOpenSelectKeyModal = false;
        static bool isWaitingOnKeyCatch = false;
        if (ImGui::BeginTable("##key_shortucts", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail())) {
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Event", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            auto shortcut = data.shortcuts.begin();
            while (shortcut != data.shortcuts.end()) {
                const auto* shortcutId = static_cast<void*>(&*shortcut);
                ImGui::PushID(shortcutId);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const char* actionName = user_actions::getName(shortcut->action).data();
                if (ImGui::Button(actionName)) {
                    currentlyEditingShortcut = *shortcut;
                    shouldOpenSelectActionModal = true;
                }
                ImGui::TableNextColumn();
                if (ImGui::Button(keyboard_shortcut_manager::getDisplayName(*shortcut).c_str())) {
                    currentlyEditingShortcut = *shortcut;
                    shouldOpenSelectKeyModal = true;
                }

                ImGui::TableNextColumn();
                const auto currentEventDisplayName = *(config::KEY_EVENT_DISPLAY_NAMES.cbegin() + *magic_enum::enum_index(shortcut->event));
                ImGui::PushItemWidth(75.f * config::get().gui.scale);

                if (ImGui::BeginCombo("##eventCombo", currentEventDisplayName)) {
                    auto it = config::KEY_EVENT_DISPLAY_NAMES.cbegin();
                    for (const auto& event: magic_enum::enum_values<config::KeyEvent>()) {
                        if (ImGui::Selectable(*it, shortcut->event == event)) {
                            shortcut->event = event;
                        }
                        ++it;
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();

                ImGui::TableNextColumn();
                ImGui::PushItemWidth(100.f * config::get().gui.scale);
                //todo modify this to allow multi-select
                const auto currentScopeDisplayName = shortcut->windowScope.empty()
                                                         ? "All Windows"
                                                         : gui::windows::getName(*shortcut->windowScope.begin());
                if (ImGui::BeginCombo("##scopeCombo", currentScopeDisplayName)) {
                    if (ImGui::Selectable("All Windows", shortcut->windowScope.empty())) {
                        shortcut->windowScope = {};
                    }
                    for (const auto& id: magic_enum::enum_values<gui::windows::Id>()) {
                        if (ImGui::Selectable(windows::getName(id), shortcut->windowScope.contains(id))) {
                            shortcut->windowScope = {id};
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopItemWidth();

                ImGui::TableNextColumn();
                std::string btnName = ICON_FA_TRASH_CAN;
                ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
                if (ImGui::Selectable(btnName.c_str())) {
                    data.shortcuts.erase(shortcut);
                } else {
                    ++shortcut;
                }
                ImGui::PopStyleColor();

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (ImGui::Button(ICON_FA_PLUS " Add new")) {
            data.shortcuts.emplace_back(user_actions::DO_NOTHING, 0, static_cast<keyboard_shortcut_manager::modifier_t>(0), config::KeyEvent::ON_PRESS);
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_ROTATE_LEFT " Reset all to default")) {
            keyboard_shortcut_manager::resetToDefault(data);
        }
        if (shouldOpenSelectActionModal) {
            ImGui::OpenPopup("Select action");
        }
        bool actionPopupOpen = true;
        if (ImGui::BeginPopupModal("Select action", &actionPopupOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            constexpr int searchBufSize = 32;
            static char searchBuf[searchBufSize];
            static user_actions::Action currentlySelectedAction = user_actions::DO_NOTHING;
            if (shouldOpenSelectActionModal) {
                currentlySelectedAction = currentlyEditingShortcut.value().get().action;
            }
            ImGui::InputTextWithHint(ICON_FA_MAGNIFYING_GLASS, "type to filter actions...", searchBuf, searchBufSize);
            if (ImGui::BeginListBox("##actionListBox")) {
                for (const auto& action: user_actions::findActionsByName(searchBuf)) {
                    const bool is_selected = (action == currentlySelectedAction);
                    if (ImGui::Selectable(user_actions::getName(action).data(), is_selected)) {
                        currentlyEditingShortcut.value().get().action = action;
                        currentlyEditingShortcut = {};
                        ImGui::CloseCurrentPopup();
                    }

                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndListBox();
            }
            ImGui::EndPopup();
        }
        if (shouldOpenSelectKeyModal) {
            keyboard_shortcut_manager::setCatchNextShortcut(true);
            ImGui::OpenPopup("Press keys");
        }
        bool selectKeyPopupOpen = true;
        if (ImGui::BeginPopupModal("Press keys", &selectKeyPopupOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Press the keys which you want to assign to this action.");
            const auto caught = keyboard_shortcut_manager::getCaughtShortcut();
            if (caught.has_value()) {
                ImGui::Text("Currently selected: %s", keyboard_shortcut_manager::getDisplayName(*caught).c_str());
            } else {
                ImGui::Text("Currently selected: %s", keyboard_shortcut_manager::getDisplayName(currentlyEditingShortcut.value().get()).c_str());
            }

            bool close = !selectKeyPopupOpen;
            if (ImGui::Button(ICON_FA_CHECK " OK")) {
                if (caught.has_value()) {
                    currentlyEditingShortcut.value().get().key = caught->key;
                    currentlyEditingShortcut.value().get().modifiers = caught->modifiers;
                }
                close = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_RECTANGLE_XMARK " Cancel")) {
                close = true;
            }
            if (close) {
                isWaitingOnKeyCatch = false;
                currentlyEditingShortcut = {};
                keyboard_shortcut_manager::setCatchNextShortcut(false);
                keyboard_shortcut_manager::clearCaughtShortcut();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    template<>
    void drawSettings(config::Config& data) {}

    template<typename D>
    const char* getTitle();

    template<>
    const char* getTitle<config::Gui>() {
        return "GUI";
    }

    template<>
    const char* getTitle<config::LDraw>() {
        return "LDraw";
    }

    template<>
    const char* getTitle<config::GraphicsDebug>() {
        return "Debug";
    }

    template<>
    const char* getTitle<config::Graphics>() {
        return "Graphics";
    }

    template<>
    const char* getTitle<config::NodeColors>() {
        return "Node Colors";
    }

    template<>
    const char* getTitle<config::ElementTree>() {
        return "Element Tree";
    }

    template<>
    const char* getTitle<config::PartPalette>() {
        return "Part Palette";
    }

    template<>
    const char* getTitle<config::BricklinkIntegration>() {
        return "Bricklink Integration";
    }

    template<>
    const char* getTitle<config::Log>() {
        return "Log";
    }

    template<>
    const char* getTitle<config::System>() {
        return "System";
    }

    template<>
    const char* getTitle<config::View3DSensitivity>() {
        return "Sensitivity";
    }

    template<>
    const char* getTitle<config::View3D>() {
        return "3D View";
    }

    template<>
    const char* getTitle<config::ElementProperties>() {
        return "Element Properties";
    }

    template<>
    const char* getTitle<config::Editor>() {
        return "Editor";
    }

    template<>
    const char* getTitle<config::Snapping>() {
        return "Snapping";
    }

    template<>
    const char* getTitle<config::KeyboardShortcuts>() {
        return "Keyboard Shortcuts";
    }

    template<>
    const char* getTitle<config::Config>() {
        return "Settings";
    }

    struct JsonEditorData {
        TextEditor editor;
        std::string originalJson;
        std::string errorMessage;
    };

    template<typename D>
    void drawTabs(D& data) {
        if (ImGui::BeginTabBar("##settingsTabs")) {
            if (ImGui::BeginTabItem("Settings")) {
                drawSettings(data);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("JSON")) {
                static uomap_t<std::size_t, JsonEditorData> allEditors;
                auto it = allEditors.find(typeid(D).hash_code());
                if (it == allEditors.end()) {
                    JsonEditorData editorData;
                    //editorData.originalJson = json_helper::to_pretty_json(data);
                    it = allEditors.emplace(typeid(D).hash_code(), editorData).first;
                }
                auto& editorData = it->second;
                auto& editor = editorData.editor;

                const auto currentDataJson = json_helper::to_pretty_json(data);
                if (currentDataJson != editorData.originalJson) {
                    //changed outside of editor
                    editorData.originalJson = currentDataJson;
                    editor.SetText(currentDataJson);
                }

                if (!editorData.errorMessage.empty()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color::RED));
                    ImGui::TextWrapped(ICON_FA_TRIANGLE_EXCLAMATION" %s", editorData.errorMessage.c_str());
                    ImGui::PopStyleColor();
                }
                editor.Render("##json");
                if (editor.IsTextChanged()) {
                    const auto currentText = editor.GetText();
                    try {
                        data = json_dto::from_json<D>(currentText);
                        editorData.errorMessage.clear();
                        editor.SetErrorMarkers({});
                        editorData.originalJson = json_helper::to_pretty_json(data);
                    } catch (json_dto::ex_t ex) {
                        editorData.errorMessage = ex.what();
                        const size_t offsetIdx = editorData.errorMessage.find("offset: ");
                        if (offsetIdx != std::string::npos) {
                            const auto offsetStr = editorData.errorMessage.substr(offsetIdx + 8, editorData.errorMessage.find_first_not_of("0123456789", offsetIdx));
                            const auto offset = std::stoul(offsetStr);
                            const auto offsetLineNo = std::count(currentText.begin(), currentText.begin() + offset, '\n');
                            TextEditor::ErrorMarkers errorMarkers;
                            errorMarkers.insert(std::make_pair(offsetLineNo + 1, editorData.errorMessage));
                            editor.SetErrorMarkers(errorMarkers);
                        }
                    }
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    struct CategoryTreeState {
        int currentIndex;
        int selectedIndex;
        std::function<void()> tabsFunction;
    };

    template<typename D>
    constexpr bool hasChildren() {
        return false;
    }

    template<typename D>
    void drawCategoryTreeChildren([[maybe_unused]] CategoryTreeState& state, [[maybe_unused]] D& data) {
        //specialize this template for all D that have children
    }

    template<typename D>
    void drawCategoryTree(CategoryTreeState& state, D& data) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
        const uint64_t ownIndex = state.currentIndex++;
        if (ownIndex == state.selectedIndex) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if constexpr (!hasChildren<D>()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (ImGui::TreeNodeEx(getTitle<D>(), flags)) {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                state.selectedIndex = ownIndex;
            }
            if constexpr (hasChildren<D>()) {
                drawCategoryTreeChildren(state, data);
            }
            ImGui::TreePop();
        }
        if (ownIndex == state.selectedIndex) {
            state.tabsFunction = [&data] {
                drawTabs(data);
            };
        }
    }


    template<>
    void drawCategoryTreeChildren(CategoryTreeState& state, config::Graphics& data) {
        drawCategoryTree(state, data.debug);
    }

    template<>
    constexpr bool hasChildren<config::Graphics>() {
        return true;
    }

    template<>
    void drawCategoryTreeChildren(CategoryTreeState& state, config::ElementTree& data) {
        drawCategoryTree(state, data.nodeColors);
    }

    template<>
    constexpr bool hasChildren<config::ElementTree>() {
        return true;
    }

    template<>
    void drawCategoryTreeChildren(CategoryTreeState& state, config::View3D& data) {
        drawCategoryTree(state, data.sensitivity);
    }

    template<>
    constexpr bool hasChildren<config::View3D>() {
        return true;
    }

    template<>
    void drawCategoryTreeChildren(CategoryTreeState& state, config::Config& data) {
        drawCategoryTree(state, data.gui);
        drawCategoryTree(state, data.ldraw);
        drawCategoryTree(state, data.graphics);
        drawCategoryTree(state, data.elementTree);
        drawCategoryTree(state, data.partPalette);
        drawCategoryTree(state, data.bricklinkIntegration);
        drawCategoryTree(state, data.log);
        drawCategoryTree(state, data.system);
        drawCategoryTree(state, data.view3d);
        drawCategoryTree(state, data.elementProperties);
        drawCategoryTree(state, data.editor);
        drawCategoryTree(state, data.snapping);
        drawCategoryTree(state, data.keyboardShortcuts);
    }

    template<>
    constexpr bool hasChildren<config::Config>() {
        return true;
    }

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            static config::Config editingConfig = config::get();
            const float treeWidth = config::get().gui.scale * 150.f;
            const float tabsWidth = ImGui::GetContentRegionAvail().x - treeWidth - ImGui::GetStyle().FramePadding.x * 2;
            const float buttonHeight = (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2);
            const float mainHeight = ImGui::GetContentRegionAvail().y - buttonHeight - ImGui::GetStyle().FramePadding.y * 4 - 1.f;
            ImGui::BeginChild("##categoryTree", {treeWidth, mainHeight});
            static CategoryTreeState treeState{0, 1, [] {}};
            treeState.currentIndex = 0;
            drawCategoryTree(treeState, editingConfig);
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("##tabs", {tabsWidth, mainHeight});
            treeState.tabsFunction();
            ImGui::EndChild();

            ImGui::Separator();
            const bool unchanged = editingConfig == config::get();
            if (unchanged) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save")) {
                config::getMutable() = editingConfig;
                config::save();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_CLOCK_ROTATE_LEFT " Discard Changes")) {
                editingConfig = config::get();
            }
            if (unchanged) {
                ImGui::EndDisabled();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_RETWEET " Restore Defaults")) {
                config::getMutable() = {};
                config::save();
                editingConfig = config::get();
            }
        }
        ImGui::End();
    }
}
