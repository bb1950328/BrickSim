#include "snap_handler.h"

#include "../persistent_state.h"
#include "../config/read.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <utility>

namespace bricksim::snap {
    void Handler::init() {
        enabled = persisted_state::get().snapping.enabled;//todo inline this member
        linear.init();
        rotational.init();
    }
    void Handler::cleanup() {
        linear.cleanup();
        rotational.cleanup();
        persisted_state::get().snapping.enabled = enabled;
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
    const LinearHandler& Handler::getLinear() const {
        return linear;
    }
    const RotationalHandler& Handler::getRotational() const {
        return rotational;
    }
    Handler::Handler() = default;
}
