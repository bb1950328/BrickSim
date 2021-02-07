//
// Created by bb1950328 on 20.01.2021.
//

#ifndef BRICKSIM_LATEST_LOG_MESSAGES_TANK_H
#define BRICKSIM_LATEST_LOG_MESSAGES_TANK_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <queue>
#include <list>
#include <ctime>

namespace latest_log_messages_tank {
    struct LogMessage {
        const long timestamp;
        const unsigned char level;
        std::string formattedTime;
        std::string message;
        LogMessage(const long timestamp, const unsigned char level, const std::string &formattedTime, const std::string &message);
    };

    namespace {
        constexpr unsigned char minLevelForAlwaysKeeping = spdlog::level::warn;
        int numLastMessages;

        std::vector<LogMessage> alwaysKeepingMessages;
        std::list<LogMessage> lastNMessages;
    }

    class iterator {
    public:
        typedef iterator self_type;

        iterator();

        self_type operator++();

        [[nodiscard]] const LogMessage *getCurrent() const;

    private:
        std::vector<LogMessage>::const_iterator itA;
        std::list<LogMessage>::const_iterator itB;
        std::vector<LogMessage>::const_iterator endA;
        std::list<LogMessage>::const_iterator endB;
        LogMessage const *current{};
    };

    void initialize();

    void addMessage(const LogMessage &msg);

    iterator getIterator();

    void clear();

    template<typename Mutex>
    class Sink : public spdlog::sinks::base_sink<Mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override {
            constexpr auto timeBufSize = sizeof("12:34:56.789");
            static char timeBuf[timeBufSize];
            const time_t timestamp = std::chrono::system_clock::to_time_t(msg.time);
            std::strftime(timeBuf, timeBufSize, "%H:%M:%S", std::localtime(&timestamp));
            const auto timeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(msg.time).time_since_epoch().count();
            snprintf(&timeBuf[8], 5, ".%03ld", timeMs % 1000);
            std::string message = msg.payload.data();
            //todo std::strcpy(message.get(), msg.payload.data());
            addMessage(LogMessage(
                    static_cast<long>(timeMs),
                    (const unsigned char) msg.level,
                    timeBuf,
                    message));
        }

        void flush_() override {}
    };
}

#endif //BRICKSIM_LATEST_LOG_MESSAGES_TANK_H