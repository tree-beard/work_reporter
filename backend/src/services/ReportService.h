#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../repositories/ReportRepository.h"

struct ReportInput {
    std::string report_date;
    std::string start_time;
    std::string end_time;
    int break_minutes = 0;
    std::string summary;
};

class ReportService {
public:
    explicit ReportService(ReportRepository& reportRepository) : reports_(reportRepository) {}

    std::optional<std::string> validate(const ReportInput& input) const;

    int64_t create(int64_t userId, const ReportInput& input);
    std::vector<WorkReport> listForUser(int64_t userId);
    std::optional<WorkReport> get(int64_t id, int64_t userId);
    bool update(int64_t id, int64_t userId, const ReportInput& input);
    bool remove(int64_t id, int64_t userId);

private:
    ReportRepository& reports_;
};
