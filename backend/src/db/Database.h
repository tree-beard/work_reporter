#pragma once

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class Database {
public:
    explicit Database(const std::string& path);

    SQLite::Database& handle();

private:
    SQLite::Database db_;
};
