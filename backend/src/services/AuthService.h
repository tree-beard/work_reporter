#pragma once

#include <string>

#include "../repositories/UserRepository.h"

struct AuthResult {
    bool success = false;
    std::string token;
    std::string error;
};

class AuthService {
public:
    AuthService(UserRepository& userRepository, std::string jwtSecret)
        : users_(userRepository), jwtSecret_(std::move(jwtSecret)) {}

    AuthResult registerUser(const std::string& email, const std::string& password);
    AuthResult login(const std::string& email, const std::string& password);

private:
    UserRepository& users_;
    std::string jwtSecret_;

    std::string issueToken(int64_t userId) const;
};
