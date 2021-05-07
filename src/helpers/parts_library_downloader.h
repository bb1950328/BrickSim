#ifndef BRICKSIM_PARTS_LIBRARY_DOWNLOADER_H
#define BRICKSIM_PARTS_LIBRARY_DOWNLOADER_H

#include <utility>

namespace parts_library_downloader {
    enum Status {
        DOING_NOTHING,
        IN_PROGRESS,
        FAILED,
        FINISHED
    };
    namespace {
        long downTotal = 0;
        long downNow = 0;
        int bytesPerSecond = 0;
        Status status = DOING_NOTHING;
        int progressFunc(void *clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow);
        bool shouldStop = false;
        long lastProgressCalledMs;
        int errorCode = -1;
    }
    void downloadPartsLibrary();
    void reset();
    Status getStatus();
    std::pair<long, long> getProgress();
    int getSpeedBytesPerSecond();
    void stopDownload();
    int getErrorCode();
}
#endif //BRICKSIM_PARTS_LIBRARY_DOWNLOADER_H
