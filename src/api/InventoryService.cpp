#include "api/InventoryService.hpp"

#include "api/ApiError.hpp"
#include "api/ItemTypeProfile.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

using drogon::orm::DbClientPtr;
using drogon::orm::DrogonDbException;
using drogon::orm::Result;
using drogon::orm::Transaction;

namespace {

constexpr const char* ITEM_TIMESTAMP_SQL =
    "to_char(created_at at time zone 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') as created_at, "
    "to_char(updated_at at time zone 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') as updated_at";

constexpr const char* MOVEMENT_TIMESTAMP_SQL =
    "to_char(created_at at time zone 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') as created_at";

std::string trim(std::string text) {
    const auto firstNonWhitespace = text.find_first_not_of(" \t\r\n");
    if (firstNonWhitespace == std::string::npos) {
        return "";
    }

    const auto lastNonWhitespace = text.find_last_not_of(" \t\r\n");
    return text.substr(firstNonWhitespace, lastNonWhitespace - firstNonWhitespace + 1);
}

std::filesystem::path resolveSchemaPath() {
    const auto currentPath = std::filesystem::current_path();
    const std::vector<std::filesystem::path> candidates = {
        currentPath / "sql" / "schema.sql",
        currentPath.parent_path() / "sql" / "schema.sql",
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            return path;
        }
    }

    throw std::runtime_error("Could not locate sql/schema.sql from the current working directory.");
}

std::string readTextFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open schema file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> splitSqlStatements(const std::string& sql) {
    std::vector<std::string> statements;
    std::string current;
    current.reserve(sql.size());

    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool inLineComment = false;
    bool inBlockComment = false;

    for (std::size_t i = 0; i < sql.size(); ++i) {
        const char ch = sql[i];
        const char next = i + 1 < sql.size() ? sql[i + 1] : '\0';

        if (inLineComment) {
            if (ch == '\n') {
                inLineComment = false;
                current.push_back(ch);
            }
            continue;
        }

        if (inBlockComment) {
            if (ch == '*' && next == '/') {
                inBlockComment = false;
                ++i;
            }
            continue;
        }

        if (!inSingleQuote && !inDoubleQuote) {
            if (ch == '-' && next == '-') {
                inLineComment = true;
                ++i;
                continue;
            }
            if (ch == '/' && next == '*') {
                inBlockComment = true;
                ++i;
                continue;
            }
        }

        if (ch == '\'' && !inDoubleQuote) {
            current.push_back(ch);
            if (inSingleQuote && next == '\'') {
                current.push_back(next);
                ++i;
            } else {
                inSingleQuote = !inSingleQuote;
            }
            continue;
        }

        if (ch == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
            current.push_back(ch);
            continue;
        }

        if (ch == ';' && !inSingleQuote && !inDoubleQuote) {
            auto statement = trim(current);
            if (!statement.empty()) {
                statements.push_back(std::move(statement));
            }
            current.clear();
            continue;
        }

        current.push_back(ch);
    }

    auto trailingStatement = trim(current);
    if (!trailingStatement.empty()) {
        statements.push_back(std::move(trailingStatement));
    }

    return statements;
}

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

std::string requireText(const std::string& value, const char* fieldName) {
    const auto text = trim(value);
    if (text.empty()) {
        throw ApiError(400, std::string("The '") + fieldName + "' field is required.");
    }
    return text;
}

std::string normalizeItemType(const std::string& value) {
    // This keeps a small but real polymorphic hierarchy in the backend domain:
    // each item type resolves to its own profile implementation at runtime.
    return makeItemTypeProfile(requireText(value, "item_type"))->storageValue();
}

void validateUnitPrice(double unitPrice) {
    if (unitPrice < 0.0) {
        throw ApiError(400, "The 'unit_price' field cannot be negative.");
    }
}

void validateQuantity(int quantity, const char* fieldName, bool allowZero) {
    const int minimum = allowZero ? 0 : 1;
    if (quantity < minimum) {
        throw ApiError(
            400,
            std::string("The '") + fieldName + "' field must be greater than or equal to " +
                std::to_string(minimum) + ".");
    }
}

void validateCreateItem(const CreateItemDto& item) {
    requireText(item.code, "code");
    normalizeItemType(item.itemType);
    requireText(item.name, "name");
    requireText(item.description, "description");
    requireText(item.informationLink, "information_link");
    validateQuantity(item.quantity, "quantity", true);
    validateUnitPrice(item.unitPrice);
    requireText(item.location, "location");
}

void validateMovement(const StockChangeRequestDto& movement) {
    validateQuantity(movement.quantity, "quantity", false);
    requireText(movement.note, "note");
}

