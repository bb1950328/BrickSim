#pragma once

#include <string>
#include "camera.h"

namespace bricksim::graphics::connection_visualization {
    void initializeIfNeeded();

    void setVisualizedPart(const std::string &partName);

    unsigned int getImage();

    const std::shared_ptr<CadCamera> &getCamera();

    void cleanupIfNeeded();
}
