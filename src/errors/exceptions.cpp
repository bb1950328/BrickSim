#include "exceptions.h"
#include "spdlog/fmt/fmt.h"

namespace bricksim::errors {
    BaseException::BaseException(const std::string& message, const std::source_location& location) :
        std::runtime_error(message), location(location) {
    }
    const std::source_location& BaseException::getLocation() const {
        return location;
    }
    TaskFailedException::TaskFailedException(const std::string& message, const std::source_location location) :
        BaseException(message, location) {}
    CriticalException::CriticalException(const std::string& message, const std::source_location location) :
        BaseException(message, location) {}
}
