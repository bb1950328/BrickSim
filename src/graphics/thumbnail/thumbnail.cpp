#include "thumbnail.h"

namespace bricksim::graphics {
    bool ThumbnailRequest::operator==(const ThumbnailRequest &rhs) const {
        return ldrFile == rhs.ldrFile && color == rhs.color && backgroundColor == rhs.backgroundColor;
    }

    bool ThumbnailRequest::operator!=(const ThumbnailRequest &rhs) const {
        return !(rhs == *this);
    }

    bool ThumbnailRequest::operator<(const ThumbnailRequest &rhs) const {
        if (ldrFile < rhs.ldrFile)
            return true;
        if (rhs.ldrFile < ldrFile)
            return false;
        if (color < rhs.color)
            return true;
        if (rhs.color < color)
            return false;
        return backgroundColor < rhs.backgroundColor;
    }

    bool ThumbnailRequest::operator>(const ThumbnailRequest &rhs) const {
        return rhs < *this;
    }

    bool ThumbnailRequest::operator<=(const ThumbnailRequest &rhs) const {
        return !(rhs < *this);
    }

    bool ThumbnailRequest::operator>=(const ThumbnailRequest &rhs) const {
        return !(*this < rhs);
    }

    std::string ThumbnailRequest::getFilename() {
        return util::escapeFilename(fmt::format("{}_{}_{}.png",
                                                ldrFile->metaInfo.name,
                                                color.code,
                                                backgroundColor.has_value()
                                                ? backgroundColor->asHtmlCode()
                                                : ""));
    }
}
