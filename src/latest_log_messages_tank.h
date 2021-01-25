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
        std::shared_ptr<const char *> formattedTime;
        std::shared_ptr<const char *> message;

        LogMessage(long timestamp, unsigned char level, std::shared_ptr<const char *> formattedTime,
                   std::shared_ptr<const char *> message);
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

    template<typename Mutex>
    class Sink : public spdlog::sinks::base_sink<Mutex> {
    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override {
            constexpr auto timeBufSize = sizeof("12:34:56.789");
            char* time = new char[timeBufSize];
            const time_t timestamp = std::chrono::system_clock::to_time_t(msg.time);
            const auto time_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(msg.time);
            std::strftime(time, timeBufSize, "%H:%M:%S", std::localtime(&timestamp));
            snprintf(&time[8], 5, ".%03d", time_ms.time_since_epoch().count()%1000);
            char* message = new char[msg.payload.size()+1];
            std::memcpy(message, msg.payload.data(), msg.payload.size()+1);
            addMessage(LogMessage(
                    static_cast<long>(timestamp),
                    (const unsigned char) msg.level,
                    std::make_shared<const char *>(time),
                    std::make_shared<const char *>(message)));
        }

        void flush_() override {}
    };
}

#endif //BRICKSIM_LATEST_LOG_MESSAGES_TANK_H
