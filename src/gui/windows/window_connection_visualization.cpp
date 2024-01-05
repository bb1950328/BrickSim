#include "window_connection_visualization.h"
#include "../../connection/visualization/connection_graphviz_generator.h"
#include "../../controller.h"
#include "../../helpers/graphviz_wrapper.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../dialogs.h"
#include "../gui.h"
#include "../gui_internal.h"
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "stb_image.h"
#include <fstream>

namespace bricksim::gui::windows::connection_visualization {
    namespace {
        struct Viewport {
            glm::vec2 uvCenter = {.5f, .5f};
            float zoom = 1;
        };

        void drawImageCanvas(const graphics::Texture& texture, Viewport& viewport) {
            const glm::vec2 availableSpace = ImGui::GetContentRegionAvail();
            const glm::vec2 windowPos = ImGui::GetWindowPos();
            const glm::vec2 regionMin = ImGui::GetWindowContentRegionMin();
            const glm::vec2 mousePos = ImGui::GetMousePos();
            const glm::vec2 regionMax = ImGui::GetWindowContentRegionMax();
            glm::vec2 relCursorPos = glm::clamp(mousePos - windowPos - regionMin, {0, 0}, availableSpace);

            const glm::vec2 imgSize = texture.getSize();

            const auto minZoom = std::min(availableSpace.x / imgSize.x, availableSpace.y / imgSize.y);
            const auto maxZoom = std::max(imgSize.x / availableSpace.x, imgSize.y / availableSpace.y) * 10.f;
            viewport.zoom = std::clamp(viewport.zoom, minZoom, maxZoom);

            const auto centerPx = viewport.uvCenter * imgSize;
            const auto topLeftPx = centerPx - availableSpace / 2.f / viewport.zoom;
            const auto bottomRightPx = centerPx + availableSpace / 2.f / viewport.zoom;

            const auto effTopLeftPx = glm::clamp(topLeftPx, {0, 0}, imgSize);
            const auto effBottomRightPx = glm::clamp(bottomRightPx, {0, 0}, imgSize);

            ImGui::SetCursorPos(regionMin + (effTopLeftPx - topLeftPx) * viewport.zoom);

            const auto yFlip = [](const glm::vec2& uv) {
                return glm::vec2(uv.x, 1 - uv.y);
            };

            ImGui::Image(gui_internal::convertTextureId(texture.getID()),
                         (effBottomRightPx - effTopLeftPx) * viewport.zoom,
                         yFlip(effTopLeftPx / imgSize),
                         yFlip(effBottomRightPx / imgSize));

            const float lastScrollDeltaY = getLastScrollDeltaY();
            if (std::abs(lastScrollDeltaY) > .01f && ImGui::IsWindowHovered()) {
                const float factor = std::pow(1.1f, lastScrollDeltaY);
                float oldZoom = viewport.zoom;
                viewport.zoom = std::clamp(oldZoom * factor, minZoom, maxZoom);
                float realFactor = viewport.zoom / oldZoom;

                const auto uv0 = topLeftPx / imgSize;
                const auto uv1 = bottomRightPx / imgSize;
                glm::vec2 cursorUv = uv1 + (uv0 - uv1) * (relCursorPos / availableSpace);
                if (std::abs(realFactor - 1.f) > .01f) {
                    glm::vec2 newUv0 = cursorUv - (cursorUv - uv0) * realFactor;
                    glm::vec2 newUv1 = cursorUv + (uv1 - cursorUv) * realFactor;
                    const auto uvMargin = glm::vec2(.5f, .5f) / (1 + viewport.zoom - minZoom);
                    viewport.uvCenter = glm::clamp((newUv0 + newUv1) / 2.f, uvMargin, glm::vec2(1, 1) - uvMargin);
                    //viewport.uvCenter = (newUv0 + newUv1) / 2.f;
                }
            }
        }

        std::shared_ptr<graphics::Texture> texture;
        bool textureUpToDate = true;
        connection::visualization::GraphVizResult graphvizCode;
        std::filesystem::path outputFile;

        void updateImage(std::shared_ptr<Editor>& editor, std::shared_ptr<etree::ModelNode>& model, const connection::visualization::GraphVizParams& params) {
            controller::getForegroundTasks().emplace("Visualize Connections with GraphViz", [editor, params, model](auto* progress) {
                *progress = .0f;
                auto& engine = editor->getConnectionEngine();
                engine.update(model);
                const auto& connectionGraph = engine.getConnections();
                *progress = .15f;
                graphvizCode = connection::visualization::generateGraphviz(connectionGraph, params, model);
                *progress = .3f;
                outputFile = params.thumbnailDirectory / "connectionVisualization.png";
                const auto renderSuccess = graphvizCode.renderToFile(outputFile);
                graphvizCode.deleteTmpFiles = false;
                *progress = .99f;
                if (renderSuccess) {
                    util::setStbiFlipVertically(true);
                    int width;
                    int height;
                    int nrChannels;
                    unsigned char* data = stbi_load(outputFile.string().c_str(), &width, &height, &nrChannels, 0);
                    if (data != nullptr) {
                        texture = std::make_shared<graphics::Texture>(data, width, height, nrChannels);
                    } else {
                        dialogs::showError(fmt::format("Cannot read image generated by GraphViz.\nReason: {}\nSave the image somewhere and open it with an external image viewer", stbi_failure_reason()));
                    }
                    textureUpToDate = data != nullptr;
                    stbi_image_free(data);
                } else {
                    dialogs::showError("Call to GraphViz was not successful. Check stdout to see details.");
                }
                *progress = 1.f;
            });
        }
    }

