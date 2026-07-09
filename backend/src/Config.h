#pragma once

#include <cstdlib>
#include <string>

struct Config {
    std::string dbPath;
    std::string jwtSecret;
    int port;

    static Config fromEnv() {
        Config config;
        config.dbPath = getEnvOr("DB_PATH", "data/work_reporter.db");
        config.jwtSecret = getEnvOr("JWT_SECRET", "dev-secret-change-me");
        config.port = std::stoi(getEnvOr("PORT", "8080"));
        return config;
    }

private:
    static std::string getEnvOr(const char* name, const std::string& fallback) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : fallback;
    }
};
