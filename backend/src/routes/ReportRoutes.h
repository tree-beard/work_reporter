#pragma once

#include <httplib/httplib.h>

#include <string>

#include "../services/ReportService.h"

void registerReportRoutes(httplib::Server& server, ReportService& reportService, const std::string& jwtSecret);
