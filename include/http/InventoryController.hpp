#ifndef HTTP_INVENTORY_CONTROLLER_HPP
#define HTTP_INVENTORY_CONTROLLER_HPP

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

class InventoryController : public drogon::HttpController<InventoryController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(InventoryController::health, "/health", drogon::Get);
    ADD_METHOD_TO(InventoryController::listMovements, "/api/movements", drogon::Get);
    ADD_METHOD_TO(InventoryController::listItems, "/api/items", drogon::Get);
    ADD_METHOD_TO(InventoryController::getItem, "/api/items/{1}", drogon::Get);
    ADD_METHOD_TO(InventoryController::createItem, "/api/items", drogon::Post);
    ADD_METHOD_TO(InventoryController::updateItem, "/api/items/{1}", drogon::Patch);
    ADD_METHOD_TO(InventoryController::deleteItem, "/api/items/{1}", drogon::Delete);
    ADD_METHOD_TO(InventoryController::registerInbound, "/api/items/{1}/inbound", drogon::Post);
    ADD_METHOD_TO(InventoryController::registerOutbound, "/api/items/{1}/outbound", drogon::Post);
    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> health(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> listMovements(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> listItems(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> getItem(
        drogon::HttpRequestPtr req,
        std::string code);
    drogon::Task<drogon::HttpResponsePtr> createItem(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> updateItem(
        drogon::HttpRequestPtr req,
        std::string code);
    drogon::Task<drogon::HttpResponsePtr> deleteItem(
        drogon::HttpRequestPtr req,
        std::string code);
    drogon::Task<drogon::HttpResponsePtr> registerInbound(
        drogon::HttpRequestPtr req,
        std::string code);
    drogon::Task<drogon::HttpResponsePtr> registerOutbound(
        drogon::HttpRequestPtr req,
        std::string code);
};

#endif
