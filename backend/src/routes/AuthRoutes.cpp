#include "AuthRoutes.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

void sendJson(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

}  // namespace

void registerAuthRoutes(httplib::Server& server, AuthService& authService) {
    server.Post("/api/auth/register", [&authService](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("email") || !body.contains("password")) {
            sendJson(res, 400, {{"error", "email and password are required"}});
            return;
        }

        auto result = authService.registerUser(
            body["email"].get<std::string>(), body["password"].get<std::string>());
        if (!result.success) {
            sendJson(res, 400, {{"error", result.error}});
            return;
        }
        sendJson(res, 201, {{"token", result.token}});
    });

    server.Post("/api/auth/login", [&authService](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("email") || !body.contains("password")) {
            sendJson(res, 400, {{"error", "email and password are required"}});
            return;
        }

        auto result = authService.login(
            body["email"].get<std::string>(), body["password"].get<std::string>());
        if (!result.success) {
            sendJson(res, 401, {{"error", result.error}});
            return;
        }
        sendJson(res, 200, {{"token", result.token}});
    });
}
