#pragma once

#include "../../element_tree.h"
#include "windows.h"

namespace bricksim::gui::windows::element_properties {

    void drawTransformationEdit(const std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::Node>& node);
    void drawDisplayNameEdit(std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::Node>& node);
    void drawType(const std::shared_ptr<etree::Node>& node);
    void drawDeleteButton(const std::shared_ptr<Editor>& activeEditor);
    void drawLayerEdit(const std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::Node>& node);
    void drawColorEdit(const std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::LdrNode>& node);
    void drawPriceGuide(const std::shared_ptr<etree::Node>& node);

    void draw(Data& data);
}
