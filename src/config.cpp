//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include <iostream>
#include <mutex>
#include "config.h"
#include "db.h"

namespace config {
    namespace {
        std::map<std::string, std::string> stringsCache;
        std::map<std::string, long> intsCache;
        std::map<std::string, bool> boolsCache;
        std::map<std::string, double> doublesCache;

        std::mutex stringsCacheMtx;
        std::mutex intsCacheMtx;
        std::mutex boolsCacheMtx;
        std::mutex doublesCacheMtx;
    }

    std::string getString(const Key& key) {
        auto it = stringsCache.find(key.name);
        if (it==stringsCache.end()) {
            std::lock_guard<std::mutex> lg(stringsCacheMtx);
            auto value = db::config::getString(key.name);
            stringsCache.emplace(key.name, value);
            return value;
        }
        return it->second;
    }

    long getLong(const Key& key) {
        auto it = intsCache.find(key.name);
        if (it==intsCache.end()) {
            std::lock_guard<std::mutex> lg(intsCacheMtx);
            auto value = db::config::getInt(key.name);
            intsCache.emplace(key.name, value);
            return value;
        }
        return it->second;
    }

    double getDouble(const Key& key) {
        auto it = doublesCache.find(key.name);
        if (it==doublesCache.end()) {
            std::lock_guard<std::mutex> lg(intsCacheMtx);
            auto value = db::config::getDouble(key.name);
            doublesCache.emplace(key.name, value);
            return value;
        }
        return it->second;
    }

    util::RGBcolor getColor(const Key &key) {
        return util::RGBcolor(getString(key));
    }

    bool getBool(const Key &key) {
        auto it = boolsCache.find(key.name);
        if (it==boolsCache.end()) {
            std::lock_guard<std::mutex> lg(boolsCacheMtx);
            auto value = db::config::getBool(key.name);
            boolsCache.emplace(key.name, value);
            return value;
        }
        return it->second;
    }

    void setString(const Key& key, const std::string &value) {
        db::config::setString(key.name, value);
        {
            std::lock_guard<std::mutex> lg(stringsCacheMtx);
            stringsCache.emplace(key.name, value);
        }
    }

    void setLong(const Key& key, long value) {
        db::config::setInt(key.name, value);
        {
            std::lock_guard<std::mutex> lg(intsCacheMtx);
            intsCache.emplace(key.name, value);
        }
    }

    void setDouble(const Key& key, double value) {
        db::config::setDouble(key.name, value);
        {
            std::lock_guard<std::mutex> lg(doublesCacheMtx);
            doublesCache.emplace(key.name, value);
        }
    }

    void setColor(const Key &key, util::RGBcolor value) {
        setString(key, value.asHtmlCode());
    }

    void setBool(const Key &key, bool value) {
        db::config::setBool(key.name, value);
        {
            std::lock_guard<std::mutex> lg(boolsCacheMtx);
            boolsCache.emplace(key.name, value);
        }
    }

    void exportToTxt() {
        std::ofstream file("config.txt");
        if (!file.good()) {
            throw std::system_error(std::make_error_code(std::errc::io_error), "error while opening config.txt, do you have sufficient permissions?");
        }
        //todo implement methods to get all values
        /*for (const auto &entry : strings) {
            file << entry.first << "=" << entry.second << std::endl;
        }
        for (const auto &entry : longs) {
            file << entry.first << "=" << entry.second << std::endl;
        }
        for (const auto &entry : doubles) {
            file << entry.first << "=" << entry.second << std::endl;
        }*/
    }

    void importFromTxt() {
        std::ifstream input("config.txt");
        if (!input.good()) {
            throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), "can't find config.txt file, should be in cwd!!");
        }
        for (std::string line; getline(input, line);) {
            if (!line.empty()) {
                auto sep_pos = line.find('=');
                auto key = line.substr(0, sep_pos);
                auto value_str = line.substr(sep_pos+1);

                auto has_dot = false;
                auto has_nondigit = false;
                for (char i : value_str) {
                    if (i=='.') {
                        has_dot = true;
                    } else if (!std::isdigit(i) && i!='-') {
                        has_nondigit = true;
                    }
                }
                if (has_nondigit) {
                    if (value_str=="true" || value_str=="false") {
                        setBool(Key(key), value_str == "true");
                    } else {
                        setString(Key(key), value_str);
                    }
                } else {
                    if (has_dot) {
                        setDouble(Key(key), std::stod(value_str));
                    } else {
                        setLong(Key(key), std::stol(value_str));
                    }
                }
            }
        }
    }

    bool Key::operator==(const Key& other) const {
        return other.name==name;
    }

    Key::Key(std::string name) : name(std::move(name)) {}
}