#pragma once
#include <stdexcept>
#include <source_location>

namespace bricksim::errors {
    class BaseException : public std::runtime_error {
        const std::source_location location;
    public:
        BaseException(const std::string& message, const std::source_location& location);
        const std::source_location& getLocation() const;
    };
    class TaskFailedException : public BaseException {
    public:
        TaskFailedException(const std::string& message, const std::source_location location = std::source_location::current());
    };
    class CriticalException : public BaseException {
    public:
        CriticalException(const std::string& message, const std::source_location location = std::source_location::current());
    };
}