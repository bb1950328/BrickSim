#include "snap_handler.h"

#include "../config.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <utility>

namespace bricksim::snap {
    void Handler::init() {
        enabled = config::get(config::SNAP_ENABLED);
        linear.init();
        rotational.init();
    }
    void Handler::cleanup() {
        linear.cleanup();
        rotational.cleanup();
        config::set(config::SNAP_ENABLED, enabled);
    }
    bool Handler::isEnabled() const {
        return enabled;
    }
    bool* Handler::isEnabledPtr() {
        return &enabled;
    }
    LinearHandler& Handler::getLinear() {
        return linear;
    }
    RotationalHandler& Handler::getRotational() {
        return rotational;
    }
    Handler::Handler() = default;
}
