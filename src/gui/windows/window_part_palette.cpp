#include "../../controller.h"
#include "../../ldr/colors.h"
#include "../../ldr/file_repo.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../../part_finder.h"
#include "../gui.h"
#include "../gui_internal.h"

#include "window_part_palette.h"

#include "../../persistent_state.h"

namespace bricksim::gui::windows::part_palette {
    namespace {
        bool nodeSelectReverseRecursively(oset_t<uint64_t>& set, const config::PartCategoryTreeNode& node) {
            if (set.contains(node.id)) {
                return true;
            }
            set.insert(node.id);
            for (auto childIt = node.children.rbegin(); childIt != node.children.rend(); ++childIt) {
                if (nodeSelectReverseRecursively(set, *childIt)) {
                    return true;
                }
            }
            return false;
        }

        ///iterates through the flattened tree in reverse, adds all ids to set, immediately returns true when it encounters a node which is already in set
        bool nodeSelectReverse(oset_t<uint64_t>& set, const std::vector<config::PartCategoryTreeNode>& nodeStack, const std::size_t stackOffset = 0) {
            const auto& node = nodeStack.back();
            set.insert(node.id);
            if (stackOffset - 1 > nodeStack.size()) {
                const auto& parent = nodeStack[nodeStack.size() - 1 - stackOffset];
                for (const auto& previousSibling: parent.children) {
                    if (previousSibling.id == node.id) {
                        break;
                    }
                    if (nodeSelectReverseRecursively(set, previousSibling)) {
                        return true;
                    }
                }
                if (nodeSelectReverse(set, nodeStack, stackOffset + 1)) {
                    return true;
                }
            }
            return false;
        }

        void drawCustomTreeNode(std::vector<config::PartCategoryTreeNode>& nodeStack, persisted_state::PartPalette& state);

        void drawCustomTreeNodeChildren(std::vector<config::PartCategoryTreeNode>& nodeStack, persisted_state::PartPalette& state) {
            for (const auto& child: nodeStack.back().children) {
                nodeStack.push_back(child);
                drawCustomTreeNode(nodeStack, state);
                nodeStack.pop_back();
            }
        }

        void drawCustomTreeNode(std::vector<config::PartCategoryTreeNode>& nodeStack, persisted_state::PartPalette& state) {
            const auto& node = nodeStack.back();
            int flags = ImGuiTreeNodeFlags_None;
            const auto initiallySelected = state.selectedTreeElements.contains(node.id);
            if (initiallySelected) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (node.children.empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }
            if (ImGui::TreeNodeEx(reinterpret_cast<void*>(node.id), flags, "%s", node.name.c_str())) {
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    if (ImGui::GetIO().KeyCtrl) {
                        if (initiallySelected) {
                            state.selectedTreeElements.erase(node.id);
                        } else {
                            state.selectedTreeElements.insert(node.id);
                        }
                    } else if (ImGui::GetIO().KeyShift) {
                        nodeSelectReverse(state.selectedTreeElements, nodeStack);
                    } else {
                        state.selectedTreeElements = {node.id};
                    }
                }
                drawCustomTreeNodeChildren(nodeStack, state);
                ImGui::TreePop();
            }
        }

        void drawDefaultCategoryTree(persisted_state::PartPalette& state) {
            const auto& partCategories = ldr::file_repo::get().getAllCategories();
            uint64_t i = 0;
            auto it = partCategories.begin();
            while (it != partCategories.end()) {
                const auto initiallySelected = state.selectedTreeElements.contains(i);
                if (ImGui::Selectable(it->c_str(), initiallySelected)) {
                    if (initiallySelected) {
                        if (state.selectedTreeElements.size() > 1) {
                            state.selectedTreeElements = {i};
                        } else {
                            state.selectedTreeElements.clear();
                        }
                    } else {
                        if (ImGui::GetIO().KeyCtrl) {
                            state.selectedTreeElements.insert(i);
                        } else if (ImGui::GetIO().KeyShift) {
                            uint64_t x = i;
                            while (x > 0 && !state.selectedTreeElements.contains(x)) {
                                state.selectedTreeElements.insert(x);
                                --x;
                            }
                        } else {
                            state.selectedTreeElements = {i};
                        }
                    }
                }
                ++i;
                ++it;
            }
        }

        //todo these algorithms can be optimized from O(n) to O(log(N)) by using binary search while looking for the next child
        std::optional<config::PartCategoryTreeNode> findNode(const config::PartCategoryTreeNode& root, const uint64_t id) {
            if (root.id == id) {
                return root;
            }
            if (!root.children.empty()) {
                std::size_t childIdx = 0;
                while (root.children.size() - 1 > childIdx && root.children[childIdx + 1].id < id) {
                    ++childIdx;
                }
                while (childIdx < root.children.size()) {
                    if (const auto res = findNode(root.children[childIdx], id)) {
                        return res;
                    }
                    ++childIdx;
                }
            }
            return std::nullopt;
        }

