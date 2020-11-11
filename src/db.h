//
// Created by Bader on 10.11.2020.
//

#ifndef BRICKSIM_DB_H
#define BRICKSIM_DB_H

#include "info_providers/price_guide_provider.h"

namespace db {
    void initialize();

    void cleanup();

    namespace requestCache {
        std::optional<std::string> get(const std::string& url);
        void put(const std::string& url, const std::string& response);
    }

    namespace priceGuideCache {
        std::optional<price_guide_provider::PriceGuide> get(const std::string& partCode, const std::string& currencyCode, const std::string& colorName);
        void put(const std::string& partCode, const std::string& currencyCode, const std::string& colorName, const price_guide_provider::PriceGuide &value);
    }

    namespace config {
        std::string getString(const std::string& key);
        int getInt(const std::string& key);
        bool getBool(const std::string& key);
        double getDouble(const std::string& key);

        void setString(const std::string& key, const std::string& value);
        void setInt(const std::string& key, int value);
        void setBool(const std::string& key, bool value);
        void setDouble(const std::string& key, double value);
    }
}

#endif //BRICKSIM_DB_H