#pragma once
#include "helpers/json_helper.h"

namespace bricksim::persisted_state {
    struct SnappingState {
        bool enabled;
        uint64_t linearStepXZ;
        uint64_t linearStepY;
        float rotationalStep;

        SnappingState() {
            json_helper::defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("enabled", enabled, true)
                    & json_dto::optional("linearStepXZ", linearStepXZ, 20)
                    & json_dto::optional("linearStepY", linearStepY, 20)
                    & json_dto::optional("rotationalStep", rotationalStep, 90.f);
        }

        friend bool operator==(const SnappingState& lhs, const SnappingState& rhs) {
            return lhs.enabled == rhs.enabled
                   && lhs.linearStepXZ == rhs.linearStepXZ
                   && lhs.linearStepY == rhs.linearStepY
                   && lhs.rotationalStep == rhs.rotationalStep;
        }

        friend bool operator!=(const SnappingState& lhs, const SnappingState& rhs) { return !(lhs == rhs); }
    };

    struct PersistedState {
        uint16_t windowWidth;
        uint16_t windowHeight;
        SnappingState snapping;

        PersistedState() {
            json_helper::defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("windowWidth", windowWidth, 1280)
                    & json_dto::optional("windowHeight", windowHeight, 720)
                    & json_dto::optional("snapping", snapping, SnappingState{});
        }

        friend bool operator==(const PersistedState& lhs, const PersistedState& rhs) {
            return lhs.windowWidth == rhs.windowWidth
                   && lhs.windowHeight == rhs.windowHeight
                   && lhs.snapping == rhs.snapping;
        }

        friend bool operator!=(const PersistedState& lhs, const PersistedState& rhs) { return !(lhs == rhs); }
    };

    void initialize();
    void cleanup();
    [[nodiscard]] PersistedState& get();
}
