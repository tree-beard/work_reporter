#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "../db/Database.h"

struct User {
    int64_t id = 0;
    std::string email;
    std::string password_hash;
};

class UserRepository {
public:
    explicit UserRepository(Database& database) : db_(database) {}

    std::optional<User> findByEmail(const std::string& email);
    int64_t create(const std::string& email, const std::string& passwordHash);

private:
    Database& db_;
};
