//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include "config.h"

Configuration* Configuration::instance = nullptr;

Configuration *Configuration::getInstance() {
    if (nullptr==instance) {
        instance = new Configuration();
    }
    return instance;
}

Configuration::Configuration() {
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
}

std::string Configuration::get_string(const std::string& key) const {
    auto it = strings.find(key);
    if (it==strings.end()) {
        return "";
    }
    return it->second;
}

long Configuration::get_long(const std::string& key) const {
    auto it = longs.find(key);
    if (it==longs.end()) {
        return 0;
    }
    return it->second;
}

double Configuration::get_double(const std::string& key) const {
    auto it = doubles.find(key);
    if (it==doubles.end()) {
        return 0.0;
    }
    return it->second;
}

void Configuration::set_string(const std::string &key, const std::string &value) {
    strings[key] = value;
}

void Configuration::set_long(const std::string &key, long value) {
    longs[key] = value;
}

void Configuration::set_double(const std::string &key, double value) {
    doubles[key] = value;
}

bool Configuration::save() {
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

