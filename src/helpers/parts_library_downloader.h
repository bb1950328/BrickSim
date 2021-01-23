// part_library_downloader.h
// Created by bab21 on 23.01.21.
//

#ifndef BRICKSIM_PARTS_LIBRARY_DOWNLOADER_H
#define BRICKSIM_PARTS_LIBRARY_DOWNLOADER_H

#include <utility>

namespace parts_library_downloader {
    enum Status {
        DOING_NOTHING,
        IN_PROGRESS,
        FINISHED
    };
    namespace {
        long downTotal = 0;
        long downNow = 0;
        Status status = DOING_NOTHING;
        int progressFunc(void *clientp, long downloadTotal, long downloadNow, long uploadTotal, long uploadNow);
        bool shouldStop = false;
    }
    void downloadPartsLibrary();
    void reset();
    Status getStatus();
    std::pair<long, long> getProgress();
    void stopDownload();
}
#endif //BRICKSIM_PARTS_LIBRARY_DOWNLOADER_H
