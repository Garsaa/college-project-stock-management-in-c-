#include "api/ItemTypeProfile.hpp"

#include "api/ApiError.hpp"

#include <algorithm>
#include <cctype>

namespace {

std::string normalizeRawItemType(std::string rawItemType) {
    std::transform(rawItemType.begin(), rawItemType.end(), rawItemType.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return rawItemType;
}

}

std::string ProductItemTypeProfile::storageValue() const {
    return "product";
}

std::string ProductItemTypeProfile::displayLabel() const {
    return "Product";
}

std::string MaterialItemTypeProfile::storageValue() const {
    return "material";
}

std::string MaterialItemTypeProfile::displayLabel() const {
    return "Material";
}

std::unique_ptr<ItemTypeProfile> makeItemTypeProfile(const std::string& rawItemType) {
    const auto normalizedType = normalizeRawItemType(rawItemType);

    if (normalizedType == "product") {
        return std::make_unique<ProductItemTypeProfile>();
    }

    if (normalizedType == "material") {
        return std::make_unique<MaterialItemTypeProfile>();
    }

    throw ApiError(400, "The 'item_type' field must be 'product' or 'material'.");
}
