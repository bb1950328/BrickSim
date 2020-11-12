//
// Created by Bader on 02.11.2020.
//

#include "zip_buffer.h"
#include <zip.h>
#include <cstring>
#include <iostream>
#include "util.h"

namespace zip_buffer {

    namespace {
        std::map<std::filesystem::path, BufferedZip> zips;
    }

    BufferedZip *openZipFile(const std::filesystem::path &path, const std::optional<std::string> &password) {
        zips.emplace(path, BufferedZip(path, password));
        return &(zips.find(path)->second);
    }

    void closeZipFile(const std::filesystem::path& path) {
        auto it = zips.find(path);
        if (it != zips.end()) {
            zips.erase(it);
        }
    }

    BufferedZip::BufferedZip(const std::filesystem::path& path, const std::optional<std::string>& password={}) {
        auto zipNameStem = path.stem().string();
        int errCode;
        struct zip *zArchive = zip_open(path.string().c_str(), 0, &errCode);
        if (zArchive == nullptr) {
            char errMessageBuffer[100];
            zip_error_to_str(errMessageBuffer, sizeof(errMessageBuffer), errCode, errno);//todo this function is deprecated
            throw std::invalid_argument(std::string("can't open zip file! Error ") + std::to_string(errCode) + ": " + std::string(errMessageBuffer));
        }

        if (password.has_value() && zip_set_default_password(zArchive, password->data()) != 0) {
            throw std::invalid_argument("Error setting zip file password ");
        }

        for (zip_int64_t i = 0; i < zip_get_num_entries(zArchive, 0); i++) {
            struct zip_stat sb{};
            if (zip_stat_index(zArchive, i, 0, &sb) == 0) {
                if (sb.name[strlen(sb.name) - 1] == '/') {
                    //safe_create_dir(sb.name);
                } else {
                    struct zip_file *zFile = zip_fopen_index(zArchive, i, 0);
                    if (!zFile) {
                        throw std::invalid_argument(std::string("can't open ") + sb.name + " in " + path.string());
                    }
                    std::string fileNameToSave(sb.name);
                    if (util::startsWith(fileNameToSave, zipNameStem)) {
                        fileNameToSave.erase(0, zipNameStem.size()+1);//plus 1 is for slash
                    }
                    if (util::endsWith(fileNameToSave, ".txt") || util::endsWith(fileNameToSave, ".dat") || util::endsWith(fileNameToSave, ".ldr") ||
                        util::endsWith(fileNameToSave, ".mpd")) {
                        std::string content;
                        content.resize(sb.size);
                        zip_fread(zFile, content.data(), sb.size);
                        textFiles.emplace(fileNameToSave, content);
                    } else {
                        std::vector<char> content;
                        content.resize(sb.size);
                        zip_fread(zFile, content.data(), sb.size);
                        binaryFiles.emplace(fileNameToSave, content);
                    }
                    zip_fclose(zFile);
                    //std::cout << "========================================================" << std::endl << fileNameToSave << std::endl << content << std::endl;
                }
            }
        }

        if (zip_close(zArchive) == -1) {
            throw std::invalid_argument("%s: can't close zip archive `%s'/n");
        }

        std::cout << "read " << textFiles.size() << " files in " << path << std::endl;
    }

    std::stringstream BufferedZip::getFileAsStream(const std::string& filename) {
        auto it = textFiles.find(filename);
        if (it != textFiles.end()) {
            std::string buffer(it->second.begin(), it->second.end());
            std::stringstream stream;
            stream << buffer;
            return stream;
        } else {
            throw std::invalid_argument(std::string("no such file in this zip! ")+filename);
        }
    }

    const std::string* BufferedZip::getFileAsString(const std::string& filename) {
        auto it = textFiles.find(filename);
        if (it != textFiles.end()) {
            return &it->second;
        } else {
            throw std::invalid_argument(std::string("no such file in this zip! ")+filename);
        }
    }
}
