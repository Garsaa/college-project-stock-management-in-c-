#include "api/AppConfig.hpp"
#include "api/InventoryService.hpp"
#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <drogon/drogon.h>

namespace {

std::size_t resolvedThreadCount(const AppConfig& config) {
    return config.threads > 0 ? config.threads
                              : static_cast<std::size_t>(std::max(1u, std::thread::hardware_concurrency()));
}

std::string displayUrl(const AppConfig& config) {
    const std::string host = config.listenHost == "0.0.0.0" ? "127.0.0.1" : config.listenHost;
    return "http://" + host + ":" + std::to_string(config.listenPort);
}

std::string bindAddress(const AppConfig& config) {
    return config.listenHost + ":" + std::to_string(config.listenPort);
}

std::string sslModeLabel(const AppConfig& config) {
    return config.database.sslMode.empty() ? "disabled" : config.database.sslMode;
}

void printStartupSummary(const AppConfig& config, std::size_t threadCount) {
    const std::vector<std::pair<std::string, std::string>> rows = {
        {"URL", displayUrl(config)},
        {"Bind", bindAddress(config)},
        {"Threads", std::to_string(threadCount)},
        {"DB Host", config.database.host + ":" + std::to_string(config.database.port)},
        {"DB Name", config.database.databaseName},
        {"DB User", config.database.username},
        {"DB Client", config.database.clientName + " (pool=" +
                          std::to_string(config.database.connections) + ")"},
        {"SSL", sslModeLabel(config)},
        {"Charset", config.database.charset},
    };

    std::cout << "\n+--------------------------------------------------------------+\n";
    std::cout << "| Inventory API                                                |\n";
    std::cout << "+--------------------------------------------------------------+\n";
    for (const auto& [label, value] : rows) {
        std::cout << "| " << std::left << std::setw(10) << label << " : " << std::setw(46)
                  << value.substr(0, 46) << "|\n";
    }
    std::cout << "+--------------------------------------------------------------+\n";
    std::cout << "| Tip        : Press Ctrl+C to stop the server                 |\n";
    std::cout << "+--------------------------------------------------------------+\n\n";
}

void logStartupStep(const std::string& message) {
    std::cout << "[startup] " << message << '\n';
}

void applyPostgresRuntimeOptions(const AppConfig& config) {
    if (config.database.sslMode.empty()) {
        return;
    }

#ifdef _WIN32
    _putenv_s("PGSSLMODE", config.database.sslMode.c_str());
#else
    setenv("PGSSLMODE", config.database.sslMode.c_str(), 1);
#endif
}

}

int main() {
    try {
        logStartupStep("Loading application configuration...");
        const auto config = AppConfig::loadFromEnvironment();
        applyPostgresRuntimeOptions(config);
        const auto threadCount = resolvedThreadCount(config);
        printStartupSummary(config, threadCount);

        logStartupStep("Connecting bootstrap database client...");
        auto bootstrapClient = drogon::orm::DbClient::newPgClient(
            config.connectionString(),
            1);

        logStartupStep("Ensuring database schema exists...");
        InventoryService(bootstrapClient).ensureSchemaSync();
        logStartupStep("Schema ready.");

        logStartupStep("Registering HTTP listener...");
        drogon::app().addListener(config.listenHost, config.listenPort);
        logStartupStep("Registering PostgreSQL client...");
        drogon::app().createDbClient(
            "postgresql",
            config.database.host,
            config.database.port,
            config.database.databaseName,
            config.database.username,
            config.database.password,
            config.database.connections,
            "",
            config.database.clientName,
            config.database.fastClient,
            config.database.charset,
            config.database.timeout,
            config.database.autoBatch);
        drogon::app().setThreadNum(static_cast<int>(threadCount));
        drogon::app().setLogLevel(trantor::Logger::kInfo);
        logStartupStep("Starting Drogon event loop...");
        drogon::app().run();
    } catch (const std::exception& error) {
        std::cerr << "Failed to start the API: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
