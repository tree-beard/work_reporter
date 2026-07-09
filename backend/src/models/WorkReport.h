#pragma once

#include <cstdint>
#include <string>

struct WorkReport {
    int64_t id = 0;
    int64_t user_id = 0;
    std::string report_date;
    std::string start_time;
    std::string end_time;
    int break_minutes = 0;
    std::string summary;
};
