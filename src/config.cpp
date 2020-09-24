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
    return strings.find(key)->second;
}

long Configuration::get_long(const std::string& key) const {
    return longs.find(key)->second;
}

double Configuration::get_double(const std::string& key) const {
    return doubles.find(key)->second;
}

