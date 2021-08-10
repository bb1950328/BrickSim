#include "window_model_info.h"
#include "../../controller.h"
#include "../gui_internal.h"
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

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
                nodes = {selectedLocked->getNode()};
            }
            spdlog::stopwatch sw;
            uint64_t sum = 0;
            for (const auto& node: nodes) {
                sum += countParts(node);
            }
            spdlog::debug("counted parts in {}", sw);
            ImGui::Text("Part count: %lu", sum);
        }
        ImGui::End();
    }
}