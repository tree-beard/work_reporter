#include "ReportService.h"

namespace {

WorkReport toModel(int64_t id, int64_t userId, const ReportInput& input) {
    WorkReport report;
    report.id = id;
    report.user_id = userId;
    report.report_date = input.report_date;
    report.start_time = input.start_time;
    report.end_time = input.end_time;
    report.break_minutes = input.break_minutes;
    report.summary = input.summary;
    return report;
}

}  // namespace

std::optional<std::string> ReportService::validate(const ReportInput& input) const {
    if (input.report_date.empty()) {
        return "report_date is required";
    }
    if (input.start_time.empty() || input.end_time.empty()) {
        return "start_time and end_time are required";
    }
    // Assumes zero-padded 24h "HH:MM" strings, so lexical order matches time order.
    if (input.end_time <= input.start_time) {
        return "end_time must be after start_time";
    }
    if (input.break_minutes < 0) {
        return "break_minutes cannot be negative";
    }
    return std::nullopt;
}

int64_t ReportService::create(int64_t userId, const ReportInput& input) {
    return reports_.create(toModel(0, userId, input));
}

std::vector<WorkReport> ReportService::listForUser(int64_t userId) {
    return reports_.findByUser(userId);
}

std::optional<WorkReport> ReportService::get(int64_t id, int64_t userId) {
    return reports_.findById(id, userId);
}

bool ReportService::update(int64_t id, int64_t userId, const ReportInput& input) {
    return reports_.update(toModel(id, userId, input));
}

bool ReportService::remove(int64_t id, int64_t userId) {
    return reports_.remove(id, userId);
}
