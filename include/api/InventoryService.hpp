#ifndef API_INVENTORY_SERVICE_HPP
#define API_INVENTORY_SERVICE_HPP

#include "api/InventoryModels.hpp"

#include <optional>
#include <string>
#include <vector>

#include <drogon/orm/DbClient.h>
#include <drogon/utils/coroutine.h>

class InventoryService {
public:
    explicit InventoryService(drogon::orm::DbClientPtr dbClient);

    void ensureSchemaSync() const;
    drogon::Task<std::vector<InventoryMovementHistoryDto>> listMovements() const;
    drogon::Task<std::vector<InventoryItemSummaryDto>> listItems(
        const std::optional<std::string>& searchTerm) const;
    drogon::Task<InventoryItemDetailsDto> getItemByCode(const std::string& code) const;
    drogon::Task<InventoryItemDetailsDto> createItem(const CreateItemDto& item) const;
    drogon::Task<InventoryItemDetailsDto> updateItem(
        const std::string& code,
        const UpdateItemDto& item) const;
    drogon::Task<InventoryItemDetailsDto> registerInbound(
        const std::string& code,
        const StockChangeRequestDto& movement) const;
    drogon::Task<InventoryItemDetailsDto> registerOutbound(
        const std::string& code,
        const StockChangeRequestDto& movement) const;
    drogon::Task<> deleteItem(const std::string& code) const;

private:
    drogon::Task<InventoryItemDetailsDto> loadItemDetailsByCode(
        const std::string& code,
        const drogon::orm::DbClientPtr& executor) const;

    drogon::orm::DbClientPtr dbClient_;
};

#endif
