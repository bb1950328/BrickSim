#include "window_model_info.h"
#include "../../config.h"
#include "../../controller.h"
#include "../../helpers/stringutil.h"
#include "../../info_providers/price_guide_provider.h"
#include "../gui_internal.h"
#include <imgui.h>
#include <inttypes.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

namespace bricksim::gui::windows::model_info {

    namespace {
        void addTableLine(const char* const description) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", description);
            ImGui::TableNextColumn();
        }

        struct PartStats {
            uint64_t count = 0;
            float totalMinPrice = 0;
            float totalAvgPrice = 0;
            std::set<std::size_t> differentParts;
            std::set<std::size_t> differentPartsAndColors;
            void reset() {
                count = 0;
                totalMinPrice = 0;
                totalAvgPrice = 0;
                differentParts.clear();
                differentPartsAndColors.clear();
            }
        };
    }

    void fillPartStats(PartStats& stats, const std::shared_ptr<etree::Node>& node, bool countSubfile = false) {
        const etree::NodeType nodeType = node->getType();

        if (nodeType == etree::NodeType::TYPE_PART) {
            const auto ldrNode = std::dynamic_pointer_cast<etree::PartNode>(node);
            const char* fileName = ldrNode->ldrFile->metaInfo.name.c_str();

            auto partCode = ldrNode->ldrFile->metaInfo.name;
            stringutil::replaceAll(partCode, ".dat", "");
            auto ldrColorName = ldr::color_repo::getColor(ldrNode->getDisplayColor().code)->name;
            auto priceGuide = info_providers::price_guide::getPriceGuideIfCached(partCode, config::get(config::BRICKLINK_CURRENCY_CODE), util::translateLDrawColorNameToBricklink(ldrColorName));
            if (priceGuide.has_value()) {
                stats.totalMinPrice += priceGuide.value().minPrice;
                stats.totalAvgPrice += priceGuide.value().avgPrice;
            }

            if (strstr(constants::SEGMENT_PART_NAMES, fileName) == nullptr) {
                stats.count += 1;
            }

            const auto nameHash = util::combinedHash(ldrNode->ldrFile->metaInfo.name);
            stats.differentParts.insert(nameHash);
            stats.differentPartsAndColors.insert(util::combinedHash(ldrNode->getDisplayColor(), nameHash));

        } else if (nodeType == etree::NodeType::TYPE_MPD_SUBFILE_INSTANCE) {
            fillPartStats(stats, std::dynamic_pointer_cast<etree::MpdSubfileInstanceNode>(node)->mpdSubfileNode, true);
        } else if (nodeType != etree::NodeType::TYPE_MPD_SUBFILE || countSubfile) {
            for (const auto& child: node->getChildren()) {
                fillPartStats(stats, child);
            }
        }
    }

    void draw(bricksim::gui::windows::Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            static std::weak_ptr<Editor> selectedEditor;
            gui_internal::drawEditorSelectionCombo(selectedEditor, "Model");
            auto selectedLocked = selectedEditor.lock();
            auto nodes = selectedLocked->getSelectedNodes();
            if (nodes.empty()) {
                ImGui::Text("Model Statistics:");
                nodes.emplace(selectedLocked->getDocumentNode(), 0);
            } else {
                ImGui::Text("Statistics of the %zu selected elements:", nodes.size());
            }
            spdlog::stopwatch sw;
            static PartStats partStats;
            static uomap_t<std::shared_ptr<etree::Node>, uint64_t> lastNodesAndVersions;
            bool nodesUnchanged = true;
            if (lastNodesAndVersions.size() == nodes.size()) {
                for (const auto& item: nodes) {
                    const auto& it = lastNodesAndVersions.find(item.first);
                    if (it == lastNodesAndVersions.end() || item.first->getVersion() != it->second) {
                        nodesUnchanged = false;
                        break;
                    }
                }
            } else {
                nodesUnchanged = false;
            }
            if (!nodesUnchanged) {
                partStats.reset();
                for (const auto& node: nodes) {
                    fillPartStats(partStats, node.first);
                }
                lastNodesAndVersions.clear();
                for (const auto& item: nodes) {
                    lastNodesAndVersions.insert({item.first, item.first->getVersion()});
                }
                spdlog::debug("counted parts in {}", sw);
            }

            if (ImGui::BeginTable("statisticsTable", 2)) {
                addTableLine("Part count");
                ImGui::Text("%" PRIu64, partStats.count);

                addTableLine("Different parts");
                ImGui::Text("%" PRIu64, partStats.differentParts.size());

                addTableLine("Different parts and colors");
                ImGui::Text("%" PRIu64, partStats.differentPartsAndColors.size());

                mesh::AxisAlignedBoundingBox modelDimensions;
                for (const auto& node: nodes) {
                    const auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node.first);
                    if (meshNode != nullptr) {
                        modelDimensions.includeAABB(selectedLocked->getScene()->getMeshCollection().getAbsoluteAABB(meshNode));
                    }
                }
                if (modelDimensions.isDefined()) {
                    const auto size = modelDimensions.getSize();
                    addTableLine("Width (X)");
                    ImGui::Text("%.0f LDU", std::abs(size.x));
                    addTableLine("Height (Y)");
                    ImGui::Text("%.0f LDU", std::abs(size.y));
                    addTableLine("Length (Z)");
                    ImGui::Text("%.0f LDU", std::abs(size.z));
                }

                const auto currency = config::get(config::BRICKLINK_CURRENCY_CODE);
                addTableLine("Min Price*");
                ImGui::Text("%s %.2f", currency.c_str(), partStats.totalMinPrice);
                addTableLine("AVG Price*");
                ImGui::Text("%s %.2f", currency.c_str(), partStats.totalAvgPrice);

                ImGui::EndTable();
            }
            ImGui::Text("*Cached Price Guides only");
        }
        ImGui::End();
    }
}
