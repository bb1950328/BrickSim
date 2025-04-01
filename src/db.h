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
        void deleteAllEntries();
    }

    namespace valueCache {
        constexpr auto LAST_INDEX_LDCONFIG_HASH = "LAST_INDEX_LDCONFIG_HASH";
        constexpr auto CURRENT_PARTS_LIBRARY_VERSION = "CURRENT_PARTS_LIBRARY_VERSION";
        template<typename T>
        std::optional<T> get(const char* key);

        template<>
        std::optional<std::string> get(const char* key);

        template<>
        std::optional<long long> get(const char* key);

        template<>
        std::optional<double> get(const char* key);

        template<typename T>
        void set(const char* key, T value) {
            set<std::string>(key, std::to_string(value));
        }

        template<>
        void set(const char* key, std::string value);
    }

    namespace state {
        std::optional<std::string> get(int64_t key);
        void set(int64_t key, const std::string& value);
        void clear(int64_t key);
    }
}
