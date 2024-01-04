#include "read.h"
#include "write.h"

namespace bricksim::config {
    const Config& get() {
        return getMutable();
    }
}