        std::optional<config::PartCategoryTreeNode> findNode(std::vector<config::PartCategoryTreeNode>& nodeStack, const uint64_t id) {
            const auto& node = nodeStack.back();
            if (node.id > id) {
                return std::nullopt;
            } else if (node.id == id) {
                return node;
            }
            std::size_t childIdx = 0;
            while (childIdx < node.children.size() - 1 && node.children[childIdx + 1].id < id) {
                ++childIdx;
            }
            while (childIdx < node.children.size()) {
                nodeStack.push_back(node.children[childIdx]);
                if (const auto res = findNode(nodeStack, id)) {
                    return res;
                }
                nodeStack.pop_back();
                ++childIdx;
            }
            if (nodeStack.size() > 1) {
                nodeStack.pop_back();
                return findNode(nodeStack, id);
            }
            return std::nullopt;
        }
    }


    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            static char searchTextBuffer[128] = {'\0'};
            ImGui::InputText(ICON_FA_MAGNIFYING_GLASS "##search", searchTextBuffer, 128);
            ImGui::SameLine();
            static int thumbnailZoomPercent = 100;//todo get from config
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::DragInt("##Zoom", &thumbnailZoomPercent, 5, 10, 500, " Zoom: %d%%");
            static auto color = ldr::color_repo::getColor(1);//todo save in config
            const glm::vec3& col = color->value.asGlmVector();
            ImGui::PushStyleColor(ImGuiCol_Text, gui_internal::getWhiteOrBlackBetterContrast(col));
            ImGui::PushStyleColor(ImGuiCol_Button, color->value);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, glm::vec4(col, .8));

            ldr::ColorReference static colorChosenInPopup;
            ImGui::SameLine();
            if (ImGui::Button(color->name.c_str())) {
                colorChosenInPopup = color->asReference();
                ImGui::OpenPopup(ICON_FA_SWATCHBOOK " Part Palette Color");
            }
            ImGui::PopStyleColor(3);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal(ICON_FA_SWATCHBOOK " Part Palette Color", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
                const ImVec2& buttonSize = ImVec2(buttonWidth, buttonWidth);
                const int columnCount = 20;

                for (const auto& colorGroup: ldr::color_repo::getAllColorsGroupedAndSortedByHue()) {
                    if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        int i = 0;
                        for (const auto& currentColor: colorGroup.second) {
                            const auto currentColorValue = currentColor.get();
                            if (i % columnCount > 0) {
                                ImGui::SameLine();
                            }
                            ImGui::PushID(currentColorValue->code);
                            ImGui::PushStyleColor(ImGuiCol_Button, currentColorValue->value);
                            if (ImGui::Button(colorChosenInPopup.code == currentColorValue->code ? ICON_FA_CHECK : "", buttonSize)) {
                                colorChosenInPopup = currentColorValue->code;
                            }
                            ImGui::PopStyleColor(/*3*/ 1);
                            ImGui::PopID();
                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::Text("%s", currentColorValue->name.c_str());
                                ImGui::EndTooltip();
                            }
                            ++i;
                        }
                        ImGui::TreePop();
                    }
                }
                if (ImGui::Button(ICON_FA_RECTANGLE_XMARK " Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_SQUARE_CHECK " Apply")) {
                    color = colorChosenInPopup.get();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::BeginChild("##categorySelectTree", ImVec2(160.f * config::get().gui.scale, 0), ImGuiChildFlags_ResizeX);

            auto& customTrees = config::get().partPalette.customTrees;
            auto& state = persisted_state::get().partPalette;
            if (!customTrees.empty()) {
                if (state.selectedCustomTree < -1 || state.selectedCustomTree >= customTrees.size()) {
                    state.selectedCustomTree = -1;
                    state.selectedTreeElements.clear();
                }
                const char* currentTreeName = state.selectedCustomTree == -1 ? "Default" : customTrees[state.selectedCustomTree].name.c_str();
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##customTreeCombo", currentTreeName)) {
                    if (ImGui::Selectable("Default", state.selectedCustomTree == -1)) {
                        state.selectedCustomTree = -1;
                        state.selectedTreeElements.clear();
                    }
                    for (std::size_t i = 0; i < customTrees.size(); ++i) {
                        if (ImGui::Selectable(customTrees[i].name.c_str(), state.selectedCustomTree == i)) {
                            state.selectedCustomTree = i;
                            state.selectedTreeElements.clear();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            static std::optional<std::shared_ptr<Editor>> selectedEditor;
            for (const auto& editor: controller::getEditors()) {
                const auto label = fmt::format(ICON_FA_FILE" {}", editor->getDisplayName());
                if (ImGui::Selectable(label.c_str(), editor == selectedEditor)) {
                    selectedEditor = editor;
                    state.selectedTreeElements.clear();
                }
            }

            if (state.selectedCustomTree == -1) {
                drawDefaultCategoryTree(state);
            } else {
                std::vector nodeStack = {customTrees[state.selectedCustomTree]};
                drawCustomTreeNodeChildren(nodeStack, state);
                if (!state.selectedTreeElements.empty()) {
                    selectedEditor = std::nullopt;
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("##thumbnailsContainer", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None);
            static constexpr auto thumbnailSpacing = 4;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(thumbnailSpacing, thumbnailSpacing));
            const auto actualThumbSize = std::floor(controller::getThumbnailGenerator()->size / 100.0 * thumbnailZoomPercent);
            const auto actualThumbSizeSquared = ImVec2(actualThumbSize, actualThumbSize);

            const bool searchEmpty = searchTextBuffer[0] == '\0';
            const auto& searchPredicate = part_finder::getPredicate(searchTextBuffer);

            const auto drawPart = [&actualThumbSizeSquared, &searchEmpty, &searchPredicate](const std::shared_ptr<ldr::File>& file) {
                if (searchEmpty || searchPredicate.matches(*file)) {
                    gui_internal::drawPartThumbnail(actualThumbSizeSquared, file, color->asReference());
                    ImGui::SameLine();
                    if (ImGui::GetContentRegionAvail().x < actualThumbSizeSquared.x) {
                        ImGui::NewLine();
                    }
                }
            };

            const auto drawEditor = [&drawPart](const std::shared_ptr<Editor>& editor) {
                for (const auto& item: editor->getRootNode()->getChildren()) {
                    const auto modelNode = std::dynamic_pointer_cast<etree::ModelNode>(item);
                    if (modelNode != nullptr) {
                        drawPart(modelNode->ldrFile);
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                            editor->openContextMenuNodeSelectedOrClicked(modelNode);
                        }
                    }
                }
            };

            const auto drawCategory = [&drawPart](const std::string& category) {
                for (const auto& part: ldr::file_repo::get().getAllFilesOfCategory(category)) {
                    drawPart(part);
                }
                ImGui::NewLine();
            };

            static uomap_t<std::pair<std::string, std::string>, oset_t<std::shared_ptr<ldr::File>>> customCategoryCache;
            const auto getCustomPartList = [](const std::string& ldrawCategory, const std::string& nameFilter) {
                if (nameFilter.empty()) {
                    return ldr::file_repo::get().getAllFilesOfCategory(ldrawCategory);
                } else {
                    const auto key = std::make_pair(ldrawCategory, nameFilter);
                    if (const auto it = customCategoryCache.find(key); it != customCategoryCache.end()) {
                        return it->second;
                    }
                    const auto& matcher = part_finder::getPredicate(nameFilter);
                    oset_t<std::shared_ptr<ldr::File>> result;
                    for (const auto& file: ldr::file_repo::get().getAllFilesOfCategory(ldrawCategory)) {
                        if (matcher.matches(*file)) {
                            result.insert(file);
                        }
                    }
                    return customCategoryCache.emplace(key, result).first->second;
                }
            };

            const auto drawCustomCategory = [&drawPart, &getCustomPartList](const config::PartCategoryTreeNode& category) {
                for (const auto& part: getCustomPartList(category.ldrawCategory, category.nameFilter)) {
                    drawPart(part);
                }
                ImGui::NewLine();
            };

            if (selectedEditor.has_value()) {
                drawEditor(*selectedEditor);
            } else if (!state.selectedTreeElements.empty()) {
                const bool drawHeaders = state.selectedTreeElements.size() != 1;
                if (state.selectedCustomTree == -1) {
                    static uomap_t<uint64_t, std::string> categoryNamesByIndex;
                    if (categoryNamesByIndex.empty()) {
                        const auto& partCategories = ldr::file_repo::get().getAllCategories();
                        uint64_t i = 0;
                        auto it = partCategories.begin();
                        while (it != partCategories.end()) {
                            categoryNamesByIndex.emplace(i, *it);
                            ++i;
                            ++it;
                        }
                    }
                    for (auto element: state.selectedTreeElements) {
                        const auto& categoryName = categoryNamesByIndex.find(element)->second;
                        if (drawHeaders) {
                            ImGui::Text("%s", categoryName.c_str());
                        }
                        drawCategory(categoryName);
                    }
                } else {
                    const auto& root = customTrees[state.selectedCustomTree];
                    if (state.selectedTreeElements.size() == 1) {
                        drawCustomCategory(*findNode(root, *state.selectedTreeElements.begin()));
                    } else {
                        std::vector nodeStack = {root};
                        for (const auto element: state.selectedTreeElements) {
                            const auto category = *findNode(nodeStack, element);
                            if (drawHeaders) {
                                ImGui::Text("%s", category.name.c_str());
                            }
                            drawCustomCategory(category);
                        }
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
