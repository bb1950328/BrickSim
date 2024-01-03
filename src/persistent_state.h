#pragma once
#include "helpers/json_helper.h"

namespace bricksim::persisted_state {
    using namespace json_helper;

    struct SnappingState {
        bool enabled;
        uint64_t linearStepXZ;
        uint64_t linearStepY;
        float rotationalStep;

        SnappingState() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("enabled", enabled, true)
                    & json_dto::optional("linearStepXZ", linearStepXZ, 20)
                    & json_dto::optional("linearStepY", linearStepY, 20)
                    & json_dto::optional("rotationalStep", rotationalStep, 90.f);
        }
    };

    struct PersistedState {
        uint16_t windowWidth;
        uint16_t windowHeight;
        SnappingState snapping;

        PersistedState() {
            defaultInit(this);
        }

        template<typename JsonIo>
        void json_io(JsonIo& io) {
            io
                    & json_dto::optional("windowWidth", windowWidth, 1280)
                    & json_dto::optional("windowHeight", windowHeight, 720)
                    & json_dto::optional("snapping", snapping, SnappingState{});
        }
    };

    void initialize();
    void cleanup();
    [[nodiscard]] PersistedState& get();
}
