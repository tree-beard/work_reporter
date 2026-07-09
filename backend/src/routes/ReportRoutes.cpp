#include "ReportRoutes.h"

#include <nlohmann/json.hpp>

#include "../middleware/AuthGuard.h"

using json = nlohmann::json;

namespace {

void sendJson(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

json toJson(const WorkReport& report) {
    return {
        {"id", report.id},
        {"report_date", report.report_date},
        {"start_time", report.start_time},
        {"end_time", report.end_time},
        {"break_minutes", report.break_minutes},
        {"summary", report.summary},
    };
}

ReportInput parseInput(const json& body) {
    ReportInput input;
    input.report_date = body.value("report_date", "");
    input.start_time = body.value("start_time", "");
    input.end_time = body.value("end_time", "");
    input.break_minutes = body.value("break_minutes", 0);
    input.summary = body.value("summary", "");
    return input;
}

}  // namespace

void registerReportRoutes(httplib::Server& server, ReportService& reportService, const std::string& jwtSecret) {
    server.Get("/api/reports",
        [&reportService, jwtSecret](const httplib::Request& req, httplib::Response& res) {
            auto userId = authenticate(req, res, jwtSecret);
            if (!userId) return;

            auto reports = reportService.listForUser(*userId);
            json body = json::array();
            for (const auto& report : reports) {
                body.push_back(toJson(report));
            }
            sendJson(res, 200, body);
        });

    server.Post("/api/reports",
        [&reportService, jwtSecret](const httplib::Request& req, httplib::Response& res) {
            auto userId = authenticate(req, res, jwtSecret);
            if (!userId) return;

            auto body = json::parse(req.body, nullptr, false);
            if (body.is_discarded()) {
                sendJson(res, 400, {{"error", "invalid JSON"}});
                return;
            }

            auto input = parseInput(body);
            if (auto error = reportService.validate(input)) {
                sendJson(res, 400, {{"error", *error}});
                return;
            }

            int64_t id = reportService.create(*userId, input);
            auto report = reportService.get(id, *userId);
            sendJson(res, 201, toJson(*report));
        });

    server.Get("/api/reports/:id",
        [&reportService, jwtSecret](const httplib::Request& req, httplib::Response& res) {
            auto userId = authenticate(req, res, jwtSecret);
            if (!userId) return;

            int64_t id = std::stoll(req.path_params.at("id"));
            auto report = reportService.get(id, *userId);
            if (!report) {
                sendJson(res, 404, {{"error", "not found"}});
                return;
            }
            sendJson(res, 200, toJson(*report));
        });

    server.Put("/api/reports/:id",
        [&reportService, jwtSecret](const httplib::Request& req, httplib::Response& res) {
            auto userId = authenticate(req, res, jwtSecret);
            if (!userId) return;

            auto body = json::parse(req.body, nullptr, false);
            if (body.is_discarded()) {
                sendJson(res, 400, {{"error", "invalid JSON"}});
                return;
            }

            auto input = parseInput(body);
            if (auto error = reportService.validate(input)) {
                sendJson(res, 400, {{"error", *error}});
                return;
            }

            int64_t id = std::stoll(req.path_params.at("id"));
            if (!reportService.update(id, *userId, input)) {
                sendJson(res, 404, {{"error", "not found"}});
                return;
            }
            auto report = reportService.get(id, *userId);
            sendJson(res, 200, toJson(*report));
        });

    server.Delete("/api/reports/:id",
        [&reportService, jwtSecret](const httplib::Request& req, httplib::Response& res) {
            auto userId = authenticate(req, res, jwtSecret);
            if (!userId) return;

            int64_t id = std::stoll(req.path_params.at("id"));
            if (!reportService.remove(id, *userId)) {
                sendJson(res, 404, {{"error", "not found"}});
                return;
            }
            res.status = 204;
        });
}
