#include "latest_log_messages_tank.h"

#include "../config/read.h"
#include <utility>

namespace bricksim::logging::latest_messages_tank {
    void addMessage(const latest_messages_tank::LogMessage& msg) {
        if (msg.level >= minLevelForAlwaysKeeping) {
            alwaysKeepingMessages.push_back(msg);
        } else {
            while (lastNMessages.size() > numLastMessages - 1) {
                lastNMessages.pop_front();
            }
            lastNMessages.push_back(msg);
        }
    }

    void initialize() {
        numLastMessages = config::get().log.notImportantLogMessageKeepCount;
    }

    iterator getIterator() {
        return {};
    }

    void clear() {
        lastNMessages.clear();
        alwaysKeepingMessages.clear();
    }

    iterator::iterator() {
        this->operator++();
    }

    iterator::self_type iterator::operator++() {
        self_type i = *this;
        if (itA != endA && (itB == endB || itA->timestamp < itB->timestamp)) {
            current = &(*itA);
            itA++;
        } else if (itB != endB) {
            current = &(*itB);
            itB++;
        } else {
            current = nullptr;
        }
        return i;
    }

    const LogMessage* iterator::getCurrent() const {
        return current;
    }

    LogMessage::LogMessage(const long timestamp, const unsigned char level, std::string formattedTime, std::string message) :
        timestamp(timestamp), level(level), formattedTime(std::move(formattedTime)), message(std::move(message)) {}
}
