#pragma once

#include "info_providers/price_guide_provider.h"
#include "types.h"
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace bricksim::db {
    void initialize();

    void cleanup();

    namespace requestCache {
        std::optional<std::string> get(const std::string& url);
        void put(const std::string& url, const std::string& response);
    }

    namespace priceGuideCache {
        std::optional<info_providers::price_guide::PriceGuide> get(const std::string& partCode, const std::string& currencyCode, const std::string& colorName);
        void put(const std::string& partCode, const std::string& currencyCode, const std::string& colorName, const info_providers::price_guide::PriceGuide& value);
    }

    namespace fileList {
        struct Entry {
            std::string name;
            std::string title;
            std::string category;
        };
        int getSize();
        void put(const std::string& name, const std::string& title, const std::string& category);
        void put(const std::vector<Entry>& entries);
        oset_t<std::string> getAllCategories();
        uoset_t<std::string> getAllFiles();
        oset_t<std::string> getAllPartsForCategory(const std::string& category);
        std::optional<std::string> containsFile(const std::string& name);
        std::optional<Entry> findFile(const std::string& name);
    }

    namespace config {
        std::optional<std::string> getString(const char* key);
        std::optional<int> getInt(const char* key);
        std::optional<double> getDouble(const char* key);

        void setString(const char* key, const std::string& value);
        void setInt(const char* key, int value);
        void setDouble(const char* key, double value);

        void deleteAll();
    }

    namespace key_shortcuts {
        typedef std::tuple<int, int, uint8_t, uint8_t> record_t;
        std::vector<record_t> loadShortcuts();
        void saveShortcut(record_t record);
        void deleteAll();
    }
}
