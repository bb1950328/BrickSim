// part_library_downloader.cpp.c
// Created by bab21 on 23.01.21.
//

#include <fstream>
#include <spdlog/spdlog.h>
#include "parts_library_downloader.h"
#include "util.h"
#include "platform_detection.h"
#include "../config.h"

namespace parts_library_downloader {
    namespace {
        int progressFunc(void *clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow) {
            downNow = downloadNow;
            downTotal = downloadTotal;
            return shouldStop?1:0;
        }
    }

    void downloadPartsLibrary() {
        status = IN_PROGRESS;
        spdlog::info("starting parts library download");
        auto result = util::requestGET("https://www.ldraw.org/library/updates/complete.zip", false, 0, progressFunc);
        if (result.first < 200 || result.first >= 300) {
            spdlog::error("parts library download failed. Error code: {}", result.first);
            return;
        }
        spdlog::info("parts library download finished, start writing to disk");
        auto filePath = util::extendHomeDir("~/ldraw.zip");
        std::ofstream out(filePath);
        out << result.second;
        out.close();
        config::setString(config::LDRAW_PARTS_LIBRARY, filePath);
        status = FINISHED;
        spdlog::info("parts library written to disk");
    }

    void reset() {
        status = DOING_NOTHING;
        downNow = 0;
        downTotal = 0;
        shouldStop = true;
    }

    Status getStatus() {
        return status;
    }

    std::pair<long, long> getProgress() {
        return {downNow, downTotal};
    }

    void stopDownload() {
        shouldStop = true;
    }
}

