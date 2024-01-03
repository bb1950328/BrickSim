#include "json_helper.h"

namespace bricksim::json_helper {
    const char* JsonValidationError::what() const noexcept {
        return message.c_str();
    }
}
