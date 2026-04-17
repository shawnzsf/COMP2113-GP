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
    bool owned;
    bool can_stack;  // Items like upgrades that can be purchased multiple times
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
