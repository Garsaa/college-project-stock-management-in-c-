#ifndef API_ITEM_TYPE_PROFILE_HPP
#define API_ITEM_TYPE_PROFILE_HPP

#include <memory>
#include <string>

class ItemTypeProfile {
public:
    virtual ~ItemTypeProfile() = default;

    virtual std::string storageValue() const = 0;
    virtual std::string displayLabel() const = 0;
};

class ProductItemTypeProfile final : public ItemTypeProfile {
public:
    std::string storageValue() const override;
    std::string displayLabel() const override;
};

class MaterialItemTypeProfile final : public ItemTypeProfile {
public:
    std::string storageValue() const override;
    std::string displayLabel() const override;
};

std::unique_ptr<ItemTypeProfile> makeItemTypeProfile(const std::string& rawItemType);

#endif
