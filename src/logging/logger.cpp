#include <memory>
#include "logger.h"
#include "latest_log_messages_tank.h"

namespace bricksim::logging {

    void initialize() {
        spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e | %-8!l | T%t | %v");
        spdlog::set_level(spdlog::level::trace);
        const std::shared_ptr<logging::latest_messages_tank::Sink<std::mutex>> x(new logging::latest_messages_tank::Sink<std::mutex>);
        spdlog::default_logger()->sinks().push_back(x);
    }

    void cleanup() {
        spdlog::shutdown();
    }
}
