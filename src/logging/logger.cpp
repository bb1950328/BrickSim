#include "logger.h"
#include "latest_log_messages_tank.h"
#include <memory>

namespace bricksim::logging {

    void initialize() {
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e | %-8!l | T%t | %v");
        spdlog::set_level(spdlog::level::trace);
        spdlog::default_logger()->sinks().push_back(std::make_shared<logging::latest_messages_tank::Sink<std::mutex>>());
    }

    void cleanup() {
        spdlog::shutdown();
    }
}
