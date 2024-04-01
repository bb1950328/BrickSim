#pragma once

#include <optional>
#include "../../ldr/files.h"

namespace bricksim::graphics {
    struct ThumbnailRequest {
        std::shared_ptr<ldr::File> ldrFile;
        ldr::ColorReference color;
        std::optional<color::RGB> backgroundColor;

        bool operator==(const ThumbnailRequest &rhs) const;

        bool operator!=(const ThumbnailRequest &rhs) const;

        bool operator<(const ThumbnailRequest &rhs) const;

        bool operator>(const ThumbnailRequest &rhs) const;

        bool operator<=(const ThumbnailRequest &rhs) const;

        bool operator>=(const ThumbnailRequest &rhs) const;

        [[nodiscard]] std::string getFilename();
    };

    using thumbnail_file_key_t = ThumbnailRequest;
}

namespace std {
    template<>
    struct hash<bricksim::graphics::thumbnail_file_key_t> {
        std::size_t operator()(const bricksim::graphics::thumbnail_file_key_t &value) const {
            return bricksim::util::combinedHash(value.ldrFile, value.color,
                                                value.backgroundColor.value_or(bricksim::color::BLACK));
        }
    };
}
