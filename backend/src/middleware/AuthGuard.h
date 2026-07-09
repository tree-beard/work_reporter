#pragma once

#include <httplib/httplib.h>

#include <cstdint>
#include <optional>
#include <string>

// Verifies the "Authorization: Bearer <token>" header. On success returns
// the authenticated user's id. On failure writes a 401 response and
// returns std::nullopt -- callers should return immediately in that case.
std::optional<int64_t> authenticate(const httplib::Request& req, httplib::Response& res,
                                     const std::string& jwtSecret);
