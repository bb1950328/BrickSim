#pragma once

#include "../../editor.h"
#include "../../element_tree.h"
namespace bricksim::gui::node_context_menu {

    struct Context {
        std::shared_ptr<Editor> editor;
        std::shared_ptr<etree::Node> node;
    };

    void openContextMenu(Context newContext);

    void drawContextMenu();
}
