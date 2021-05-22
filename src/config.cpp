#include <mutex>
#include "db.h"
#include "config.h"

namespace config {

    template<>
    [[nodiscard]] std::string get<std::string>(const Key<std::string> &key) {
        return stringValues.get(key.name, key.defaultValue);
    }

    template<>
    [[nodiscard]] int get<int>(const Key<int> &key) {
        return intValues.get(key.name, key.defaultValue);
    }

    template<>
    [[nodiscard]] double get<double>(const Key<double> &key) {
        return doubleValues.get(key.name, key.defaultValue);
    }

    template<>
    [[nodiscard]] color::RGB get<color::RGB>(const Key<color::RGB> &key) {
        return color::RGB(stringValues.get(key.name, key.defaultValue.asHtmlCode()));
    }


    template<>
    [[nodiscard]] float get<float>(const Key<float> &key) {
        return doubleValues.get(key.name, key.defaultValue);
    }
    template<>
    [[nodiscard]] bool get<bool>(const Key<bool> &key) {
        return intValues.get(key.name, key.defaultValue);
    }

    template<>
    void set(const Key<color::RGB> &key, const color::RGB &value) {
        stringValues.set(key.name, value.asHtmlCode());
    }

    void resetAllToDefault() {
        stringValues.clear();
        intValues.clear();
        doubleValues.clear();
        db::config::deleteAll();
    }
}