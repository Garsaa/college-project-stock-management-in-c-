#include "http/InventoryController.hpp"

#include "api/ApiError.hpp"
#include "api/InventoryModels.hpp"
#include "api/InventoryService.hpp"

#include <json/value.h>

#include <optional>
#include <string>

#include <drogon/drogon.h>

namespace {

// Build a service instance backed by the default PostgreSQL client.
InventoryService createService() {
    return InventoryService(drogon::app().getDbClient("default"));
}

drogon::HttpResponsePtr jsonResponse(
    const Json::Value& body,
    drogon::HttpStatusCode status = drogon::k200OK) {
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(status);
    return response;
}

drogon::HttpResponsePtr errorResponse(
    int status,
    const std::string& message) {
    Json::Value json;
    json["error"] = message;
    return jsonResponse(json, static_cast<drogon::HttpStatusCode>(status));
}

std::string requiredStringField(const Json::Value& json, const char* fieldName) {
    if (!json.isMember(fieldName) || !json[fieldName].isString()) {
        throw ApiError(
            400,
            std::string("The '") + fieldName + "' field is required and must be a string.");
    }
    return json[fieldName].asString();
}

std::optional<std::string> optionalStringField(const Json::Value& json, const char* fieldName) {
    if (!json.isMember(fieldName)) {
        return std::nullopt;
    }
    if (!json[fieldName].isString()) {
        throw ApiError(400, std::string("The '") + fieldName + "' field must be a string.");
    }
    return json[fieldName].asString();
}

int requiredIntField(const Json::Value& json, const char* fieldName) {
    if (!json.isMember(fieldName) || !json[fieldName].isInt()) {
        throw ApiError(
            400,
            std::string("The '") + fieldName + "' field is required and must be an integer.");
    }
    return json[fieldName].asInt();
}

std::optional<double> optionalDoubleField(const Json::Value& json, const char* fieldName) {
    if (!json.isMember(fieldName)) {
        return std::nullopt;
    }
    if (!json[fieldName].isNumeric()) {
        throw ApiError(400, std::string("The '") + fieldName + "' field must be numeric.");
    }
    return json[fieldName].asDouble();
}

double requiredDoubleField(const Json::Value& json, const char* fieldName) {
    if (!json.isMember(fieldName) || !json[fieldName].isNumeric()) {
        throw ApiError(
            400,
            std::string("The '") + fieldName + "' field is required and must be numeric.");
    }
    return json[fieldName].asDouble();
}

const Json::Value& requestJsonBody(drogon::HttpRequestPtr req) {
    const auto json = req->getJsonObject();
    if (!json) {
        throw ApiError(400, "Invalid JSON request body.");
    }
    return *json;
}

Json::Value toJson(const InventoryMovementDto& movement) {
    Json::Value json;
    json["id"] = Json::Int64(movement.id);
    json["operation"] = movement.operation;
    json["quantity"] = movement.quantity;
    json["note"] = movement.note;
    json["created_at"] = movement.createdAt;
    return json;
}

Json::Value toJson(const InventoryMovementHistoryDto& movement) {
    Json::Value json;
    json["id"] = Json::Int64(movement.id);
    json["item_id"] = Json::Int64(movement.itemId);
    json["item_code"] = movement.itemCode;
    json["item_type"] = movement.itemType;
    json["item_name"] = movement.itemName;
    json["operation"] = movement.operation;
    json["quantity"] = movement.quantity;
    json["note"] = movement.note;
    json["created_at"] = movement.createdAt;
    return json;
}

Json::Value toJson(const InventoryItemSummaryDto& item) {
    Json::Value json;
    json["id"] = Json::Int64(item.id);
    json["code"] = item.code;
    json["item_type"] = item.itemType;
    json["name"] = item.name;
    json["quantity"] = item.quantity;
    json["unit_price"] = item.unitPrice;
    json["total_value"] = item.totalValue;
    json["location"] = item.location;
    json["updated_at"] = item.updatedAt;
    return json;
}

Json::Value toJson(const InventoryItemDetailsDto& item) {
    Json::Value json;
    json["id"] = Json::Int64(item.id);
    json["code"] = item.code;
    json["item_type"] = item.itemType;
    json["name"] = item.name;
    json["description"] = item.description;
    json["information_link"] = item.informationLink;
    json["quantity"] = item.quantity;
    json["unit_price"] = item.unitPrice;
    json["total_value"] = item.totalValue;
    json["location"] = item.location;
    json["created_at"] = item.createdAt;
    json["updated_at"] = item.updatedAt;

    Json::Value movements(Json::arrayValue);
    for (const auto& movement : item.movements) {
        movements.append(toJson(movement));
    }
    json["movements"] = std::move(movements);
    return json;
}

CreateItemDto readCreateItem(const Json::Value& json) {
    CreateItemDto item;
    item.code = requiredStringField(json, "code");
    item.itemType = requiredStringField(json, "item_type");
    item.name = requiredStringField(json, "name");
    item.description = requiredStringField(json, "description");
    item.informationLink = requiredStringField(json, "information_link");
    item.quantity = requiredIntField(json, "quantity");
    item.unitPrice = requiredDoubleField(json, "unit_price");
    item.location = requiredStringField(json, "location");
    return item;
}

UpdateItemDto readUpdateItem(const Json::Value& json) {
    UpdateItemDto item;
    item.itemType = optionalStringField(json, "item_type");
    item.name = optionalStringField(json, "name");
    item.description = optionalStringField(json, "description");
    item.informationLink = optionalStringField(json, "information_link");
    item.unitPrice = optionalDoubleField(json, "unit_price");
    item.location = optionalStringField(json, "location");
    return item;
}

StockChangeRequestDto readStockChangeRequest(const Json::Value& json) {
    StockChangeRequestDto movement;
    movement.quantity = requiredIntField(json, "quantity");
    movement.note = requiredStringField(json, "note");
    return movement;
}

template <typename Function>
drogon::Task<drogon::HttpResponsePtr> executeWithErrorHandling(Function&& function) {
    // Keep controller actions focused on HTTP flow while centralizing API/database error mapping.
    try {
        co_return co_await function();
    } catch (const ApiError& error) {
        co_return errorResponse(error.statusCode(), error.what());
    } catch (const drogon::orm::DrogonDbException& error) {
        LOG_ERROR << "Database error: " << error.base().what();
        co_return errorResponse(500, "Database access failed.");
    } catch (const std::exception& error) {
        LOG_ERROR << "Internal error: " << error.what();
        co_return errorResponse(500, "Internal server error.");
    }
}

}

