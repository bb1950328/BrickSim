//
// Created by bb1950328 on 19.09.20.
//

#include <fstream>
#include <iostream>
#include <mutex>
#include <utility>
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

        void setStringNoMutex(const StringKey &key, const std::string &value) {
            db::config::setString(key.name, value);
            stringsCache[key.name] = value;
        }

        void setIntNoMutex(const IntKey &key, int value) {
            db::config::setInt(key.name, value);
            intsCache[key.name] = value;
        }

        void setDoubleNoMutex(const DoubleKey &key, double value) {
            db::config::setDouble(key.name, value);
            doublesCache[key.name] = value;
        }

        void setBoolNoMutex(const BoolKey &key, bool value) {
            db::config::setBool(key.name, value);
            boolsCache[key.name] = value;
        }
    }

    std::string getString(const StringKey &key) {
        auto it = stringsCache.find(key.name);
        if (it == stringsCache.end()) {
            std::lock_guard<std::mutex> lg(stringsCacheMtx);
            auto valueOpt = db::config::getString(key.name);
            std::string value;
            if (valueOpt.has_value()) {
                value = valueOpt.value();
                stringsCache.emplace(key.name, value);
            } else {
                value = key.defaultValue;
                setStringNoMutex(key, value);
            }
            return value;
        }
        return it->second;
    }

    int getInt(const IntKey &key) {
        auto it = intsCache.find(key.name);
        if (it == intsCache.end()) {
            std::lock_guard<std::mutex> lg(intsCacheMtx);
            auto valueOpt = db::config::getInt(key.name);
            int value;
            if (valueOpt.has_value()) {
                value = valueOpt.value();
                intsCache.emplace(key.name, value);
            } else {
                value = key.defaultValue;
                setIntNoMutex(key, value);
            }
            return value;
        }
        return it->second;
    }

    double getDouble(const DoubleKey &key) {
        auto it = doublesCache.find(key.name);
        if (it == doublesCache.end()) {
            std::lock_guard<std::mutex> lg(intsCacheMtx);
            auto valueOpt = db::config::getDouble(key.name);
            double value;
            if (valueOpt.has_value()) {
                value = valueOpt.value();
                doublesCache.emplace(key.name, value);
            } else {
                value = key.defaultValue;
                setDoubleNoMutex(key, value);
            }
            return value;
        }
        return it->second;
    }

    util::RGBcolor getColor(const ColorKey &key) {
        return util::RGBcolor(getString(key));
    }

    bool getBool(const BoolKey &key) {
        auto it = boolsCache.find(key.name);
        if (it == boolsCache.end()) {
            std::lock_guard<std::mutex> lg(boolsCacheMtx);
            auto valueOpt = db::config::getBool(key.name);
            bool value;
            if (valueOpt.has_value()) {
                value = valueOpt.value();
                boolsCache.emplace(key.name, value);
            } else {
                value = key.defaultValue;
                setBoolNoMutex(key, value);
            }
            return value;
        }
        return it->second;
    }

    void setString(const StringKey &key, const std::string &value) {
        std::lock_guard<std::mutex> lg(stringsCacheMtx);
        setStringNoMutex(key, value);
    }

    void setInt(const IntKey &key, int value) {
        std::lock_guard<std::mutex> lg(intsCacheMtx);
        setIntNoMutex(key, value);
    }

    void setDouble(const DoubleKey &key, double value) {
        std::lock_guard<std::mutex> lg(doublesCacheMtx);
        setDoubleNoMutex(key, value);
    }

    void setColor(const ColorKey &key, util::RGBcolor value) {
        setString(key, value.asHtmlCode());
    }

    void setBool(const BoolKey &key, bool value) {
        std::lock_guard<std::mutex> lg(boolsCacheMtx);
        setBoolNoMutex(key, value);
    }

    bool Key::operator==(const Key &other) const {
        return other.name == name;
    }

    Key::Key(std::string name) : name(std::move(name)) {}

    StringKey::StringKey(const std::string &name, std::string defaultValue) : Key(name), defaultValue(std::move(defaultValue)) {}

    IntKey::IntKey(const std::string &name, const int defaultValue) : Key(name), defaultValue(defaultValue) {}

    DoubleKey::DoubleKey(const std::string &name, const double defaultValue) : Key(name), defaultValue(defaultValue) {}

    ColorKey::ColorKey(const std::string &name, const util::RGBcolor &defaultValue) : StringKey(name, defaultValue.asHtmlCode()), defaultValue(defaultValue) {}

    BoolKey::BoolKey(const std::string &name, const bool defaultValue) : Key(name), defaultValue(defaultValue) {}
}