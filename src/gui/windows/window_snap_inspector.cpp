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
    }
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            char partName[256] = {0};
            ImGui::InputText("Part Name", partName, sizeof(partName), ImGuiInputTextFlags_CallbackAlways, partNameInputCallback);
            if (currentFile != nullptr) {
                ImGui::Text("Snap Meta Info for %s", currentFile->metaInfo.title.c_str());
            } else {
                ImGui::Text("No file named \"%s\" in memory.", partName);
            }
        }
        ImGui::End();
    }
}