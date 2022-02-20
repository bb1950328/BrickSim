#include "db.h"
#include <SQLiteCpp/Database.h>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace bricksim::db {
    namespace {
        std::optional<SQLite::Database> configDb;
        std::optional<SQLite::Database> cacheDb;

        const int NEWEST_CONFIG_DB_VERSION = 2;
        const int NEWEST_CACHE_DB_VERSION = 1;

        void upgradeConfigDbToVersion(int newVersion) {
            spdlog::info("Upgrading config.db3 to version {}", newVersion);
            switch (newVersion) {
                case 2: {
                    configDb->exec("CREATE TABLE key_shortcuts ("
                                   "   action INTEGER PRIMARY KEY,"
                                   "   key INTEGER,"
                                   "   modifier INTEGER,"
                                   "   event INTEGER);");
                }
                default:
                    break;
            }
            SQLite::Statement stmt(configDb.value(), "UPDATE meta SET version=?");
            stmt.bind(1, newVersion);
            stmt.exec();
        }

        void upgradeCacheDbToVersion(int newVersion) {
            spdlog::info("Upgrading cache.db3 to version {}", newVersion);
            switch (newVersion) {
                default:
                    break;
            }
            SQLite::Statement stmt(cacheDb.value(), "UPDATE meta SET version=?");
            stmt.bind(1, newVersion);
            stmt.exec();
        }

        void escapeSqlStringLiterals(std::string& str) {
            auto it = str.find('\'');
            while (it != std::string::npos) {
                str.insert(it, 1, '\'');
                it = str.find('\'', it + 2);//it+1 is the one we just inserted
            }
        }
    }

    void initialize() {
        bool configDbNew = !std::filesystem::is_regular_file("config.db3");
        bool cacheDbNew = !std::filesystem::is_regular_file("cache.db3");
        configDb = SQLite::Database("config.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
        cacheDb = SQLite::Database("cache.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

        if (configDbNew) {
            // !! DO NOT EDIT THIS SCRIPT ANYMORE -> add it in upgradeConfigDbToVersion and increment NEWEST_CONFIG_DB_VERSION by 1 !!
            configDb.value().exec("CREATE TABLE strings ("
                                  "   key TEXT PRIMARY KEY,"
                                  "   value TEXT NOT NULL);"
                                  //"CREATE UNIQUE INDEX idx_strings ON strings (key);"

                                  "CREATE TABLE ints ("
                                  "   key TEXT PRIMARY KEY,"
                                  "   value INTEGER NOT NULL);"
                                  //"CREATE UNIQUE INDEX idx_ints ON ints (key);"

                                  "CREATE TABLE doubles ("
                                  "   key TEXT PRIMARY KEY,"
                                  "   value REAL NOT NULL);"
                                  //"CREATE UNIQUE INDEX idx_doubles ON doubles (key);"

                                  "CREATE TABLE meta (version INTEGER);"
                                  "INSERT INTO meta (version) VALUES (1);");
        }
        while (true) {
            SQLite::Statement stmt(configDb.value(), "SELECT version FROM meta");
            if (stmt.executeStep()) {
                int currentDbVersion = stmt.getColumn(0);
                if (currentDbVersion < NEWEST_CONFIG_DB_VERSION) {
                    upgradeConfigDbToVersion(currentDbVersion + 1);
                } else {
                    break;
                }
            } else {
                throw std::invalid_argument("config.meta table should have exactly one record!");
            }
        }

        if (cacheDbNew) {
            // !! DO NOT EDIT THIS SCRIPT ANYMORE -> add it in upgradeCacheDbToVersion and increment NEWEST_CACHE_DB_VERSION by 1 !!
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

                                 "CREATE TABLE meta (version INTEGER);"
                                 "INSERT INTO meta (version) VALUES (1);");
        }
        while (true) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT version FROM meta");
            if (stmt.executeStep()) {
                int currentDbVersion = stmt.getColumn(0);
                if (currentDbVersion < NEWEST_CACHE_DB_VERSION) {
                    upgradeCacheDbToVersion(currentDbVersion + 1);
                } else {
                    break;
                }
            } else {
                throw std::invalid_argument("cache.meta table should have exactly one record!");
            }
        }
    }

    namespace requestCache {
        std::optional<std::string> get(const std::string& url) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT response FROM requestCache WHERE url=?;");
            stmt.bind(1, url);
            if (stmt.executeStep()) {
                return std::make_optional<std::string>(stmt.getColumn(0).getString());
            }
            return std::nullopt;
        }

        void put(const std::string& url, const std::string& response) {
            SQLite::Statement stmt(cacheDb.value(), "INSERT INTO requestCache (url, response) VALUES (?, ?);");
            stmt.bind(1, url);
            stmt.bind(2, response);
            stmt.exec();
        }
    }

    namespace priceGuideCache {
        std::optional<info_providers::price_guide::PriceGuide> get(const std::string& partCode, const std::string& currencyCode, const std::string& colorName) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT * FROM priceGuideCache WHERE partCode=? AND currencyCode=? AND colorName=?");
            stmt.bind(1, partCode);
            stmt.bind(2, currencyCode);
            stmt.bind(3, colorName);
            if (stmt.executeStep()) {
                return info_providers::price_guide::PriceGuide{
                        stmt.getColumn("available").getInt() != 0,
                        stmt.getColumn("currencyCode"),
                        stmt.getColumn("totalLots"),
                        stmt.getColumn("totalQty"),
                        static_cast<float>(stmt.getColumn("minPrice").getDouble()),
                        static_cast<float>(stmt.getColumn("avgPrice").getDouble()),
                        static_cast<float>(stmt.getColumn("qtyAvgPrice").getDouble()),
                        static_cast<float>(stmt.getColumn("maxPrice").getDouble())};
            }
            return {};
        }

        void put(const std::string& partCode, const std::string& currencyCode, const std::string& colorName, const info_providers::price_guide::PriceGuide& value) {
            SQLite::Statement stmt(cacheDb.value(),
                                   "REPLACE INTO priceGuideCache (partCode,currencyCode,colorName,available,totalLots,totalQty,minPrice,avgPrice,qtyAvgPrice,maxPrice) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
            stmt.bind(1, partCode);
            stmt.bind(2, currencyCode);
            stmt.bind(3, colorName);
            stmt.bind(4, value.available ? 1 : 0);
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
        std::optional<std::string> getString(const char* key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM strings WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return std::make_optional<std::string>(query.getColumn(0).getString());
            }
            return std::nullopt;
        }

        std::optional<int> getInt(const char* key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0).getInt();
            }
            return {};
        }

        std::optional<double> getDouble(const char* key) {
            SQLite::Statement query(configDb.value(), "SELECT value FROM doubles WHERE key=?");
            query.bind(1, key);
            if (query.executeStep()) {
                return query.getColumn(0).getDouble();
            }
            return {};
        }

        void setString(const char* key, const std::string& value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO strings (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value);
            query.exec();
        }

        void setInt(const char* key, int value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO ints (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value);
            query.exec();
        }

        void setDouble(const char* key, double value) {
            SQLite::Statement query(configDb.value(), "REPLACE INTO doubles (key, value) VALUES (?, ?)");
            query.bind(1, key);
            query.bind(2, value);
            query.exec();
        }

        void deleteAll() {
            SQLite::Statement query(configDb.value(), "DELETE FROM strings; DELETE FROM ints; DELETE FROM doubles;");
            query.exec();
        }
    }

    namespace fileList {

        int getSize() {
            SQLite::Statement stmt(cacheDb.value(), "SELECT COUNT(*) FROM files;");
            if (stmt.executeStep()) {
                return stmt.getColumn(0);
            }
            return -1;
        }

        void put(const std::string& name, const std::string& title, const std::string& category) {
            SQLite::Statement stmt(cacheDb.value(), "INSERT INTO files (name, title, category) VALUES (?, ?, ?);");
            stmt.bind(1, name);
            stmt.bind(2, title);
            stmt.bind(3, category);
            stmt.exec();
        }

        oset_t<std::string> getAllCategories() {
            SQLite::Statement stmt(cacheDb.value(), "SELECT DISTINCT category FROM files ORDER BY category;");
            oset_t<std::string> result;
            while (stmt.executeStep()) {
                result.insert(stmt.getColumn(0));
            }
            return result;
        }

        uoset_t<std::string> getAllFiles() {
            SQLite::Statement stmt(cacheDb.value(), "SELECT name FROM files;");
            uoset_t<std::string> result;
            while (stmt.executeStep()) {
                result.insert(stmt.getColumn(0));
            }
            return result;
        }

        oset_t<std::string> getAllPartsForCategory(const std::string& category) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT name FROM files WHERE category=? ORDER BY title;");
            stmt.bind(1, category);
            oset_t<std::string> result;
            while (stmt.executeStep()) {
                result.insert(stmt.getColumn(0));
            }
            return result;
        }

        std::optional<std::string> containsFile(const std::string& name) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT name FROM files WHERE name=?;");
            stmt.bind(1, name);
            if (stmt.executeStep()) {
                return std::make_optional<std::string>(stmt.getColumn(0).getString());
            }
            return std::nullopt;
        }

        std::optional<Entry> findFile(const std::string& name) {
            SQLite::Statement stmt(cacheDb.value(), "SELECT name, title, category FROM files WHERE name=?;");
            stmt.bind(1, name);
            if (stmt.executeStep()) {
                return {{stmt.getColumn(0),
                         stmt.getColumn(1),
                         stmt.getColumn(2)}};
            } else {
                return {};
            }
        }

        void put(const std::vector<Entry>& entries) {
            std::string command = "INSERT INTO files (name, title, category) VALUES ";
            std::string name, title, category;
            for (const auto& entry: entries) {
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
            SQLite::Statement stmt(cacheDb.value(), command);
            stmt.exec();
        }
    }

    namespace key_shortcuts {

        std::vector<record_t> loadShortcuts() {
            SQLite::Statement stmt(configDb.value(), "SELECT action, key, modifier, event FROM key_shortcuts;");
            auto result = std::vector<record_t>();
            while (stmt.executeStep()) {
                result.emplace_back(
                        stmt.getColumn(0).getInt(),
                        stmt.getColumn(1).getInt(),
                        stmt.getColumn(2).getInt(),
                        stmt.getColumn(3).getInt());
            }
            return result;
        }

        void saveShortcut(record_t record) {
            SQLite::Statement stmt(configDb.value(), "INSERT INTO key_shortcuts (action, key, modifier, event) VALUES (?, ?, ?, ?);");
            stmt.bind(1, std::get<0>(record));
            stmt.bind(2, std::get<1>(record));
            stmt.bind(3, std::get<2>(record));
            stmt.bind(4, std::get<3>(record));
            stmt.exec();
        }

        void deleteAll() {
            configDb->exec("DELETE FROM key_shortcuts");
        }
    }
}
