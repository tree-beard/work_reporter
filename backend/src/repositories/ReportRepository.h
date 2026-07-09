#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "../db/Database.h"
#include "../models/WorkReport.h"

class ReportRepository {
public:
    explicit ReportRepository(Database& database) : db_(database) {}

    int64_t create(const WorkReport& report);
    std::optional<WorkReport> findById(int64_t id, int64_t userId);
    std::vector<WorkReport> findByUser(int64_t userId);
    bool update(const WorkReport& report);
    bool remove(int64_t id, int64_t userId);

private:
    Database& db_;
};
