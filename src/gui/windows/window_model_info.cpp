#include "window_model_info.h"
#include "../../controller.h"
#include "../gui_internal.h"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <inttypes.h>

namespace bricksim::gui::windows::model_info {

    uint64_t countParts(const std::shared_ptr<etree::Node>& node, bool countSubfile = false) {
        const etree::NodeType nodeType = node->getType();

        if (nodeType == etree::TYPE_PART) {
            const char* fileName = std::dynamic_pointer_cast<etree::PartNode>(node)->ldrFile->metaInfo.name.c_str();
            return strstr(constants::SEGMENT_PART_NAMES, fileName) == nullptr ? 1 : 0;
        } else if (nodeType == etree::TYPE_MPD_SUBFILE_INSTANCE) {
            return countParts(std::dynamic_pointer_cast<etree::MpdSubfileInstanceNode>(node)->mpdSubfileNode, true);
        } else if (nodeType != etree::TYPE_MPD_SUBFILE || countSubfile) {
            uint64_t sum = 0;
            for (const auto& child: node->getChildren()) {
                sum += countParts(child);
            }
            return sum;
        }
        return 0;
    }

    void draw(bricksim::gui::windows::Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            static std::weak_ptr<Editor> selectedEditor;
            gui_internal::drawEditorSelectionCombo(selectedEditor, "Model");
            auto selectedLocked = selectedEditor.lock();
            uoset_t<std::shared_ptr<etree::Node>> nodes = selectedLocked->getSelectedNodes();
            if (!nodes.empty()) {
                ImGui::Text("Statistics of the %zu selected elements:", nodes.size());
            } else {
                ImGui::Text("Model Statistics:");
                nodes = {selectedLocked->getDocumentNode()};
            }
            spdlog::stopwatch sw;
            uint64_t sum = 0;
            for (const auto& node: nodes) {
                sum += countParts(node);
            }
            spdlog::debug("counted parts in {}", sw);
            ImGui::Text("Part count: %" PRIu64, sum);
            mesh::AxisAlignedBoundingBox modelDimensions;
            for (const auto& node : nodes) {
                const auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node);
                if (meshNode != nullptr) {
                    modelDimensions.addAABB(selectedLocked->getScene()->getMeshCollection().getAbsoluteAABB(meshNode));
                }
            }
            if (modelDimensions.isDefined()) {
                const auto size = modelDimensions.getSize();
                ImGui::Text("Width (X): %.0f LDU", std::abs(size.x));
                ImGui::Text("Height (Y): %.0f LDU", std::abs(size.y));
                ImGui::Text("Length (Z): %.0f LDU", std::abs(size.z));
            }
        }
        ImGui::End();
    }
}
