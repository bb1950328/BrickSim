#include "persistent_state.h"

namespace bricksim::persisted_state {
    namespace {
        PersistedState state;
    }

    void initialize() {
        //todo create new record in db
    }

    void cleanup() {
        //todo delete record if not the only one
    }

    PersistedState& get() {
        return state;
    }
}
