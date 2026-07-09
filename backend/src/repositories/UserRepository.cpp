#include "UserRepository.h"

std::optional<User> UserRepository::findByEmail(const std::string& email) {
    SQLite::Statement query(db_.handle(), "SELECT id, email, password_hash FROM users WHERE email = ?");
    query.bind(1, email);

    if (!query.executeStep()) {
        return std::nullopt;
    }

    User user;
    user.id = query.getColumn(0).getInt64();
    user.email = query.getColumn(1).getString();
    user.password_hash = query.getColumn(2).getString();
    return user;
}

int64_t UserRepository::create(const std::string& email, const std::string& passwordHash) {
    SQLite::Statement insert(db_.handle(), "INSERT INTO users (email, password_hash) VALUES (?, ?)");
    insert.bind(1, email);
    insert.bind(2, passwordHash);
    insert.exec();
    return db_.handle().getLastInsertRowid();
}
