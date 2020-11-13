//
// Created by Bader on 10.11.2020.
//

#include <SQLiteCpp/Database.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include "db.h"


namespace db {
    namespace {
        std::optional<SQLite::Database> configDb;
        std::optional<SQLite::Database> cacheDb;

        const int NEWEST_CONFIG_DB_VERSION = 1;
        const int NEWEST_CACHE_DB_VERSION = 1;

        void upgradeConfigDbToVersion(int newVersion) {
            std::cout << "INFO: Upgrading config.db3 to version " << newVersion << std::endl;
            switch (newVersion) {
                default: break;
            }
        }

        void upgradeCacheDbToVersion(int newVersion) {
            std::cout << "INFO: Upgrading cache.db3 to version " << newVersion << std::endl;
            switch (newVersion) {
                default: break;
            }
        }
    }

    void initialize() {
        bool configDbNew = !std::filesystem::is_regular_file("config.db3");
        bool cacheDbNew = !std::filesystem::is_regular_file("cache.db3");
        configDb = SQLite::Database("config.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
        cacheDb = SQLite::Database("cache.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

        if (configDbNew) {
            configDb.value().exec("CREATE TABLE strings ("
                                  "   key TEXT PRIMARY KEY,"
                                  "   value TEXT NOT NULL);"
                                  "CREATE UNIQUE INDEX idx_strings ON strings (key);"

                                  "CREATE TABLE ints ("
                                  "   key TEXT PRIMARY KEY,"
                                  "   value INTEGER NOT NULL);"
                                  "CREATE UNIQUE INDEX idx_ints ON ints (key);"

                                  "CREATE TABLE doubles ("
                                  "   key TEXT PRIMARY KEY,"
                                  "   value REAL NOT NULL);"
                                  "CREATE UNIQUE INDEX idx_doubles ON doubles (key);"

                                  "CREATE TABLE meta (version INTEGER);");

            SQLite::Statement stmt(configDb.value(), "INSERT INTO meta (version) VALUES (?)");
            stmt.bind(1, NEWEST_CONFIG_DB_VERSION);
            stmt.exec();
        }
        while (true) {
            SQLite::Statement stmt(configDb.value(), "SELECT version FROM meta");
            if (stmt.executeStep()) {
                int currentDbVersion = stmt.getColumn(0);
                if (currentDbVersion < NEWEST_CONFIG_DB_VERSION) {
                    upgradeConfigDbToVersion(currentDbVersion+1);
                } else {
                    break;
                }
            }
        }

        if (cacheDbNew) {
            cacheDb.value().exec("CREATE TABLE files ("
                                "   name TEXT PRIMARY KEY COLLATE NOCASE,"
                                "   title TEXT,"
                                "   category TEXT NOT NULL"
                                ");"
                                "CREATE UNIQUE INDEX idx_files ON files (name);"

                                "CREATE TABLE requestCache ("
                                "   url TEXT PRIMARY KEY COLLATE NOCASE,"
                                "   response TEXT"
                                ");"
                                "CREATE UNIQUE INDEX idx_requestCache ON requestCache (url);"

                                "CREATE TABLE priceGuideCache ("
                                "   partCode TEXT COLLATE NOCASE,"
                                "   currencyCode TEXT NOT NULL,"
                                "   colorName TEXT NOT NULL,"
                                "   available INTEGER,"
                                "   totalLots INTEGER NOT NULL,"
                                "   totalQty INTEGER NOT NULL,"
                                "   minPrice REAL NOT NULL,"
                                "   avgPrice REAL NOT NULL,"
                                "   qtyAvgPrice REAL NOT NULL,"
                                "   maxPrice REAL NOT NULL"
                                ");"
                                "CREATE UNIQUE INDEX idx_priceGuideCache ON priceGuideCache(partCode, currencyCode, colorName);"

                                "CREATE TABLE meta (version INTEGER)");
            SQLite::Statement stmt(cacheDb.value(), "INSERT INTO meta (version) VALUES (?)");
            stmt.bind(1, NEWEST_CACHE_DB_VERSION);
            stmt.exec();
        }
        while (true) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT version FROM meta");
            if (stmt.executeStep()) {
                int currentDbVersion = stmt.getColumn(0);
                if (currentDbVersion < NEWEST_CACHE_DB_VERSION) {
                    upgradeCacheDbToVersion(currentDbVersion+1);
                } else {
                    break;
                }
            }
        }
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
                    stmt.getColumn("available").getInt()!=0,
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
            SQLite::Statement stmt(cacheDb.value(), "REPLACE INTO priceGuideCache (partCode,currencyCode,colorName,available,totalLots,totalQty,minPrice,avgPrice,qtyAvgPrice,maxPrice) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
            stmt.bind(1, partCode);
            stmt.bind(2, currencyCode);
            stmt.bind(3, colorName);
            stmt.bind(4, value.available?1:0);
            stmt.bind(5, value.totalLots);
            stmt.bind(6, value.totalQty);
            stmt.bind(7, value.minPrice);
            stmt.bind(8, value.avgPrice);
            stmt.bind(9, value.qtyAvgPrice);
            stmt.bind(10, value.maxPrice);
            stmt.exec();
        }
    }

    namespace config {
        std::optional<std::string> getString(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM strings WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0);
            }
            return {};
        }

