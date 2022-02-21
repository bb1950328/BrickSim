#include "parts_library_downloader.h"
#include "../config.h"
#include "util.h"
#include <chrono>
#include <fstream>
#include <spdlog/spdlog.h>
#include "../constant_data/constants.h"

namespace bricksim::parts_library_downloader {
    namespace {
        int progressFunc(void* clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow) {
            const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            const auto msDiff = now - lastProgressCalledMs;
            const auto bytesDiff = downloadNow - downNow;
            bytesPerSecond = (int)(0.001 * bytesDiff / msDiff);
            downNow = downloadNow;
            downTotal = downloadTotal;
            return shouldStop ? 1 : 0;
        }
    }

    void downloadPartsLibrary() {
        status = IN_PROGRESS;
        spdlog::info("starting parts library download");
        auto filePath = util::extendHomeDir("~/ldraw.zip");
        auto result = util::downloadFile(constants::LDRAW_LIBRARY_DOWNLOAD_URL, filePath, progressFunc);
        if (result.first < 200 || result.first >= 300) {
            spdlog::error("parts library download failed. Error code: {}", result.first);
            errorCode = result.first;
            status = FAILED;
            return;
        }
        spdlog::info("parts library download finished");
        
        config::set(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(filePath));
        status = FINISHED;
    }

    void reset() {
        status = DOING_NOTHING;
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
