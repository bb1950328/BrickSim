// overlay_2d.cpp.c
// Created by bab21 on 26.12.20.
//

#include "overlay_2d.h"

void Overlay2dCollection::rebuild(element_id_t firstElementId) {

}

bool Overlay2dCollection::clickEvent(element_id_t elementId) {
    if (elementId < firstElementId || elementId > lastElementIds.back()) {
        return false;
    }
    for (int i = 0; i < overlays.size(); ++i) {

    }
}
