//
// Created by Bader on 10.11.2020.
//

#include <SQLiteCpp/Database.h>
#include <vector>
#include <iostream>
#include "db.h"

const std::vector<std::string> CONFIG_TABLE_CREATION_SCRIPTS = {
        "CREATE TABLE IF NOT EXISTS strings ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL);"
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_strings ON strings (key);",

        "CREATE TABLE IF NOT EXISTS ints ("
        "key TEXT PRIMARY KEY,"
        "value INTEGER NOT NULL);"
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_ints ON ints (key)",

        "CREATE TABLE IF NOT EXISTS doubles ("
        "key TEXT PRIMARY KEY,"
        "value REAL NOT NULL);"
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_doubles ON doubles (key)",
};

namespace db {
    namespace {
        std::optional<SQLite::Database> configDb;
        std::optional<SQLite::Database> cacheDb;
    }

    void initialize() {
        configDb = SQLite::Database("config.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
        cacheDb = SQLite::Database("cache.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

        for (const auto &script : CONFIG_TABLE_CREATION_SCRIPTS) {
            configDb.value().exec(script);
        }

        cacheDb.value().exec("CREATE TABLE IF NOT EXISTS files ("
                                "name TEXT PRIMARY KEY COLLATE NOCASE,"
                                "title TEXT,"
                                "category TEXT NOT NULL);"
                                "CREATE UNIQUE INDEX IF NOT EXISTS idx_files ON files (name);"
                                ""
                                "CREATE TABLE IF NOT EXISTS requestCache ("
                                "url TEXT PRIMARY KEY COLLATE NOCASE,"
                                "response TEXT"
                                ");"
                                "CREATE UNIQUE INDEX IF NOT EXISTS idx_requestCache ON requestCache (url);"
                                ""
                                "CREATE TABLE IF NOT EXISTS priceGuideCache ("
                                "partCode TEXT COLLATE NOCASE,"
                                "currencyCode TEXT NOT NULL,"
                                "colorName TEXT NOT NULL,"
                                "totalLots INTEGER NOT NULL,"
                                "totalQty INTEGER NOT NULL,"
                                "minPrice REAL NOT NULL,"
                                "avgPrice REAL NOT NULL,"
                                "qtyAvgPrice REAL NOT NULL,"
                                "maxPrice REAL NOT NULL"
                                ");"
                                "CREATE UNIQUE INDEX IF NOT EXISTS idx_priceGuideCache ON priceGuideCache(partCode, currencyCode, colorName);");
    }

    namespace requestCache {
        std::optional<std::string> get(const std::string &url) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT response FROM requestCache WHERE url=?;");
            stmt.bind(1, url);
            if (stmt.executeStep()) {
                return std::make_optional<std::string>(stmt.getColumn(0));
            }
            return {};
        }

        void put(const std::string& url, const std::string& response) {
            SQLite::Statement stmt(cacheDb.value(), "INSERT INTO requestCache (url, response) VALUES (?, ?);");
            stmt.bind(1, url);
            stmt.bind(2, response);
            stmt.exec();
        }
    }

    namespace priceGuideCache {
        std::optional<price_guide_provider::PriceGuide> get(const std::string& partCode, const std::string& currencyCode, const std::string& colorName) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT * FROM priceGuideCache WHERE partCode=? AND currencyCode=? AND colorName=?");
            stmt.bind(1, partCode);
            stmt.bind(2, currencyCode);
            stmt.bind(3, colorName);
            if (stmt.executeStep()) {
                return price_guide_provider::PriceGuide{
                    stmt.getColumn("currencyCode"),
                    stmt.getColumn("totalLots"),
                    stmt.getColumn("totalQty"),
                    static_cast<float>(stmt.getColumn("minPrice").getDouble()),
                    static_cast<float>(stmt.getColumn("avgPrice").getDouble()),
                    static_cast<float>(stmt.getColumn("qtyAvgPrice").getDouble()),
                    static_cast<float>(stmt.getColumn("maxPrice").getDouble())
                };
            }
            return {};
        }
        void put(const std::string& partCode, const std::string& currencyCode, const std::string& colorName, const price_guide_provider::PriceGuide &value) {
            SQLite::Statement stmt(cacheDb.value(), "REPLACE INTO priceGuideCache (partCode,currencyCode,colorName,totalLots,totalQty,minPrice,avgPrice,qtyAvgPrice,maxPrice) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);");
            stmt.bind(1, partCode);
            stmt.bind(2, currencyCode);
            stmt.bind(3, colorName);
            stmt.bind(4, value.totalLots);
            stmt.bind(5, value.totalQty);
            stmt.bind(6, value.minPrice);
            stmt.bind(7, value.avgPrice);
            stmt.bind(8, value.qtyAvgPrice);
            stmt.bind(9, value.maxPrice);
            stmt.exec();
        }
    }

    namespace config {
        std::string getString(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM strings WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0);
            }
            throw std::invalid_argument("key " + key + " not found in strings");
        }

        int getInt(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0);
            }
            throw std::invalid_argument("key " + key + " not found in ints");
        }

        bool getBool(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return ((int) query.getColumn(0)) != 0;
            }
            throw std::invalid_argument("key " + key + " not found in ints");
        }

        double getDouble(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM doubles WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0).getDouble();
            }
            throw std::invalid_argument("key " + key + " not found in floats");
        }

        void setString(const std::string &key, const std::string &value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO strings (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value);
            query.exec();
        }

        void setInt(const std::string &key, int value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO ints (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value);
            query.exec();
        }

        void setBool(const std::string &key, bool value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO ints (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value?1:0);
            query.exec();
        }

        void setDouble(const std::string &key, double value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO doubles (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value);
            query.exec();
        }
    }
}
