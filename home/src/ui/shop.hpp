#pragma once
#include <string>
#include <vector>

enum class ItemCategory {
    WEAPON,
    BULLET,
    ITEM,
    ABILITY
};

struct ShopItem {
    std::string name;
    std::string description;
    int cost;
    ItemCategory category;
    bool owned;           // For one-time purchases (abilities)
    bool can_stack;      // For stackable items (upgrades)
    int quantity;        // How many owned (for consumables)
    int max_quantity;    // Maximum allowed (0 = unlimited for upgrades, >0 = limit)
    int upgrade_level;   // Current upgrade level (for abilities)
    int max_upgrade_level; // Maximum upgrade level (0 = no upgrades)
};

class Shop {
public:
    Shop();

    // Get all shop items
    const std::vector<ShopItem>& GetAllItems() const;
    
    // Get items by category
    std::vector<ShopItem> GetItemsByCategory(ItemCategory category) const;
    
    // Check if player can afford item
    bool CanAfford(const ShopItem& item, int cash) const;

    // Check if item can be purchased (respects limits)
    bool CanPurchase(const ShopItem& item) const;

    bool PurchaseItemByIndex(int global_index, int& cash);

    // Purchase an item
    bool PurchaseItem(ShopItem& item, int& cash);
    
    // Mark an item as owned (used when loading game state)
    void MarkItemAsOwned(const std::string& item_name);
    
    // Reset shop state
    void Reset();

private:
    std::vector<ShopItem> items;
    
    void InitializeItems();
};
