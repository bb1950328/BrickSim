#include "latest_log_messages_tank.h"
#include "../config.h"

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
        numLastMessages = config::get(config::NOT_IMPORTANT_LOG_MESSAGE_KEEP_COUNT);
    }

    iterator getIterator() {
        return iterator();
    }

    void clear() {
        lastNMessages.clear();
        alwaysKeepingMessages.clear();
    }

    iterator::iterator() :
        itA(alwaysKeepingMessages.cbegin()), itB(lastNMessages.cbegin()), endA(alwaysKeepingMessages.cend()), endB(lastNMessages.cend()) {
        this->operator++();
    }

    iterator::self_type iterator::operator++() {
        self_type i = *this;
        if (itA != endA && (itB == endB || itA->timestamp < itB->timestamp)) {
            current = itA.base();
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

    LogMessage::LogMessage(const long timestamp, const unsigned char level, const std::string& formattedTime, const std::string& message) :
        timestamp(timestamp), level(level), formattedTime(formattedTime), message(message) {}
}