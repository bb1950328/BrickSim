#include "parts_library_downloader.h"
#include "../config.h"
#include "util.h"
#include <chrono>
#include <fstream>
#include <spdlog/spdlog.h>
#include "../constant_data/constants.h"

namespace bricksim::parts_library_downloader {
    namespace {
        int progressFunc([[maybe_unused]] void* clientp, long downloadTotal, long downloadNow, [[maybe_unused]] long uploadTotal, [[maybe_unused]] long uploadNow) {
            const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            const auto msDiff = now - lastSpeedCalcTimestampMs;
            if (msDiff > 1000) {
                const auto bytesDiff = downloadNow - lastSpeedCalcBytes;
                lastSpeedCalcTimestampMs = now;
                lastSpeedCalcBytes = downloadNow;
                bytesPerSecond = static_cast<int>((static_cast<double>(bytesDiff) / (.001 * static_cast<double>(msDiff))));
                spdlog::debug("{} {} {}", bytesDiff, msDiff, bytesPerSecond);
            }
            downNow = downloadNow;
            downTotal = downloadTotal;
            return shouldStop ? 1 : 0;
        }
    }

    void downloadPartsLibrary() {
        status = Status::IN_PROGRESS;
        spdlog::info("starting parts library download");
        auto filePath = util::extendHomeDir("~/ldraw.zip");
        auto [statusCode, content] = util::downloadFile(constants::LDRAW_LIBRARY_DOWNLOAD_URL, filePath, progressFunc);
        if (statusCode < 200 || statusCode >= 300) {
            spdlog::error("parts library download failed. Error code: {}", statusCode);
            errorCode = statusCode;
            status = Status::FAILED;
            return;
        }
        spdlog::info("parts library download finished");

        config::set(config::LDRAW_PARTS_LIBRARY, util::replaceSpecialPaths(filePath).string());
        status = Status::FINISHED;
    }

    void reset() {
        status = Status::DOING_NOTHING;
        downNow = 0;
        downTotal = 0;
        bytesPerSecond = 0;
        shouldStop = true;
        errorCode = -1;
    }

    Status getStatus() {
        return status;
    }

    std::pair<long, long> getProgress() {
        return {downNow, downTotal};
    }

    int getSpeedBytesPerSecond() {
        return bytesPerSecond;
    }

    void stopDownload() {
        shouldStop = true;
    }

    int getErrorCode() {
        return errorCode;
    }
}