bool looksLikeDuplicateError(const DrogonDbException& error) {
    const std::string message = toLower(error.base().what());
    return message.find("duplicate key") != std::string::npos ||
           message.find("unique constraint") != std::string::npos;
}

InventoryItemSummaryDto mapSummaryRow(const drogon::orm::Row& row) {
    InventoryItemSummaryDto item;
    const auto itemType = makeItemTypeProfile(row["item_type"].as<std::string>());
    item.id = row["id"].as<std::int64_t>();
    item.code = row["code"].as<std::string>();
    item.itemType = itemType->storageValue();
    item.name = row["name"].as<std::string>();
    item.quantity = row["quantity"].as<int>();
    item.unitPrice = row["unit_price"].as<double>();
    item.totalValue = row["total_value"].as<double>();
    item.location = row["location"].as<std::string>();
    item.updatedAt = row["updated_at"].as<std::string>();
    return item;
}

InventoryItemDetailsDto mapDetailsRow(const drogon::orm::Row& row) {
    InventoryItemDetailsDto item;
    const auto itemType = makeItemTypeProfile(row["item_type"].as<std::string>());
    item.id = row["id"].as<std::int64_t>();
    item.code = row["code"].as<std::string>();
    item.itemType = itemType->storageValue();
    item.name = row["name"].as<std::string>();
    item.description = row["description"].as<std::string>();
    item.informationLink = row["information_link"].as<std::string>();
    item.quantity = row["quantity"].as<int>();
    item.unitPrice = row["unit_price"].as<double>();
    item.totalValue = row["total_value"].as<double>();
    item.location = row["location"].as<std::string>();
    item.createdAt = row["created_at"].as<std::string>();
    item.updatedAt = row["updated_at"].as<std::string>();
    return item;
}

InventoryMovementDto mapMovementRow(const drogon::orm::Row& row) {
    InventoryMovementDto movement;
    movement.id = row["id"].as<std::int64_t>();
    movement.operation = row["operation"].as<std::string>();
    movement.quantity = row["quantity"].as<int>();
    movement.note = row["note"].as<std::string>();
    movement.createdAt = row["created_at"].as<std::string>();
    return movement;
}

InventoryMovementHistoryDto mapMovementHistoryRow(const drogon::orm::Row& row) {
    InventoryMovementHistoryDto movement;
    const auto itemType = makeItemTypeProfile(row["item_type"].as<std::string>());
    movement.id = row["id"].as<std::int64_t>();
    movement.itemId = row["item_id"].as<std::int64_t>();
    movement.itemCode = row["item_code"].as<std::string>();
    movement.itemType = itemType->storageValue();
    movement.itemName = row["item_name"].as<std::string>();
    movement.operation = row["operation"].as<std::string>();
    movement.quantity = row["quantity"].as<int>();
    movement.note = row["note"].as<std::string>();
    movement.createdAt = row["created_at"].as<std::string>();
    return movement;
}

CreateItemDto buildUpdatedItem(
    const InventoryItemDetailsDto& currentItem,
    const UpdateItemDto& patch) {
    CreateItemDto item;
    item.code = currentItem.code;
    item.itemType = patch.itemType.value_or(currentItem.itemType);
    item.name = patch.name.value_or(currentItem.name);
    item.description = patch.description.value_or(currentItem.description);
    item.informationLink = patch.informationLink.value_or(currentItem.informationLink);
    item.quantity = currentItem.quantity;
    item.unitPrice = patch.unitPrice.value_or(currentItem.unitPrice);
    item.location = patch.location.value_or(currentItem.location);
    return item;
}

}

InventoryService::InventoryService(DbClientPtr dbClient)
    : dbClient_(std::move(dbClient)) {}

void InventoryService::ensureSchemaSync() const {
    const auto schemaSql = trim(readTextFile(resolveSchemaPath()));
    if (schemaSql.empty()) {
        throw std::runtime_error("The schema file is empty.");
    }

    const auto statements = splitSqlStatements(schemaSql);
    if (statements.empty()) {
        throw std::runtime_error("No SQL statements were found in the schema file.");
    }

    for (const auto& statement : statements) {
        dbClient_->execSqlSync(statement);
    }
}

drogon::Task<std::vector<InventoryMovementHistoryDto>> InventoryService::listMovements() const {
    auto result = co_await dbClient_->execSqlCoro(
        "select m.id, m.item_id, i.code as item_code, i.item_type, i.name as item_name, "
        "m.operation, m.quantity, m.note, "
        "to_char(m.created_at at time zone 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') as created_at "
        "from estoque_inventory_movements m "
        "inner join estoque_items i on i.id = m.item_id "
        "order by m.id desc");

    std::vector<InventoryMovementHistoryDto> movements;
    movements.reserve(result.size());
    for (const auto& row : result) {
        movements.push_back(mapMovementHistoryRow(row));
    }

    co_return movements;
}

