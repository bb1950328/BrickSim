#include "main_menu_bar.h"
#include "../controller.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "gui.h"
#include "gui_internal.h"
namespace bricksim::gui {
    namespace {
        void drawDocumentMenu(const std::shared_ptr<Editor>& editor) {
            gui_internal::actionMenuItem(user_actions::SAVE_FILE, editor);
            gui_internal::actionMenuItem(user_actions::SAVE_FILE_AS, editor);
            gui_internal::actionMenuItem(user_actions::SAVE_COPY_AS, editor);
            gui_internal::actionMenuItem(user_actions::TAKE_SCREENSHOT, editor);
        }
    }
    void drawMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                gui_internal::actionMenuItem(user_actions::NEW_FILE);
                gui_internal::actionMenuItem(user_actions::OPEN_FILE);
                gui_internal::actionMenuItem(user_actions::SAVE_FILE);
                gui_internal::actionMenuItem(user_actions::SAVE_FILE_AS);
                gui_internal::actionMenuItem(user_actions::SAVE_COPY_AS);

                ImGui::Separator();

                gui_internal::actionMenuItem(user_actions::EXIT);

                ImGui::MenuItem(ICON_FA_CIRCLE_INFO " About", "", windows::isVisible(windows::Id::ABOUT));
                ImGui::MenuItem(ICON_FA_MICROCHIP " System Info", "", windows::isVisible(windows::Id::SYSTEM_INFO));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                gui_internal::actionMenuItem(user_actions::UNDO);
                gui_internal::actionMenuItem(user_actions::REDO);
                ImGui::Separator();
                gui_internal::actionMenuItem(user_actions::CUT);
                gui_internal::actionMenuItem(user_actions::COPY);
                gui_internal::actionMenuItem(user_actions::PASTE);
                ImGui::Separator();
                gui_internal::actionMenuItem(user_actions::INLINE_SELECTED_ELEMENTS);
                ImGui::EndMenu();
            }
            auto& editors = controller::getEditors();
            if (editors.size() == 1) {
                if (ImGui::BeginMenu("Document")) {
                    drawDocumentMenu(*editors.begin());
                    ImGui::EndMenu();
                }
            } else {
                if (ImGui::BeginMenu("Documents")) {
                    for (const auto& editor: editors) {
                        if (editor->isActive()) {
                            ImGui::PushStyleColor(ImGuiCol_Text, COLOR_ACTIVE_EDITOR);
                        }
                        const auto opened = ImGui::BeginMenu(editor->getFilename().c_str());
                        ImGui::PopStyleColor(editor->isActive() ? 1 : 0);
                        if (opened) {
                            drawDocumentMenu(editor);
                            ImGui::EndMenu();
                        }
                    }
                    ImGui::EndMenu();
                }
            }

            if (ImGui::BeginMenu("View")) {
                gui_internal::windowMenuItem(windows::Id::VIEW_3D);
                gui_internal::windowMenuItem(windows::Id::ORIENTATION_CUBE);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::ELEMENT_TREE);
                gui_internal::windowMenuItem(windows::Id::ELEMENT_PROPERTIES);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::PART_PALETTE);
                gui_internal::windowMenuItem(windows::Id::LDRAW_FILE_INSPECTOR);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::MODEL_INFO);
                gui_internal::windowMenuItem(windows::Id::EDITOR_META_INFO);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::SETTINGS);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::IMGUI_DEMO);
                gui_internal::windowMenuItem(windows::Id::DEBUG);
                gui_internal::windowMenuItem(windows::Id::LOG);
                ImGui::Separator();
                gui_internal::actionMenuItem(user_actions::APPLY_DEFAULT_WINDOW_LAYOUT);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Snap")) {
                auto& handler = controller::getSnapHandler();
                ImGui::MenuItem("Enabled", nullptr, handler.isEnabledPtr());
                if (ImGui::BeginMenu(ICON_FA_RULER_COMBINED " Linear snap steps")) {
                    auto& linearHandler = handler.getLinear();
                    const auto& presets = linearHandler.getPresets();
                    static_assert(snap::LinearHandler::TEMPORARY_PRESET_INDEX == -1);
                    for (int i = -1; i < static_cast<int>(presets.size()); ++i) {
                        const auto& preset = i >= 0 ? presets[i] : linearHandler.getTemporaryPreset();
                        const std::string& name = i >= 0 ? preset.name : "Custom";
                        const auto txt = fmt::format("{} XZ={} Y={}", name, preset.stepXZ, preset.stepY);
                        if (ImGui::MenuItem(txt.c_str(), nullptr, linearHandler.getCurrentPresetIndex() == i)) {
                            linearHandler.setCurrentPresetIndex(i);
                        }
                    }
                    if (ImGui::BeginMenu(ICON_FA_PENCIL " Edit Custom")) {
                        static int xz;
                        static int y;
                        auto& tmpPreset = linearHandler.getTemporaryPreset();
                        xz = tmpPreset.stepXZ;
                        y = tmpPreset.stepY;
                        if (ImGui::InputInt("XZ", &xz, 1, 20) | ImGui::InputInt("Y", &y, 1, 20)) {
                            linearHandler.setTemporaryPreset({"",
                                                              std::clamp(xz, 1, 20000),
                                                              std::clamp(y, 1, 20000)});
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(ICON_FA_ARROWS_SPIN " Rotational snap steps")) {
                    auto& rotationalHandler = handler.getRotational();
                    const auto& presets = rotationalHandler.getPresets();
                    static_assert(snap::RotationalHandler::TEMPORARY_PRESET_INDEX == -1);
                    for (int i = -1; i < static_cast<int>(presets.size()); ++i) {
                        const std::string& name = i >= 0
                                                          ? presets[i].name
                                                          : "Custom";
                        const float stepDeg = i >= 0
                                                      ? presets[i].stepDeg
                                                      : rotationalHandler.getTemporaryPreset().stepDeg;

                        const auto txt = fmt::format("{}: {:g}°", name, stepDeg);
                        if (ImGui::MenuItem(txt.c_str(), nullptr, rotationalHandler.getCurrentPresetIndex() == i)) {
                            rotationalHandler.setCurrentPresetIndex(i);
                        }
                    }
                    if (ImGui::BeginMenu(ICON_FA_PENCIL " Edit Custom")) {
                        static float step;
                        auto& tmpPreset = rotationalHandler.getTemporaryPreset();
                        step = tmpPreset.stepDeg;
                        if (ImGui::InputFloat("##step", &step, 1.f, 10.f, "%.2f°")) {
                            rotationalHandler.setTemporaryPreset({"", std::clamp(step, 0.f, 360.f)});
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Selection")) {
                gui_internal::actionMenuItem(user_actions::SELECT_ALL);
                gui_internal::actionMenuItem(user_actions::SELECT_NOTHING);
                gui_internal::actionMenuItem(user_actions::SELECT_CONNECTED);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("3D")) {
                gui_internal::actionMenuItem(user_actions::VIEW_3D_FRONT, "Front");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_TOP, "Top");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_RIGHT, "Right");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_REAR, "Rear");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_BOTTOM, "Bottom");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_LEFT, "Left");
                ImGui::Separator();
                const auto activeEditor = controller::getActiveEditor();
                if (activeEditor == nullptr) {
                    ImGui::BeginDisabled();
                    ImGui::MenuItem("Show Surfaces", "");
                    ImGui::MenuItem("Show Lines", "");
                    ImGui::EndDisabled();
                } else {
                    const auto mainScene = activeEditor->getScene();
                    ImGui::MenuItem("Show Surfaces", "", mainScene->isDrawTriangles());
                    ImGui::MenuItem("Show Lines", "", mainScene->isDrawLines());
                }
                gui_internal::actionMenuItem(user_actions::TAKE_SCREENSHOT);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                ImGui::MenuItem(windows::getName(windows::Id::GEAR_RATIO_CALCULATOR), "", windows::isVisible(windows::Id::GEAR_RATIO_CALCULATOR));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                gui_internal::actionMenuItem(user_actions::EXECUTE_ACTION_BY_NAME);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

}
