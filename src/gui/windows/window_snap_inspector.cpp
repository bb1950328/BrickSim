#include "window_snap_inspector.h"
#include "../../ldr/file_repo.h"
#include "imgui.h"
#include "spdlog/spdlog.h"

namespace bricksim::gui::windows::snap_inspector {
    namespace {
        std::shared_ptr<ldr::File> currentFile = nullptr;

        int partNameInputCallback(ImGuiInputTextCallbackData* data) {
            auto& fileRepo = ldr::file_repo::get();
            if (fileRepo.hasFileCached(data->Buf)) {
                currentFile = fileRepo.getFile(data->Buf);
            } else {
                const auto extendedName = std::string(data->Buf) + ".dat";
                if (fileRepo.hasFileCached(extendedName)) {
                    const auto lengthBefore = data->BufTextLen;
                    data->InsertChars(lengthBefore, ".dat");
                    data->CursorPos = lengthBefore;
                    currentFile = fileRepo.getFile(extendedName);
                } else {
                    currentFile = nullptr;
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
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            char partName[256] = {0};
            ImGui::InputText("Part Name", partName, sizeof(partName), ImGuiInputTextFlags_CallbackAlways, partNameInputCallback);
            if (currentFile != nullptr) {
                ImGui::Text("Snap Meta Info for %s", currentFile->metaInfo.title.c_str());
                static std::weak_ptr<connection::ldcad_snap_meta::MetaCommand> currentlySelected;
                showSnapLineNodes(currentFile, currentlySelected);
            } else {
                ImGui::Text("No file named \"%s\" in memory.", partName);
            }
        }
        ImGui::End();
    }
}