#pragma once

#include "../element_tree.h"
#include <string>
namespace bricksim::gui::node_context_menu {
    class ContextMenuDrawHandler {
    public:
        virtual bool beginMenu() const = 0;
        virtual bool drawAction(const std::string& name) const = 0;
        virtual void endMenu() const = 0;
    };
    class ImGuiContextMenuDrawHandler : public ContextMenuDrawHandler {
        bool beginMenu() const override;
        bool drawAction(const std::string& name) const override;
        void endMenu() const override;
    };

    void drawContextMenu(const std::shared_ptr<etree::Node>& context, const ContextMenuDrawHandler& drawHandler);
}