// GET /health
// Lightweight health check used to confirm the API process is alive.
drogon::Task<drogon::HttpResponsePtr> InventoryController::health(drogon::HttpRequestPtr req) {
    (void)req;
    Json::Value json;
    json["status"] = "ok";
    json["service"] = "inventory_api";
    co_return jsonResponse(json);
}

// GET /api/movements
// Returns the full stock movement history with item metadata for the ERP history view.
drogon::Task<drogon::HttpResponsePtr> InventoryController::listMovements(drogon::HttpRequestPtr req) {
    (void)req;
    co_return co_await executeWithErrorHandling([]() -> drogon::Task<drogon::HttpResponsePtr> {
        auto movements = co_await createService().listMovements();
        Json::Value json;
        Json::Value movementList(Json::arrayValue);
        for (const auto& movement : movements) {
            movementList.append(toJson(movement));
        }
        json["movements"] = std::move(movementList);
        json["total"] = static_cast<Json::UInt64>(movements.size());
        co_return jsonResponse(json);
    });
}

// GET /api/items
// Lists all items, optionally filtering by the "search" query parameter.
drogon::Task<drogon::HttpResponsePtr> InventoryController::listItems(drogon::HttpRequestPtr req) {
    auto searchTerm = req->getOptionalParameter<std::string>("search");
    co_return co_await executeWithErrorHandling(
        [searchTerm]() -> drogon::Task<drogon::HttpResponsePtr> {
            auto items = co_await createService().listItems(searchTerm);
            Json::Value json;
            Json::Value itemList(Json::arrayValue);
            for (const auto& item : items) {
                itemList.append(toJson(item));
            }
            json["items"] = std::move(itemList);
            json["total"] = static_cast<Json::UInt64>(items.size());
            co_return jsonResponse(json);
        });
}