drogon::Task<std::vector<InventoryItemSummaryDto>> InventoryService::listItems(
    const std::optional<std::string>& searchTerm) const {
    Result result(nullptr);
    const auto normalizedSearchTerm = searchTerm ? trim(*searchTerm) : "";

    if (normalizedSearchTerm.empty()) {
        result = co_await dbClient_->execSqlCoro(
            "select id, code, item_type, name, quantity, unit_price, "
            "(quantity * unit_price) as total_value, location, "
            "to_char(updated_at at time zone 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') as updated_at "
            "from estoque_items order by name asc, code asc");
    } else {
        const auto filter = "%" + normalizedSearchTerm + "%";
        result = co_await dbClient_->execSqlCoro(
            "select id, code, item_type, name, quantity, unit_price, "
            "(quantity * unit_price) as total_value, location, "
            "to_char(updated_at at time zone 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') as updated_at "
            "from estoque_items "
            "where code ilike $1 or name ilike $1 "
            "order by name asc, code asc",
            filter);
    }

    std::vector<InventoryItemSummaryDto> items;
    items.reserve(result.size());
    for (const auto& row : result) {
        items.push_back(mapSummaryRow(row));
    }
    co_return items;
}

drogon::Task<InventoryItemDetailsDto> InventoryService::getItemByCode(const std::string& code) const {
    co_return co_await loadItemDetailsByCode(
        requireText(code, "code"),
        dbClient_);
}

drogon::Task<InventoryItemDetailsDto> InventoryService::createItem(const CreateItemDto& item) const {
    validateCreateItem(item);

    std::shared_ptr<Transaction> trans;
    try {
        trans = co_await dbClient_->newTransactionCoro();

        const auto code = requireText(item.code, "code");
        const auto itemType = normalizeItemType(item.itemType);
        const auto name = requireText(item.name, "name");
        const auto description = requireText(item.description, "description");
        const auto informationLink = requireText(item.informationLink, "information_link");
        const auto location = requireText(item.location, "location");

        auto existingItem = co_await trans->execSqlCoro(
            "select 1 from estoque_items where code = $1",
            code);
        if (!existingItem.empty()) {
            throw ApiError(409, "An item with this code already exists.");
        }

        auto insertion = co_await trans->execSqlCoro(
            "insert into estoque_items (code, item_type, name, description, information_link, quantity, "
            "unit_price, location, updated_at) "
            "values ($1, $2, $3, $4, $5, $6, $7, $8, now()) "
            "returning id",
            code,
            itemType,
            name,
            description,
            informationLink,
            item.quantity,
            item.unitPrice,
            location);

        const auto itemId = insertion[0]["id"].as<std::int64_t>();

        if (item.quantity > 0) {
            co_await trans->execSqlCoro(
                "insert into estoque_inventory_movements (item_id, operation, quantity, note) "
                "values ($1, 'inbound', $2, $3)",
                itemId,
                item.quantity,
                std::string("Initial registration."));
        }

        co_return co_await loadItemDetailsByCode(code, trans);
    } catch (const DrogonDbException& error) {
        if (trans) {
            trans->rollback();
        }
        if (looksLikeDuplicateError(error)) {
            throw ApiError(409, "An item with this code already exists.");
        }
        throw;
    } catch (...) {
        if (trans) {
            trans->rollback();
        }
        throw;
    }
}

drogon::Task<InventoryItemDetailsDto> InventoryService::updateItem(
    const std::string& code,
    const UpdateItemDto& patch) const {
    if (!patch.itemType && !patch.name && !patch.description && !patch.informationLink &&
        !patch.unitPrice && !patch.location) {
        throw ApiError(400, "Provide at least one field to update.");
    }

    std::shared_ptr<Transaction> trans;
    try {
        trans = co_await dbClient_->newTransactionCoro();

        const auto normalizedCode = requireText(code, "code");
        const auto currentItem = co_await loadItemDetailsByCode(normalizedCode, trans);
        const auto updatedItem = buildUpdatedItem(currentItem, patch);
        validateCreateItem(updatedItem);

        co_await trans->execSqlCoro(
            "update estoque_items set item_type = $2, name = $3, description = $4, "
            "information_link = $5, unit_price = $6, location = $7, updated_at = now() "
            "where code = $1",
            normalizedCode,
            normalizeItemType(updatedItem.itemType),
            requireText(updatedItem.name, "name"),
            requireText(updatedItem.description, "description"),
            requireText(updatedItem.informationLink, "information_link"),
            updatedItem.unitPrice,
            requireText(updatedItem.location, "location"));

        co_return co_await loadItemDetailsByCode(normalizedCode, trans);
    } catch (...) {
        if (trans) {
            trans->rollback();
        }
        throw;
    }
}

