/**
 * @file shop.cpp
 * @brief Shop item management
 */

#include "shop.hpp"

Shop::Shop() {
    InitializeItems();
}

void Shop::InitializeItems() {
    items.clear();

    // WEAPONS (firing patterns) - one-time purchase
    items.push_back({"Dual Shot", "Fire two bullets simultaneously", 100, ItemCategory::WEAPON, false, false, 0, 1, 0, 0});
    items.push_back({"Tri Shot", "Fire three bullets in a spread", 300, ItemCategory::WEAPON, false, false, 0, 1, 0, 0});

    // BULLETS - Unlockable bullet types (one-time purchase)
    items.push_back({"Unlock Explosive", "Unlock explosive bullets", 300, ItemCategory::BULLET, false, false, 0, 1, 0, 0});
    items.push_back({"Unlock Piercing", "Unlock piercing bullets", 400, ItemCategory::BULLET, false, false, 0, 1, 0, 0});

    // Basic bullet upgrades (stackable, max 5)
    items.push_back({"Basic Damage +1", "Increase basic bullet damage", 50, ItemCategory::BULLET, false, true, 0, 5, 0, 0});
    items.push_back({"Basic Speed +1", "Increase basic bullet speed", 60, ItemCategory::BULLET, false, true, 0, 5, 0, 0});

    // Explosive bullet upgrades (stackable, max 5)
    items.push_back({"Explosive Damage +1", "Increase explosive bullet damage", 60, ItemCategory::BULLET, false, true, 0, 5, 0, 0});
    items.push_back({"Explosive Radius +1", "Increase blast radius", 60, ItemCategory::BULLET, false, true, 0, 5, 0, 0});

    // Piercing bullet upgrades (stackable, max 5)
    items.push_back({"Piercing Damage +1", "Increase piercing bullet damage", 50, ItemCategory::BULLET, false, true, 0, 5, 0, 0});
    items.push_back({"Piercing Penetra. +1", "Increase penetration count", 60, ItemCategory::BULLET, false, true, 0, 5, 0, 0});

    // ITEMS (consumables - limited quantity, max 3 each)
    items.push_back({"Speed Boost", "+50% speed for 10 seconds", 30, ItemCategory::ITEM, false, true, 0, 10, 0, 0});
    items.push_back({"Health Pack", "Restore 1 HP instantly", 40, ItemCategory::ITEM, false, true, 0, 10, 0, 0});
    items.push_back({"Shield Pack", "Shield for 30 seconds", 25, ItemCategory::ITEM, false, true, 0, 10, 0, 0});
    items.push_back({"Damage Boost", "2x damage for 8 seconds", 25, ItemCategory::ITEM, false, true, 0, 10, 0, 0});

    // ABILITIES (one-time purchase, upgradable - max level 3)
    items.push_back({"Shield Barrier", "Create a protective shield", 75, ItemCategory::ABILITY, false, false, 0, 1, 0, 1});
    items.push_back({"Rapid Fire", "Double your fire rate", 120, ItemCategory::ABILITY, false, false, 0, 1, 0, 1});
    // items.push_back({"Time Slow", "Slow down enemies temporarily", 150, ItemCategory::ABILITY, false, false, 0, 1, 0, 3});
    items.push_back({"Freeze", "Freeze all enemies for 5 seconds", 200, ItemCategory::ABILITY, false, false, 0, 1, 0, 1});
}

const std::vector<ShopItem>& Shop::GetAllItems() const {
    return items;
}

std::vector<ShopItem> Shop::GetItemsByCategory(ItemCategory category) const {
    std::vector<ShopItem> result;
    for (const auto& item : items) {
        if (item.category == category) {
            result.push_back(item);
        }
    }
    return result;
}

bool Shop::CanAfford(const ShopItem& item, int cash) const {
    return cash >= item.cost;
}

// Check if item can be purchased (respects limits)
bool Shop::CanPurchase(const ShopItem& item) const {
    if (item.owned && !item.can_stack) {
        // One-time purchase item already owned
        return false;
    }
    if (item.max_quantity > 0 && item.quantity >= item.max_quantity) {
        // Reached max quantity for this item
        return false;
    }
    if (item.max_upgrade_level > 0 && item.upgrade_level >= item.max_upgrade_level) {
        // Reached max upgrade level
        return false;
    }
    return true;
}
bool Shop::PurchaseItemByIndex(int global_index, int& cash) {
    if (global_index < 0 || global_index >= (int)items.size()) return false;
    ShopItem& item = items[global_index];
    if (!CanAfford(item, cash) || !CanPurchase(item)) return false;
    
    cash -= item.cost;
    if (item.can_stack) {
        if (item.quantity < item.max_quantity) item.quantity++;
    } else if (item.max_upgrade_level > 0) {
        item.upgrade_level++;
        item.owned = true;
    } else {
        item.owned = true;
    }
    return true;
}

bool Shop::PurchaseItem(ShopItem& item, int& cash) {
    if (!CanAfford(item, cash) || !CanPurchase(item)) {
        return false;
    }

    cash -= item.cost;

    if (item.can_stack) {
        // Stackable item: increase quantity
        item.quantity++;
    } else if (item.max_upgrade_level > 0) {
        // Upgradeable ability: increase level
        item.upgrade_level++;
        item.owned = true;
    } else {
        // One-time purchase
        item.owned = true;
    }

    return true;
}

void Shop::MarkItemAsOwned(const std::string& item_name) {
    for (auto& item : items) {
        if (item.name == item_name) {
            item.owned = true;
            break;
        }
    }
}

void Shop::Reset() {
    InitializeItems();
}
