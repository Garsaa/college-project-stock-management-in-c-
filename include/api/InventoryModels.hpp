#ifndef API_INVENTORY_MODELS_HPP
#define API_INVENTORY_MODELS_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct InventoryMovementDto {
    std::int64_t id{};
    std::string operation;
    int quantity{};
    std::string note;
    std::string createdAt;
};

struct InventoryMovementHistoryDto {
    std::int64_t id{};
    std::int64_t itemId{};
    std::string itemCode;
    std::string itemType;
    std::string itemName;
    std::string operation;
    int quantity{};
    std::string note;
    std::string createdAt;
};

struct InventoryItemSummaryDto {
    std::int64_t id{};
    std::string code;
    std::string itemType;
    std::string name;
    int quantity{};
    double unitPrice{};
    double totalValue{};
    std::string location;
    std::string updatedAt;
};

struct InventoryItemDetailsDto {
    std::int64_t id{};
    std::string code;
    std::string itemType;
    std::string name;
    std::string description;
    std::string informationLink;
    int quantity{};
    double unitPrice{};
    double totalValue{};
    std::string location;
    std::string createdAt;
    std::string updatedAt;
    std::vector<InventoryMovementDto> movements;
};

struct CreateItemDto {
    std::string code;
    std::string itemType;
    std::string name;
    std::string description;
    std::string informationLink;
    int quantity{};
    double unitPrice{};
    std::string location;
};

struct UpdateItemDto {
    std::optional<std::string> itemType;
    std::optional<std::string> name;
    std::optional<std::string> description;
    std::optional<std::string> informationLink;
    std::optional<double> unitPrice;
    std::optional<std::string> location;
};

struct StockChangeRequestDto {
    int quantity{};
    std::string note;
};

#endif
