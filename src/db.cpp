//
// Created by Bader on 10.11.2020.
//

#include <memory>
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

        "CREATE TABLE IF NOT EXISTS floats ("
        "key TEXT PRIMARY KEY,"
        "value REAL NOT NULL);"
        "CREATE UNIQUE INDEX IF NOT EXISTS idx_floats ON floats (key)",
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
            SQLite::Statement stmt(configDb.value(), script);
            std::cout << stmt.exec() << std::endl;
        }

        SQLite::Statement stmt(fileListDb.value(),
                                "CREATE TABLE IF NOT EXISTS files ("
                                "name TEXT PRIMARY KEY COLLATE NOCASE,"
                                "title TEXT,"
                                "category TEXT NOT NULL);"
                                "CREATE UNIQUE INDEX IF NOT EXISTS idx_files ON files (name)");
        stmt.exec();
    }
}