    static const char* const POPUP_ID = "connection_visualization_options";

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible, ImGuiWindowFlags_None)) {
            if (graphviz_wrapper::isAvailable()) {
                static std::weak_ptr<Editor> editor;
                std::shared_ptr<Editor> editorLocked = editor.lock();
                if (editorLocked == nullptr) {
                    editorLocked = controller::getEditors().front();
                }

                static std::weak_ptr<etree::ModelNode> model;
                std::shared_ptr<etree::ModelNode> modelLocked = model.lock();
                if (modelLocked != nullptr && !modelLocked->isChildOf(editorLocked->getRootNode())) {
                    modelLocked = nullptr;
                }
                if (modelLocked == nullptr) {
                    modelLocked = editorLocked->getEditingModel();
                }

                static connection::visualization::GraphVizParams params;
                params.thumbnailDirectory = util::replaceSpecialPaths(config::get().system.renderingTmpDirectory);
                std::filesystem::create_directories(params.thumbnailDirectory);

                static Viewport viewport;

                if (texture != nullptr) {
                    drawImageCanvas(*texture, viewport);
                }

                ImVec2 buttonSize(ImGui::GetFontSize() * 2, ImGui::GetFontSize() * 2);

                ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
                if (ImGui::Button(ICON_FA_SLIDERS, buttonSize)) {
                    ImGui::OpenPopup(POPUP_ID);
                }
                ImGui::SetItemTooltip("Options");

                if (texture != nullptr) {
                    if (ImGui::Button(ICON_FA_FILE_IMAGE, buttonSize)) {
                        const auto path = dialogs::showSaveImageDialog("Save Connection Visualization as Image");
                        if (path) {
                            if (textureUpToDate) {
                                texture->saveToFile(*path);
                            } else {
                                std::filesystem::copy_file(outputFile, *path);
                            }
                        }
                    }
                    ImGui::SetItemTooltip("Save rendered image as");

                    if (ImGui::Button(ICON_FA_CAMERA, buttonSize)) {
                        const auto path = dialogs::showSaveImageDialog("Render Connection Visualization with GraphViz");
                        if (path) {
                            controller::getForegroundTasks().emplace("Rerender Connection Graph with GraphViz", [&path](auto* progress) {
                                graphviz_wrapper::renderDot(*path, graphvizCode.dotCode);
                            });
                        }
                    }
                    ImGui::SetItemTooltip("Rerender image and save as");

                    if (ImGui::Button(ICON_FA_FILE_CODE, buttonSize)) {
                        const auto path = dialogs::showSaveDotFileDialog("Save Connection Visualization .dot Code");
                        if (path) {
                            std::ofstream stream(*path);
                            stream << graphvizCode.dotCode;
                        }
                    }
                    ImGui::SetItemTooltip("Save .dot code as");
                }

                if (ImGui::BeginPopup(POPUP_ID)) {
                    if (ImGui::Button(ICON_FA_ROTATE " Update")) {
                        updateImage(editorLocked, modelLocked, params);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::CollapsingHeader("General Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::BeginCombo("Editor", editorLocked->getDisplayName().c_str())) {
                            for (const auto& e: controller::getEditors()) {
                                if (ImGui::Selectable(e->getDisplayName().c_str(), e == editorLocked)) {
                                    editor = e;
                                    editorLocked = e;
                                }
                                if (e == editorLocked) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        if (ImGui::BeginCombo("Model", modelLocked->displayName.c_str())) {
                            for (const auto& m: editorLocked->getRootNode()->getChildren()) {
                                auto currentModel = std::dynamic_pointer_cast<etree::ModelNode>(m);
                                if (currentModel != nullptr && ImGui::Selectable(currentModel->displayName.c_str(), m == modelLocked)) {
                                    model = currentModel;
                                    modelLocked = currentModel;
                                }
                                if (currentModel == modelLocked) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        gui_internal::drawEnumCombo("Layout", params.general.layout);
                        ImGui::InputInt("DPI", &params.general.dpi, 24);
                        ImGui::Checkbox("Dark Theme", &params.general.darkTheme);
                    }
                    if (ImGui::CollapsingHeader("Node Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Checkbox("Show Thumbnail", &params.node.showThumbnail);
                        ImGui::Checkbox("Show Location", &params.node.showLocation);
                        ImGui::Checkbox("Show Name", &params.node.showName);
                        ImGui::Checkbox("Show Title", &params.node.showTitle);
                        ImGui::Checkbox("Color Node like part", &params.node.colorBoxLikePart);
                    }
                    if (ImGui::CollapsingHeader("Edge Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Checkbox("Only one edge between two nodes", &params.edge.oneLineBetweenNode);
                        ImGui::Checkbox("Draw non-rigid connections dashed", &params.edge.nonRigidConnectionsDashed);
                        ImGui::Checkbox("Color edges based on connector type", &params.edge.colorLineByConnectorType);
                    }
                    ImGui::EndPopup();
                }
            } else {
                ImGui::Text("GraphViz not available");
            }
        }
        ImGui::End();
    }

    void cleanup() {
        texture = nullptr;
        graphvizCode = {};
    }
}
