//
// Created by bb1950328 on 18.10.2020.
//

#ifndef BRICKSIM_FILE_DOWNLOADER_H
#define BRICKSIM_FILE_DOWNLOADER_H

#include <string>
#include <filesystem>

namespace file_downloader {
    void *download_file(std::string url, const std::filesystem::path &destination, std::pair<float, long long int> *progressPercentBytes);
}

#endif //BRICKSIM_FILE_DOWNLOADER_H
