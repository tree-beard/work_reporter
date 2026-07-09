#include "Database.h"

Database::Database(const std::string& path)
    : db_(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    db_.exec("PRAGMA journal_mode=WAL;");
    db_.exec("PRAGMA foreign_keys=ON;");

    db_.exec(R"sql(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            created_at TEXT NOT NULL DEFAULT (datetime('now'))
        )
    )sql");

    db_.exec(R"sql(
        CREATE TABLE IF NOT EXISTS work_reports (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL REFERENCES users(id),
            report_date TEXT NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT NOT NULL,
            break_minutes INTEGER NOT NULL DEFAULT 0,
            summary TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL DEFAULT (datetime('now')),
            updated_at TEXT NOT NULL DEFAULT (datetime('now'))
        )
    )sql");

    db_.exec("CREATE INDEX IF NOT EXISTS idx_work_reports_user_date ON work_reports(user_id, report_date)");
}

SQLite::Database& Database::handle() {
    return db_;
}
