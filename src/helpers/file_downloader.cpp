//
// Created by bb1950328 on 18.10.2020.
//

#include "file_downloader.h"
#include <cstdio>
#include <curl/curl.h>

namespace file_downloader {
    namespace {
        size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
            size_t written = fwrite(ptr, size, nmemb, stream);
            return written;
        }
    }

    void *downloadFile(std::string url, std::pair<const std::filesystem::path &, std::pair<float, long long int> *> args) {
        auto destination = args.first;
        auto progressPercentBytes = args.second;
        CURL *curl;
        FILE *fp;
        CURLcode res;
        char *urlChar = url.data();
        char outfilename[FILENAME_MAX];
        strcpy(outfilename, destination.string().c_str());
        curl = curl_easy_init();

        auto xfer_callback = [progressPercentBytes](void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
            progressPercentBytes->first = 100.0 * dlnow / dltotal;
            progressPercentBytes->second = dlnow;
        };

        fp = fopen(outfilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, urlChar);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xfer_callback);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
        return nullptr;
    }

}