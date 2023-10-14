#pragma once

#include "../../editor/editor.h"
#include "../../element_tree.h"
namespace bricksim::gui::node_context_menu {

    struct Context {
        std::weak_ptr<Editor> editor;
        std::vector<std::weak_ptr<etree::Node>> nodes;
    };

    void openContextMenu(Context newContext);

    void drawContextMenu();
}
