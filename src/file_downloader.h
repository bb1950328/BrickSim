//
// Created by Bader on 18.10.2020.
//

#ifndef BRICKSIM_FILE_DOWNLOADER_H
#define BRICKSIM_FILE_DOWNLOADER_H

#include <string>
#include <filesystem>

void *download_file(std::string url, const std::filesystem::path& destination, std::pair<float, long long int>* progressPercentBytes);

#endif //BRICKSIM_FILE_DOWNLOADER_H
