#include "window_connection_visualization.h"
#include "../../connection/visualization/connection_graphviz_generator.h"
#include "../../controller.h"
#include "../../helpers/graphviz_wrapper.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../gui.h"
#include "../gui_internal.h"
#include "imgui.h"
#include "spdlog/spdlog.h"

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

            ImGui::Image(gui_internal::convertTextureId(texture.getID()),
                         (effBottomRightPx - effTopLeftPx) * viewport.zoom,
                         effTopLeftPx / imgSize,
                         effBottomRightPx / imgSize);

            const float lastScrollDeltaY = getLastScrollDeltaY();
            if (std::abs(lastScrollDeltaY) > .01f) {
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

            /*const auto imgAspectRatio = imgSize.x / imgSize.x;
            const auto windowAspectRatio = availableSpace.x/availableSpace.y;

            const auto imgUvPerPixel = 1.f/ imgSize;

            const auto naturalZoom = std::min(availableSpace.x, availableSpace.y)/std::max(imgSize.x, imgSize.y);

            */

            /*const auto textureSize = glm::vec2(texture.getSize());
            const float maxZoom = std::max(8.f, util::biggestValue(textureSize / availableSpace) * 4.f);



            glm::vec2 u1half = ((availableSpace / std::min(availableSpace.x, availableSpace.y)) / (textureSize / std::min(textureSize.x, textureSize.y))) / viewport.zoom / 2.f;

            glm::vec2 uv0 = viewport.uvCenter - u1half;
            glm::vec2 uv1 = viewport.uvCenter + u1half;

            glm::vec2 cursorUv = uv1 + (uv0 - uv1) * (relCursorPos / availableSpace);
            //spdlog::debug("{} {}", cursorUv.x, cursorUv.y);

            const float lastScrollDeltaY = getLastScrollDeltaY();
            if (std::abs(lastScrollDeltaY) > .01f) {
                const float factor = std::pow(1.1f, lastScrollDeltaY);
                float oldZoom = viewport.zoom;
                viewport.zoom = std::clamp(oldZoom * factor, 1.f, maxZoom);
                float realFactor = viewport.zoom / oldZoom;

                if (std::abs(realFactor - 1.f) > .01f) {
                    glm::vec2 newUv0 = cursorUv - (cursorUv - uv0) * realFactor;
                    glm::vec2 newUv1 = cursorUv + (uv1 - cursorUv) * realFactor;
                    const auto uvMargin = glm::vec2(.5f, .5f) / viewport.zoom;
                    viewport.uvCenter = glm::clamp((newUv0 + newUv1) / 2.f, uvMargin, glm::vec2(1, 1) - uvMargin);
                }
            }

            const auto effectiveUv0 = glm::clamp(uv0, {0, 0}, {1, 1});
            const auto effectiveUv1 = glm::clamp(uv1, {0, 0}, {1, 1});

            const auto uvDiff = uv1 - uv0;
            const auto effectiveUvDiff = effectiveUv1 - effectiveUv0;
            const auto effectiveImgSize = availableSpace / uvDiff * effectiveUvDiff;
            //const auto uvPerPixel = uvDiff / availableSpace;
            //const auto effectiveImgSize = glm::clamp(1.f / uvPerPixel, {0, 0}, availableSpace);

            ImGui::SetCursorPos(glm::vec2(ImGui::GetWindowContentRegionMin()) */
            /* + (availableSpace - effectiveImgSize) / 2.f*/ /*);

            ImGui::Image(gui_internal::convertTextureId(texture.getID()),
                         */
            /*effectiveImgSize*/                             /* availableSpace,
                         */
            /*effectiveUv0*/                                 /* uv0,
                         */
            /*effectiveUv1*/                                 /* uv1);*/
        }

        std::shared_ptr<graphics::Texture> texture;

        void updateImage(std::shared_ptr<Editor>& editor, std::shared_ptr<etree::ModelNode>& model, const connection::visualization::GraphVizParams& params) {
            controller::getForegroundTasks().emplace("Visualize Connections with GraphViz", [editor, params, model](auto* progress) {
                *progress = .0f;
                auto& engine = editor->getConnectionEngine();
                engine.update(model);
                const auto& connectionGraph = engine.getConnections();
                *progress = .15f;
                auto graphvizCode = connection::visualization::generateGraphviz(connectionGraph, params, model);
                *progress = .3f;
                const auto outputFile = params.thumbnailDirectory / "connectionVisualization.png";
                graphvizCode.renderToFile(outputFile);
                graphvizCode.deleteTmpFiles = false;
                *progress = .99f;
                util::setStbiFlipVertically(false);
                texture = std::make_shared<graphics::Texture>(outputFile);
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
                params.thumbnailDirectory = util::replaceSpecialPaths(config::get(config::RENDERING_TMP_DIRECTORY));
                std::filesystem::create_directories(params.thumbnailDirectory);

                static Viewport viewport;

                if (texture != nullptr) {
                    drawImageCanvas(*texture, viewport);
                }

                ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
                if (ImGui::Button(ICON_FA_SLIDERS)) {
                    ImGui::OpenPopup(POPUP_ID);
                }

                if (ImGui::BeginPopup(POPUP_ID)) {
                    ImGui::Checkbox("Show Part Thumbnails", &params.showPartThumbnails);
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

                    if (ImGui::Button("Update")) {
                        updateImage(editorLocked, modelLocked, params);
                        ImGui::CloseCurrentPopup();
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
    }
}
