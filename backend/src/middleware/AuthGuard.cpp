#include "AuthGuard.h"

#include <jwt-cpp/traits/nlohmann-json/defaults.h>

std::optional<int64_t> authenticate(const httplib::Request& req, httplib::Response& res,
                                     const std::string& jwtSecret) {
    const std::string prefix = "Bearer ";
    std::string header = req.get_header_value("Authorization");
    if (header.rfind(prefix, 0) != 0) {
        res.status = 401;
        res.set_content("Missing or malformed Authorization header", "text/plain");
        return std::nullopt;
    }

    try {
        auto token = header.substr(prefix.size());
        auto decoded = jwt::decode(token);

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{jwtSecret})
            .with_issuer("work-reporter")
            .verify(decoded);

        return std::stoll(decoded.get_payload_claim("sub").as_string());
    } catch (const std::exception&) {
        res.status = 401;
        res.set_content("Invalid or expired token", "text/plain");
        return std::nullopt;
    }
}
