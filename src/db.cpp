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
        std::optional<SQLite::Database> fileListDb;
    }

    void initialize() {
        configDb = SQLite::Database("config.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
        fileListDb = SQLite::Database("fileList.db3", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

        for (const auto &script : CONFIG_TABLE_CREATION_SCRIPTS) {
            configDb.value().exec(script);
        }

        fileListDb.value().exec("CREATE TABLE IF NOT EXISTS files ("
                                "name TEXT PRIMARY KEY COLLATE NOCASE,"
                                "title TEXT,"
                                "category TEXT NOT NULL);"
                                "CREATE UNIQUE INDEX IF NOT EXISTS idx_files ON files (name)");
    }

    std::string config::getString(const std::string &key) {
        SQLite::Statement query(configDb.value(), "SELECT value FROM strings WHERE key=?");
        query.bind(1, key);
        if (query.executeStep()) {
            return query.getColumn(0);
        }
        throw std::invalid_argument("key " + key + " not found in strings");
    }

    int config::getInt(const std::string &key) {
        SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
        query.bind(1, key);
        if (query.executeStep()) {
            return query.getColumn(0);
        }
        throw std::invalid_argument("key " + key + " not found in ints");
    }

    bool config::getBool(const std::string &key) {
        SQLite::Statement query(configDb.value(), "SELECT value FROM ints WHERE key=?");
        query.bind(1, key);
        if (query.executeStep()) {
            return ((int) query.getColumn(0)) != 0;
        }
        throw std::invalid_argument("key " + key + " not found in ints");
    }

    double config::getDouble(const std::string &key) {
        SQLite::Statement query(configDb.value(), "SELECT value FROM doubles WHERE key=?");
        query.bind(1, key);
        if (query.executeStep()) {
            return query.getColumn(0).getDouble();
        }
        throw std::invalid_argument("key " + key + " not found in floats");
    }

    void config::setString(const std::string &key, const std::string &value) {
        SQLite::Statement query(configDb.value(), "REPLACE INTO strings (key, value) VALUES (?, ?)");
        query.bind(1, key);
        query.bind(2, value);
        query.exec();
    }

    void config::setInt(const std::string &key, int value) {
        SQLite::Statement query(configDb.value(), "REPLACE INTO ints (key, value) VALUES (?, ?)");
        query.bind(1, key);
        query.bind(2, value);
        query.exec();
    }

    void config::setBool(const std::string &key, bool value) {
        SQLite::Statement query(configDb.value(), "REPLACE INTO ints (key, value) VALUES (?, ?)");
        query.bind(1, key);
        query.bind(2, value?1:0);
        query.exec();
    }

    void config::setDouble(const std::string &key, double value) {
        SQLite::Statement query(configDb.value(), "REPLACE INTO doubles (key, value) VALUES (?, ?)");
        query.bind(1, key);
        query.bind(2, value);
        query.exec();
    }
}
