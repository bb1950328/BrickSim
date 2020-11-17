//
// Created by Bader on 15.11.2020.
//

#include "gui.h"
#include "../ldr_file_repository.h"
#include "../controller.h"
#include "gui_internal.h"
#include "../part_finder.h"

namespace gui {
    void windows::drawPartPaletteWindow(bool *show) {
        ImGui::Begin("Part palette", show);

        static char searchTextBuffer[128] = {'\0'};
        ImGui::InputText("##search", searchTextBuffer, 128);
        ImGui::SameLine();
        static int thumbnailZoomPercent = 100;//todo get from config
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        ImGui::DragInt("##Zoom", &thumbnailZoomPercent, 5, 10, 500, " Zoom: %d%%");
        static LdrColor *color = ldr_color_repo::get_color(1);//todo save in config
        const glm::vec3 &col = color->value.asGlmVector();
        const ImVec4 &txtColor = gui_internal::getWhiteOrBlackBetterContrast(col);
        ImGui::PushStyleColor(ImGuiCol_Text, txtColor);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x, col.y, col.z, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x, col.y, col.z, .8));

        static LdrColor *colorChosenInPopup;
        ImGui::SameLine();
        if (ImGui::Button(color->name.c_str())) {
            colorChosenInPopup = color;
            ImGui::OpenPopup("Part Palette Color");
        }
        ImGui::PopStyleColor(3);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Part Palette Color", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
            const ImVec2 &buttonSize = ImVec2(buttonWidth, buttonWidth);
            const int columnCount = 20;

            for (const auto &colorGroup: ldr_color_repo::getAllColorsGroupedAndSortedByHue()) {
                if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    int i = 0;
                    for (const auto *currentColor : colorGroup.second) {
                        if (i % columnCount > 0) {
                            ImGui::SameLine();
                        }
                        ImGui::PushID(currentColor->code);
                        const ImColor imColor = ImColor(currentColor->value.red, currentColor->value.green, currentColor->value.blue);
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) imColor);
                        if (ImGui::Button(colorChosenInPopup->code == currentColor->code ? "#" : "", buttonSize)) {
                            colorChosenInPopup = ldr_color_repo::get_color(currentColor->code);
                        }
                        ImGui::PopStyleColor(/*3*/1);
                        ImGui::PopID();
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", currentColor->name.c_str());
                            ImGui::EndTooltip();
                        }
                        ++i;
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply")) {
                color = colorChosenInPopup;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        static float categorySelectWidth = 250;//todo save
        const auto totalWidth = ImGui::GetContentRegionAvailWidth();
        const auto itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
        float thumbnailContainerWidth = totalWidth - categorySelectWidth - itemSpacingX;
        //static const auto partsGrouped = ldr_file_repo::getAllPartsGroupedByCategory();
        static const auto partCategories = ldr_file_repo::getAllCategories();
        static std::set<std::string> selectedCategories = {*partCategories.begin()};//first category preselected

        ImGui::BeginChild("##categorySelectTree", ImVec2(categorySelectWidth, 0));
        for (const auto &category : partCategories) {
            int flags = selectedCategories.find(category) != selectedCategories.end()
                        ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected
                        : ImGuiTreeNodeFlags_Leaf;
            if (ImGui::TreeNodeEx(category.c_str(), flags)) {
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    if (ImGui::GetIO().KeyCtrl) {
                        if (selectedCategories.find(category) == selectedCategories.end()) {
                            selectedCategories.insert(category);
                        } else {
                            selectedCategories.erase(category);
                        }
                    } else if (ImGui::GetIO().KeyShift) {
                        auto groupIt = partCategories.find(category);
                        while (groupIt != partCategories.begin() && selectedCategories.find(*groupIt) == selectedCategories.end()) {
                            selectedCategories.insert(*groupIt);
                            groupIt--;
                        }
                        selectedCategories.insert(*groupIt);
                    } else {
                        bool wasOnlySelectionBefore = selectedCategories.size() == 1 && *selectedCategories.begin() == category;
                        selectedCategories.clear();
                        if (!wasOnlySelectionBefore) {
                            selectedCategories.insert(category);
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##thumbnailsContainer", ImVec2(thumbnailContainerWidth, 0), ImGuiWindowFlags_AlwaysVerticalScrollbar);
        const static auto thumbnailSpacing = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(thumbnailSpacing, thumbnailSpacing));
        auto actualThumbSize = std::floor(controller::getThumbnailGenerator().size / 100.0 * thumbnailZoomPercent);
        auto actualThumbSizeSquared = ImVec2(actualThumbSize, actualThumbSize);
        int columns = std::max(1.0, std::floor((ImGui::GetContentRegionAvailWidth() + thumbnailSpacing) / (actualThumbSize + thumbnailSpacing)));
        int currentCol = 0;

        const bool searchEmpty = searchTextBuffer[0]=='\0';
        const auto& searchPredicate = part_finder::getPredicate(searchTextBuffer);

        if (selectedCategories.size() > 1) {
            for (const auto &category : selectedCategories) {
                ImGui::Text("%s", category.c_str());
                for (const auto &part : ldr_file_repo::getAllFilesOfCategory(category)) {
                    if (searchEmpty || searchPredicate.matches(*part)) {
                        gui_internal::drawPartThumbnail(actualThumbSizeSquared, part, color);
                        currentCol++;
                        if (currentCol == columns) {
                            currentCol = 0;
                        } else {
                            ImGui::SameLine();
                        }
                    }
                }
                if (currentCol != 0) {
                    ImGui::NewLine();
                }
                currentCol = 0;
            }
        } else if (selectedCategories.size() == 1) {
            for (const auto &part : ldr_file_repo::getAllFilesOfCategory(*selectedCategories.begin())) {
                if (searchEmpty || searchPredicate.matches(*part)) {
                    gui_internal::drawPartThumbnail(actualThumbSizeSquared, part, color);
                    currentCol++;
                    if (currentCol == columns) {
                        currentCol = 0;
                    } else {
                        ImGui::SameLine();
                    }
                }
            }
        } else {
            if (ldr_file_repo::areAllPartsLoaded()) {
                for (const auto &category : ldr_file_repo::getAllPartsGroupedByCategory()) {
                    bool textWritten = false;
                    for (const auto &part : category.second) {
                        if (searchEmpty || searchPredicate.matches(*part)) {
                            if (!textWritten) {
                                ImGui::Text("%s", category.first.c_str());
                                textWritten = true;
                            }
                            gui_internal::drawPartThumbnail(actualThumbSizeSquared, part, color);
                            currentCol++;
                            if (currentCol == columns) {
                                currentCol = 0;
                            } else {
                                ImGui::SameLine();
                            }
                        }
                    }
                    if (currentCol != 0) {
                        ImGui::NewLine();
                    }
                    currentCol = 0;
                }
            } else {
                static bool taskAdded = false;
                if (!taskAdded) {
                    controller::addBackgroundTask("Load remaining Parts", [](){
                        ldr_file_repo::getAllPartsGroupedByCategory();
                    });
                    taskAdded = true;
                }
                const auto loaded = ldr_file_repo::getLoadedPartsGroupedByCategory().size();
                const auto all = ldr_file_repo::getAllCategories().size();
                ImGui::ProgressBar(loaded*1.0f/all);
                ImGui::Text("%c %lu of %lu categories loaded, please wait", gui_internal::getLoFiSpinner(), loaded, all);
            }
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }
}