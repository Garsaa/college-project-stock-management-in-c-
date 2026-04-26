#include "api/AppConfig.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {

std::string trim(std::string text) {
    const auto firstNonWhitespace = text.find_first_not_of(" \t\r\n");
    if (firstNonWhitespace == std::string::npos) {
        return "";
    }

    const auto lastNonWhitespace = text.find_last_not_of(" \t\r\n");
    return text.substr(firstNonWhitespace, lastNonWhitespace - firstNonWhitespace + 1);
}

std::string stripOptionalQuotes(std::string value) {
    if (value.size() >= 2) {
        const char first = value.front();
        const char last = value.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return value.substr(1, value.size() - 2);
        }
    }
    return value;
}

std::unordered_map<std::string, std::string> loadDotEnvFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line)) {
        const auto trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine.front() == '#') {
            continue;
        }

        const auto separator = trimmedLine.find('=');
        if (separator == std::string::npos) {
            continue;
        }

        auto key = trim(trimmedLine.substr(0, separator));
        auto value = trim(trimmedLine.substr(separator + 1));
        if (key.empty()) {
            continue;
        }

        values[std::move(key)] = stripOptionalQuotes(std::move(value));
    }

    return values;
}

const std::unordered_map<std::string, std::string>& dotEnvValues() {
    static const auto values = loadDotEnvFile(".env");
    return values;
}

std::string readEnv(const char* name, const std::string& defaultValue) {
    if (const char* value = std::getenv(name)) {
        return value;
    }

    const auto& values = dotEnvValues();
    if (const auto entry = values.find(name); entry != values.end()) {
        return entry->second;
    }

    return defaultValue;
}

std::string readFirstAvailableEnv(
    std::initializer_list<const char*> names,
    const std::string& defaultValue) {
    for (const auto* name : names) {
        const auto value = readEnv(name, "");
        if (!value.empty()) {
            return value;
        }
    }

    return defaultValue;
}

std::uint16_t readPort(const char* name, std::uint16_t defaultValue) {
    const auto value = readEnv(name, "");
    if (value.empty()) {
        return defaultValue;
    }
    const auto portNumber = std::stoi(value);
    if (portNumber < 1 || portNumber > 65535) {
        throw std::runtime_error(std::string("Invalid port in ") + name + ".");
    }
    return static_cast<std::uint16_t>(portNumber);
}

std::uint16_t readPort(
    std::initializer_list<const char*> names,
    std::uint16_t defaultValue) {
    const auto value = readFirstAvailableEnv(names, "");
    if (value.empty()) {
        return defaultValue;
    }

    const auto portNumber = std::stoi(value);
    if (portNumber < 1 || portNumber > 65535) {
        throw std::runtime_error("Invalid port value in the provided environment variables.");
    }

    return static_cast<std::uint16_t>(portNumber);
}

std::size_t readSize(const char* name, std::size_t defaultValue) {
    const auto value = readEnv(name, "");
    if (value.empty()) {
        return defaultValue;
    }
    const auto parsedValue = std::stoll(value);
    if (parsedValue < 0) {
        throw std::runtime_error(std::string("Invalid value in ") + name + ".");
    }
    return static_cast<std::size_t>(parsedValue);
}

double readDouble(const char* name, double defaultValue) {
    const auto value = readEnv(name, "");
    if (value.empty()) {
        return defaultValue;
    }
    return std::stod(value);
}

bool readBool(const char* name, bool defaultValue) {
    const auto value = readEnv(name, "");
    if (value.empty()) {
        return defaultValue;
    }

    std::string normalizedValue = value;
    std::transform(
        normalizedValue.begin(),
        normalizedValue.end(),
        normalizedValue.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

    if (normalizedValue == "1" || normalizedValue == "true" ||
        normalizedValue == "yes" || normalizedValue == "on") {
        return true;
    }
    if (normalizedValue == "0" || normalizedValue == "false" ||
        normalizedValue == "no" || normalizedValue == "off") {
        return false;
    }

    throw std::runtime_error(std::string("Invalid boolean in ") + name + ".");
}

std::string escapeConnectionValue(const std::string& value) {
    std::string escapedValue;
    escapedValue.reserve(value.size() + 8);

    for (const char c : value) {
        if (c == '\\' || c == '\'') {
            escapedValue.push_back('\\');
        }
        escapedValue.push_back(c);
    }

    return escapedValue;
}

}

AppConfig AppConfig::loadFromEnvironment() {
    AppConfig config;
    config.listenHost = readEnv("APP_HOST", config.listenHost);
    config.listenPort = readPort({"PORT", "APP_PORT"}, config.listenPort);
    config.threads = readSize("APP_THREADS", config.threads);

    config.database.host = readEnv("POSTGRES_HOST", config.database.host);
    config.database.port = readPort("POSTGRES_PORT", config.database.port);
    config.database.databaseName = readEnv("POSTGRES_DB", config.database.databaseName);
    config.database.username = readEnv("POSTGRES_USER", config.database.username);
    config.database.password = readEnv("POSTGRES_PASSWORD", config.database.password);
    config.database.connections = readSize("POSTGRES_CONNECTIONS", config.database.connections);
    config.database.clientName = readEnv("POSTGRES_CLIENT_NAME", config.database.clientName);
    config.database.fastClient = readBool("POSTGRES_FAST_CLIENT", config.database.fastClient);
    config.database.charset = readEnv("POSTGRES_CHARSET", config.database.charset);
    config.database.timeout = readDouble("POSTGRES_TIMEOUT", config.database.timeout);
    config.database.autoBatch = readBool("POSTGRES_AUTO_BATCH", config.database.autoBatch);
    config.database.sslMode = readEnv("POSTGRES_SSLMODE", config.database.sslMode);

    config.cors.allowedOrigin = readEnv("CORS_ALLOWED_ORIGIN", config.cors.allowedOrigin);
    config.cors.allowedMethods = readEnv("CORS_ALLOWED_METHODS", config.cors.allowedMethods);
    config.cors.allowedHeaders = readEnv("CORS_ALLOWED_HEADERS", config.cors.allowedHeaders);
    config.cors.maxAge = readEnv("CORS_MAX_AGE", config.cors.maxAge);

    return config;
}

std::string AppConfig::connectionString() const {
    std::ostringstream connection;
    connection << "host='" << escapeConnectionValue(database.host) << "' "
               << "port='" << database.port << "' "
               << "dbname='" << escapeConnectionValue(database.databaseName) << "' "
               << "user='" << escapeConnectionValue(database.username) << "' "
               << "password='" << escapeConnectionValue(database.password) << "' "
               << "client_encoding='" << escapeConnectionValue(database.charset) << "'";

    if (!database.sslMode.empty()) {
        connection << " sslmode='" << escapeConnectionValue(database.sslMode) << "'";
    }

    return connection.str();
}
