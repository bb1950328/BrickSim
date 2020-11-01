//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include "config.h"

namespace config {
    namespace {
        std::map<std::string, std::string> strings;
        std::map<std::string, long> longs;
        std::map<std::string, double> doubles;
    }

    void _ensure_settings_loaded() {
        static bool settingsLoaded = false;
        if (!settingsLoaded) {
            std::ifstream input("config.txt");
            if (!input.good()) {
                throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), "can't find config.txt file, should be in cwd!!");
            }
            for (std::string line; getline(input, line);) {
                if (!line.empty()) {
                    auto sep_pos = line.find('=');
                    auto key = line.substr(0, sep_pos);
                    auto value_str = line.substr(sep_pos+1);

                    //todo improve type detection
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
                        strings[key] = value_str;
                    } else {
                        if (has_dot) {
                            doubles[key] = std::stod(value_str);
                        } else {
                            longs[key] = std::stol(value_str);
                        }
                    }
                }
            }
            settingsLoaded = true;
        }
    }

    std::string get_string(const Key& key) {
        _ensure_settings_loaded();
        auto it = strings.find(key.name);
        if (it==strings.end()) {
            return "";
        }
        return it->second;
    }

    long get_long(const Key& key) {
        _ensure_settings_loaded();
        auto it = longs.find(key.name);
        if (it==longs.end()) {
            return 0;
        }
        return it->second;
    }

    double get_double(const Key& key) {
        _ensure_settings_loaded();
        auto it = doubles.find(key.name);
        if (it==doubles.end()) {
            return 0.0;
        }
        return it->second;
    }

    util::RGBcolor get_color(const Key &key) {
        _ensure_settings_loaded();
        return util::RGBcolor(get_string(key));
    }

    bool get_bool(const Key &key) {
        _ensure_settings_loaded();
        return get_string(key)=="true";
    }

    void set_string(const Key& key, const std::string &value) {
        _ensure_settings_loaded();
        strings[key.name] = value;
    }

    void set_long(const Key& key, long value) {
        _ensure_settings_loaded();
        longs[key.name] = value;
    }

    void set_double(const Key& key, double value) {
        _ensure_settings_loaded();
        doubles[key.name] = value;
    }

    void set_color(const Key &key, util::RGBcolor value) {
        _ensure_settings_loaded();
        set_string(key, value.asHtmlCode());
    }

    void set_bool(const Key &key, bool value) {
        _ensure_settings_loaded();
        set_string(key, value?"true":"false");
    }

    bool save() {
        _ensure_settings_loaded();
        std::ofstream file("config.txt");
        if (!file.good()) {
            return false;
        }
        for (const auto &entry : strings) {
            file << entry.first << "=" << entry.second << std::endl;
        }
        for (const auto &entry : longs) {
            file << entry.first << "=" << entry.second << std::endl;
        }
        for (const auto &entry : doubles) {
            file << entry.first << "=" << entry.second << std::endl;
        }
        return true;
    }
}