        std::optional<int> getInt(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0);
            }
            return {};
        }

        std::optional<bool> getBool(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return ((int) query.getColumn(0)) != 0;
            }
            return {};
        }

        std::optional<double> getDouble(const std::string &key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM doubles WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0).getDouble();
            }
            return {};
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

    int fileList::getSize() {
        SQLite::Statement stmt(cacheDb.value(), "SELECT COUNT(*) FROM files;");
        if (stmt.executeStep()) {
            return stmt.getColumn(0);
        }
        return -1;
    }

    void fileList::put(const std::string &name, const std::string &title, const std::string &category) {
        SQLite::Statement stmt(cacheDb.value(), "INSERT INTO files (name, title, category) VALUES (?, ?, ?);");
        stmt.bind(1, name);
        stmt.bind(2, title);
        stmt.bind(3, category);
        stmt.exec();
    }

    std::set<std::string> fileList::getAllCategories() {
        SQLite::Statement stmt(cacheDb.value(), "SELECT DISTINCT category FROM files;");
        std::set<std::string> result;
        while (stmt.executeStep()) {
            result.insert(stmt.getColumn(0));
        }
        return result;
    }

    std::set<std::string> fileList::getAllFiles() {
        SQLite::Statement stmt(cacheDb.value(), "SELECT name FROM files;");
        std::set<std::string> result;
        while (stmt.executeStep()) {
            result.insert(stmt.getColumn(0));
        }
        return result;
    }

    std::set<std::string> fileList::getAllFilesForCategory(const std::string &category) {
        SQLite::Statement stmt(cacheDb.value(), "SELECT name FROM files WHERE category=?;");
        stmt.bind(1, category);
        std::set<std::string> result;
        while (stmt.executeStep()) {
            result.insert(stmt.getColumn(0));
        }
        return result;
    }

    std::optional<std::string> fileList::containsFile(const std::string &name) {
        SQLite::Statement stmt(cacheDb.value(), "SELECT name FROM files WHERE name=?;");
        stmt.bind(1, name);
        if (stmt.executeStep()) {
            return stmt.getColumn(0);
        } else {
            return {};
        }
    }

    void escapeSqlStringLiterals(std::string& str) {
        auto it = str.find('\'');
        while (it != std::string::npos) {
            str.insert(it, 1, '\'');
            it = str.find('\'', it+2);//it+1 is the one we just inserted
        }
    }

    void fileList::put(const std::vector<Entry> &entries) {
        std::string command = "INSERT INTO files (name, title, category) VALUES ";
        std::string name, title, category;
        for (const auto &entry : entries) {
            name = entry.name;
            title = entry.title;
            category = entry.category;
            escapeSqlStringLiterals(name);
            escapeSqlStringLiterals(title);
            escapeSqlStringLiterals(category);
            command += "('";
            command += name;
            command += "','";
            command += title;
            command += "','";
            command += category;
            command += "'),";
        }
        command.pop_back();//last comma
        command.push_back(';');
        std::cout << command << std::endl;
        SQLite::Statement stmt(cacheDb.value(), command);
        stmt.exec();
    }
}
