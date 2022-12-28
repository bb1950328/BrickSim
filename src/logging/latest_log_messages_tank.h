#pragma once

#include "../helpers/platform_detection.h"
#include <cinttypes>
#include <list>
#include <spdlog/fmt/chrono.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

namespace bricksim::logging::latest_messages_tank {
    struct LogMessage {
        const long timestamp;
        const unsigned char level;
        std::string formattedTime;
        std::string message;

        LogMessage(long timestamp, unsigned char level, std::string formattedTime, std::string message);
    };

    namespace {
        constexpr unsigned char minLevelForAlwaysKeeping = spdlog::level::warn;
        int numLastMessages;

        std::vector<LogMessage> alwaysKeepingMessages;
        std::list<LogMessage> lastNMessages;
    }

    class iterator {
    public:
        using self_type = iterator;

        iterator();

        self_type operator++();

        [[nodiscard]] const LogMessage* getCurrent() const;

    private:
        std::vector<LogMessage>::const_iterator itA = alwaysKeepingMessages.cbegin();
        std::list<LogMessage>::const_iterator itB = lastNMessages.cbegin();
        std::vector<LogMessage>::const_iterator endA = alwaysKeepingMessages.cend();
        std::list<LogMessage>::const_iterator endB = lastNMessages.cend();
        LogMessage const* current{};
    };

    void initialize();

    void addMessage(const LogMessage& msg);

    iterator getIterator();

    void clear();

    template<typename Mutex>
    class Sink : public spdlog::sinks::base_sink<Mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            const auto timeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(msg.time).time_since_epoch().count();

            addMessage({timeMs,
                        (const unsigned char)msg.level,
                        fmt::format("{:%H:%M:%S}", msg.time),
                        msg.payload.data()});
        }

        void flush_() override {}
    };
}