// GET /api/items/{code}
// Returns one item with full details plus its movement history.
drogon::Task<drogon::HttpResponsePtr> InventoryController::getItem(
    drogon::HttpRequestPtr req,
    std::string code) {
    (void)req;
    co_return co_await executeWithErrorHandling(
        [code = std::move(code)]() -> drogon::Task<drogon::HttpResponsePtr> {
            auto item = co_await createService().getItemByCode(code);
            Json::Value json;
            json["item"] = toJson(item);
            co_return jsonResponse(json);
        });
}

// POST /api/items
// Validates the JSON body, creates the item, and returns the created record.
drogon::Task<drogon::HttpResponsePtr> InventoryController::createItem(drogon::HttpRequestPtr req) {
    co_return co_await executeWithErrorHandling([req]() -> drogon::Task<drogon::HttpResponsePtr> {
        const auto item = readCreateItem(requestJsonBody(req));
        auto createdItem = co_await createService().createItem(item);
        Json::Value json;
        json["message"] = "Item created successfully.";
        json["item"] = toJson(createdItem);
        co_return jsonResponse(json, drogon::k201Created);
    });
}

// PATCH /api/items/{code}
// Applies partial updates to an existing item's descriptive fields.
drogon::Task<drogon::HttpResponsePtr> InventoryController::updateItem(
    drogon::HttpRequestPtr req,
    std::string code) {
    co_return co_await executeWithErrorHandling(
        [req, code = std::move(code)]() -> drogon::Task<drogon::HttpResponsePtr> {
            const auto item = readUpdateItem(requestJsonBody(req));
            auto updatedItem = co_await createService().updateItem(code, item);
            Json::Value json;
            json["message"] = "Item updated successfully.";
            json["item"] = toJson(updatedItem);
            co_return jsonResponse(json);
        });
}

// DELETE /api/items/{code}
// Removes the item and its related movement history from the database.
drogon::Task<drogon::HttpResponsePtr> InventoryController::deleteItem(
    drogon::HttpRequestPtr req,
    std::string code) {
    (void)req;
    co_return co_await executeWithErrorHandling(
        [code = std::move(code)]() -> drogon::Task<drogon::HttpResponsePtr> {
            co_await createService().deleteItem(code);
            Json::Value json;
            json["message"] = "Item deleted successfully.";
            co_return jsonResponse(json);
        });
}

// POST /api/items/{code}/inbound
// Adds stock quantity and records the inbound movement in the history table.
drogon::Task<drogon::HttpResponsePtr> InventoryController::registerInbound(
    drogon::HttpRequestPtr req,
    std::string code) {
    co_return co_await executeWithErrorHandling(
        [req, code = std::move(code)]() -> drogon::Task<drogon::HttpResponsePtr> {
            const auto movement = readStockChangeRequest(requestJsonBody(req));
            auto updatedItem = co_await createService().registerInbound(code, movement);
            Json::Value json;
            json["message"] = "Inbound stock registered successfully.";
            json["item"] = toJson(updatedItem);
            co_return jsonResponse(json);
        });
}

// POST /api/items/{code}/outbound
// Removes stock quantity when available and records the outbound movement.
drogon::Task<drogon::HttpResponsePtr> InventoryController::registerOutbound(
    drogon::HttpRequestPtr req,
    std::string code) {
    co_return co_await executeWithErrorHandling(
        [req, code = std::move(code)]() -> drogon::Task<drogon::HttpResponsePtr> {
            const auto movement = readStockChangeRequest(requestJsonBody(req));
            auto updatedItem = co_await createService().registerOutbound(code, movement);
            Json::Value json;
            json["message"] = "Outbound stock registered successfully.";
            json["item"] = toJson(updatedItem);
            co_return jsonResponse(json);
        });
}
