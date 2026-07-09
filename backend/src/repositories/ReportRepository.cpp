#include "ReportRepository.h"

namespace {

WorkReport mapRow(SQLite::Statement& query) {
    WorkReport report;
    report.id = query.getColumn(0).getInt64();
    report.user_id = query.getColumn(1).getInt64();
    report.report_date = query.getColumn(2).getString();
    report.start_time = query.getColumn(3).getString();
    report.end_time = query.getColumn(4).getString();
    report.break_minutes = query.getColumn(5).getInt();
    report.summary = query.getColumn(6).getString();
    return report;
}

const char* kSelectColumns =
    "id, user_id, report_date, start_time, end_time, break_minutes, summary";

}  // namespace

int64_t ReportRepository::create(const WorkReport& report) {
    SQLite::Statement insert(db_.handle(),
        "INSERT INTO work_reports (user_id, report_date, start_time, end_time, break_minutes, summary) "
        "VALUES (?, ?, ?, ?, ?, ?)");
    insert.bind(1, report.user_id);
    insert.bind(2, report.report_date);
    insert.bind(3, report.start_time);
    insert.bind(4, report.end_time);
    insert.bind(5, report.break_minutes);
    insert.bind(6, report.summary);
    insert.exec();
    return db_.handle().getLastInsertRowid();
}

std::optional<WorkReport> ReportRepository::findById(int64_t id, int64_t userId) {
    SQLite::Statement query(db_.handle(),
        std::string("SELECT ") + kSelectColumns + " FROM work_reports WHERE id = ? AND user_id = ?");
    query.bind(1, id);
    query.bind(2, userId);

    if (!query.executeStep()) {
        return std::nullopt;
    }
    return mapRow(query);
}

std::vector<WorkReport> ReportRepository::findByUser(int64_t userId) {
    SQLite::Statement query(db_.handle(),
        std::string("SELECT ") + kSelectColumns + " FROM work_reports WHERE user_id = ? ORDER BY report_date DESC");
    query.bind(1, userId);

    std::vector<WorkReport> reports;
    while (query.executeStep()) {
        reports.push_back(mapRow(query));
    }
    return reports;
}

bool ReportRepository::update(const WorkReport& report) {
    SQLite::Statement update(db_.handle(),
        "UPDATE work_reports SET report_date = ?, start_time = ?, end_time = ?, "
        "break_minutes = ?, summary = ?, updated_at = datetime('now') "
        "WHERE id = ? AND user_id = ?");
    update.bind(1, report.report_date);
    update.bind(2, report.start_time);
    update.bind(3, report.end_time);
    update.bind(4, report.break_minutes);
    update.bind(5, report.summary);
    update.bind(6, report.id);
    update.bind(7, report.user_id);
    return update.exec() > 0;
}

bool ReportRepository::remove(int64_t id, int64_t userId) {
    SQLite::Statement del(db_.handle(), "DELETE FROM work_reports WHERE id = ? AND user_id = ?");
    del.bind(1, id);
    del.bind(2, userId);
    return del.exec() > 0;
}
