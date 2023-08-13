#include "icons.h"
#include "../constant_data/resources.h"
#include "../graphics/texture.h"
#include "gui_internal.h"
namespace bricksim::gui::icons {
    namespace {
        constexpr float step = 1.f / magic_enum::enum_count<IconType>();
        std::unique_ptr<graphics::Texture> texture;
    }
    ImVec2 getU(IconType icon) {
        return {*magic_enum::enum_index(icon) * step, 1.f};
    }
    ImVec2 getV(IconType icon) {
        return {(*magic_enum::enum_index(icon) + 1) * step, 0.f};
    }
    void initialize() {
        texture = std::make_unique<graphics::Texture>(resources::icons_png.data(), resources::icons_png.size());
    }
    void cleanup() {
        texture = nullptr;
    }
    ImTextureID getTexture() {
        return gui_internal::convertTextureId(texture->getID());
    }
    bool drawButton(IconType icon, float size) {
        return ImGui::ImageButton(magic_enum::enum_name(icon).data(), getTexture(), {size, size}, getU(icon), getV(icon));
    }
}
