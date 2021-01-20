//
// Created by bb1950328 on 20.01.2021.
//

#include "gui.h"
#include "../latest_log_messages_tank.h"

namespace gui {
    void windows::drawLogWindow(bool *show){
        if (ImGui::Begin(ICON_FA_LIST " Log", show)) {
            auto it = latest_log_messages_tank::getIterator();
            while (it.getCurrent()!= nullptr) {
                //todo some nice display using table
                ImGui::Text("%s", *it.getCurrent()->message);
                it.operator++();
            }
        }
        ImGui::End();
    }
}