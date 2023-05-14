#pragma once

#include "../editor.h"
#include "../element_tree.h"
#include <string>
namespace bricksim::gui::node_context_menu {
    class ContextMenuDrawHandler {
    public:
        ContextMenuDrawHandler();
        virtual ~ContextMenuDrawHandler();

        [[nodiscard]] virtual bool beginMenu() const = 0;
        [[nodiscard]] virtual bool beginSubMenu(const std::string& name) const;
        [[nodiscard]] virtual bool beginSubMenu(const std::string& name, std::optional<color::RGB> color) const = 0;
        [[nodiscard]] virtual bool drawAction(const std::string& name, std::optional<color::RGB> color) const = 0;
        [[nodiscard]] virtual bool drawAction(const std::string& name) const;
        virtual void endSubMenu() const = 0;
        virtual void endMenu() const = 0;
    };
    class ImGuiContextMenuDrawHandler : public ContextMenuDrawHandler {
    public:
        ImGuiContextMenuDrawHandler();
        ~ImGuiContextMenuDrawHandler() override;

        [[nodiscard]] bool beginMenu() const override;
        [[nodiscard]] bool beginSubMenu(const std::string& name, std::optional<color::RGB> color) const override;
        [[nodiscard]] bool drawAction(const std::string& name, std::optional<color::RGB> color) const override;
        void endSubMenu() const override;
        void endMenu() const override;
    };

    struct Context {
        std::shared_ptr<Editor> editor;
        std::shared_ptr<etree::Node> node;
    };

    void openContextMenu(Context newContext);

    void drawContextMenu();
}
