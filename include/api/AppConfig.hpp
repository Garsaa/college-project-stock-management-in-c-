#ifndef API_APP_CONFIG_HPP
#define API_APP_CONFIG_HPP

#include <cstddef>
#include <cstdint>
#include <string>

struct DatabaseConfig {
    std::string host = "127.0.0.1";
    std::uint16_t port = 5432;
    std::string databaseName = "inventory_api";
    std::string username = "postgres";
    std::string password;
    std::size_t connections = 4;
    std::string clientName = "default";
    bool fastClient = false;
    std::string charset = "UTF8";
    double timeout = -1.0;
    bool autoBatch = false;
    std::string sslMode;
};

struct CorsConfig {
    std::string allowedOrigin = "*";
    std::string allowedMethods = "GET, POST, PATCH, DELETE, OPTIONS";
    std::string allowedHeaders = "Content-Type, Authorization";
    std::string maxAge = "86400";
};

struct AppConfig {
    std::string listenHost = "0.0.0.0";
    std::uint16_t listenPort = 2020;
    std::size_t threads = 0;
    DatabaseConfig database;
    CorsConfig cors;

    static AppConfig loadFromEnvironment();
    std::string connectionString() const;
};

#endif