drogon::Task<InventoryItemDetailsDto> InventoryService::registerInbound(
    const std::string& code,
    const StockChangeRequestDto& movement) const {
    validateMovement(movement);

    std::shared_ptr<Transaction> trans;
    try {
        trans = co_await dbClient_->newTransactionCoro();

        const auto normalizedCode = requireText(code, "code");
        auto lockedRows = co_await trans->execSqlCoro(
            "select id from estoque_items where code = $1 for update",
            normalizedCode);

        if (lockedRows.empty()) {
            throw ApiError(404, "Item not found.");
        }

        const auto itemId = lockedRows[0]["id"].as<std::int64_t>();

        co_await trans->execSqlCoro(
            "update estoque_items set quantity = quantity + $2, updated_at = now() "
            "where code = $1",
            normalizedCode,
            movement.quantity);

        co_await trans->execSqlCoro(
            "insert into estoque_inventory_movements (item_id, operation, quantity, note) "
            "values ($1, 'inbound', $2, $3)",
            itemId,
            movement.quantity,
            requireText(movement.note, "note"));

        co_return co_await loadItemDetailsByCode(normalizedCode, trans);
    } catch (...) {
        if (trans) {
            trans->rollback();
        }
        throw;
    }
}

drogon::Task<InventoryItemDetailsDto> InventoryService::registerOutbound(
    const std::string& code,
    const StockChangeRequestDto& movement) const {
    validateMovement(movement);

    std::shared_ptr<Transaction> trans;
    try {
        trans = co_await dbClient_->newTransactionCoro();

        const auto normalizedCode = requireText(code, "code");
        auto lockedRows = co_await trans->execSqlCoro(
            "select id, quantity from estoque_items where code = $1 for update",
            normalizedCode);

        if (lockedRows.empty()) {
            throw ApiError(404, "Item not found.");
        }

        const auto itemId = lockedRows[0]["id"].as<std::int64_t>();
        const auto currentQuantity = lockedRows[0]["quantity"].as<int>();
        if (movement.quantity > currentQuantity) {
            throw ApiError(409, "There is not enough quantity in stock.");
        }

        co_await trans->execSqlCoro(
            "update estoque_items set quantity = quantity - $2, updated_at = now() "
            "where code = $1",
            normalizedCode,
            movement.quantity);

        co_await trans->execSqlCoro(
            "insert into estoque_inventory_movements (item_id, operation, quantity, note) "
            "values ($1, 'outbound', $2, $3)",
            itemId,
            movement.quantity,
            requireText(movement.note, "note"));

        co_return co_await loadItemDetailsByCode(normalizedCode, trans);
    } catch (...) {
        if (trans) {
            trans->rollback();
        }
        throw;
    }
}

drogon::Task<> InventoryService::deleteItem(const std::string& code) const {
    const auto normalizedCode = requireText(code, "code");
    const auto result = co_await dbClient_->execSqlCoro(
        "delete from estoque_items where code = $1",
        normalizedCode);

    if (result.affectedRows() == 0) {
        throw ApiError(404, "Item not found.");
    }
}

drogon::Task<InventoryItemDetailsDto> InventoryService::loadItemDetailsByCode(
    const std::string& code,
    const DbClientPtr& executor) const {
    auto itemRows = co_await executor->execSqlCoro(
        "select id, code, item_type, name, description, information_link, quantity, "
        "unit_price, (quantity * unit_price) as total_value, location, "
        + std::string(ITEM_TIMESTAMP_SQL) +
        " from estoque_items where code = $1",
        code);

    if (itemRows.empty()) {
        throw ApiError(404, "Item not found.");
    }

    auto item = mapDetailsRow(itemRows[0]);
    auto movementRows = co_await executor->execSqlCoro(
        "select id, operation, quantity, note, "
        + std::string(MOVEMENT_TIMESTAMP_SQL) +
        " from estoque_inventory_movements where item_id = $1 order by id asc",
        item.id);

    item.movements.reserve(movementRows.size());
    for (const auto& row : movementRows) {
        item.movements.push_back(mapMovementRow(row));
    }

    co_return item;
}
