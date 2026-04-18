#include "shop.hpp"

Shop::Shop() {
    InitializeItems();
}

void Shop::InitializeItems() {
    items.clear();

    // WEAPONS (firing patterns)
    items.push_back({"Dual Shot", "Fire two bullets simultaneously", 100, ItemCategory::WEAPON, false, false});
    items.push_back({"Tri Shot", "Fire three bullets in a spread", 300, ItemCategory::WEAPON, false, false});

    // BULLETS (bullet types and upgrades)
    // Unlockable bullet types
    items.push_back({"Unlock Explosive", "Unlock explosive bullets (cannot be used with Dual/Tri Shot, Press 2 to activate)", 300, ItemCategory::BULLET, false, false});
    items.push_back({"Unlock Piercing", "Unlock piercing bullets (Press 3 to activate)", 500, ItemCategory::BULLET, false, false});

    // Basic bullet upgrades
    items.push_back({"Basic Damage +1", "Increase basic bullet damage", 100, ItemCategory::BULLET, false, true});
    items.push_back({"Basic Speed +1", "Increase basic bullet speed", 120, ItemCategory::BULLET, false, true});

    // Explosive bullet upgrades (choose one at a time)
    items.push_back({"Explosive Damage +1", "Increase explosive bullet damage", 120, ItemCategory::BULLET, false, true});
    items.push_back({"Explosive Radius +1", "Increase blast radius", 120, ItemCategory::BULLET, false, true});

    // Piercing bullet upgrades (choose one at a time)
    items.push_back({"Piercing Damage +1", "Increase piercing bullet damage", 100, ItemCategory::BULLET, false, true});
    items.push_back({"Piercing Penetra. +1", "Increase penetration count", 120, ItemCategory::BULLET, false, true});

    // ITEMS (one-time use consumables)
    items.push_back({"Speed Boost", "+50% speed for 10 seconds", 30, ItemCategory::ITEM, false, false});
    items.push_back({"Health Pack", "Restore 1 HP instantly", 40, ItemCategory::ITEM, false, false});
    items.push_back({"Shield Pack", "Shield for 30 seconds", 50, ItemCategory::ITEM, false, false});
    items.push_back({"Damage Boost", "2x damage for 8 seconds", 45, ItemCategory::ITEM, false, false});

    // ABILITIES
    items.push_back({"Shield Barrier", "Create a protective shield", 75, ItemCategory::ABILITY, false, false});
    items.push_back({"Rapid Fire", "Double your fire rate", 120, ItemCategory::ABILITY, false, false});
    items.push_back({"Time Slow", "Slow down enemies temporarily", 150, ItemCategory::ABILITY, false, false});
    items.push_back({"Freeze", "Freeze all enemies for 5 seconds", 180, ItemCategory::ABILITY, false, false});
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

bool Shop::PurchaseItem(ShopItem& item, int& cash) {
    if (!CanAfford(item, cash)) {
        return false;
    }
    
    cash -= item.cost;
    
    // Only mark as owned if not stackable (stackable items stay available)
    if (!item.can_stack) {
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
