#include "window_ldraw_file_inspector.h"
#include "../../ldr/file_repo.h"
#include "../../ldr/file_writer.h"
#include "imgui.h"
#include "spdlog/fmt/bundled/format.h"

namespace bricksim::gui::windows::ldraw_file_inspector {
    namespace {
        std::shared_ptr<ldr::File> currentFile = nullptr;
        std::string content;

        void currentFileChanged() {
            if (currentFile != nullptr) {
                std::stringstream sstr;
                ldr::writeFile(currentFile, sstr, currentFile->metaInfo.name);
                content = sstr.str();
            } else {
                content = "";
            }
        }

        int partNameInputCallback(ImGuiInputTextCallbackData* data) {
            auto& fileRepo = ldr::file_repo::get();
            if (fileRepo.hasFileCached(data->Buf)) {
                setCurrentFile(fileRepo.getFile(data->Buf));
            } else {
                const auto extendedName = std::string(data->Buf) + ".dat";
                if (fileRepo.hasFileCached(extendedName)) {
                    const auto lengthBefore = data->BufTextLen;
                    data->InsertChars(lengthBefore, ".dat");
                    data->CursorPos = lengthBefore;
                    setCurrentFile(fileRepo.getFile(extendedName));
                } else {
                    setCurrentFile(nullptr);
                }
            }
            return 0;
        }

        void showSnapLineNodes(const std::shared_ptr<ldr::File>& file, std::weak_ptr<connection::ldcad_snap_meta::MetaCommand>& currentlySelected) {
            for (const auto& item: file->ldcadSnapMetas) {
                auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (currentlySelected.lock() == item) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }
                ImGui::TreeNodeEx(reinterpret_cast<const void*>(item.get()), flags, "%s", item->to_string().c_str());
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    currentlySelected = item;
                }
            }
            for (const auto& item: file->elements) {
                if (item->getType() == 1) {
                    const auto subfileRef = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                    const auto subfile = subfileRef->getFile();
                    if (!subfile->ldcadSnapMetas.empty()) {
                        if (ImGui::TreeNode(subfileRef.get(), "%s", subfileRef->filename.c_str())) {
                            showSnapLineNodes(subfile, currentlySelected);
                            ImGui::TreePop();
                        }
                    }
                }
            }
        }
    }

    void setCurrentFile(const std::shared_ptr<ldr::File>& newFile) {
        if (currentFile != newFile) {
            currentFile = newFile;
            currentFileChanged();
        }
    }

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            char partName[256] = {0};
            ImGui::InputText("Part Name", partName, sizeof(partName), ImGuiInputTextFlags_CallbackAlways, partNameInputCallback);

            if (ImGui::BeginListBox("All Files")) {
                for (const auto& item: ldr::file_repo::get().getAllFilesInMemory()) {
                    const auto text = fmt::format("{}: {}", item.first, item.second.second->metaInfo.title);
                    if (ImGui::Selectable(text.c_str(), item.second.second == currentFile)) {
                        setCurrentFile(item.second.second);
                    }
                }
                ImGui::EndListBox();
            }

            if (currentFile != nullptr) {
                ImGui::Separator();

                ImGui::Text("Inspecting %s: %s", currentFile->metaInfo.name.c_str(), currentFile->metaInfo.title.c_str());

                if (ImGui::BeginTabBar("##fileInspectorTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
                    if (ImGui::BeginTabItem("Snap Info")) {
                        static std::weak_ptr<connection::ldcad_snap_meta::MetaCommand> currentlySelected;
                        showSnapLineNodes(currentFile, currentlySelected);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Raw Content")) {
                        if (ImGui::BeginChild("Content", ImVec2(0, 0), true, ImGuiWindowFlags_None)) {
                            ImGui::TextUnformatted(content.c_str());
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

            } else {
                ImGui::Text("No file named \"%s\" in memory.", partName);
            }
        }
        ImGui::End();
    }
}