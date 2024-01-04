#include "persistent_state.h"

#include "db.h"

namespace bricksim::persisted_state {
    namespace {
        PersistedState state;
        constexpr int64_t KEY = 0;
    }

    void initialize() {
        if (const auto value = db::state::get(KEY)) {
            state = json_dto::from_json<PersistedState>(*value);
        }
    }

    void cleanup() {
        db::state::set(KEY, json_dto::to_json(state));
    }

    PersistedState& get() {
        return state;
    }
}
