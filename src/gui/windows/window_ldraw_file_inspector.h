#pragma once

#include "../../ldr/files.h"
#include "windows.h"

namespace bricksim::gui::windows::ldraw_file_inspector {
    void setCurrentFile(const std::shared_ptr<ldr::File>& newFile);
    void draw(Data& data);
}
