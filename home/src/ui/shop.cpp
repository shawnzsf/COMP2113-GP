#include "shop.hpp"

Shop::Shop() {
    InitializeItems();
}

void Shop::InitializeItems() {
    items.clear();
    
    // WEAPONS
    items.push_back({"Dual Shot", "Fire two bullets simultaneously", 50, ItemCategory::WEAPON, false, false});
    items.push_back({"Tri Shot", "Fire three bullets in a spread", 100, ItemCategory::WEAPON, false, false});
    items.push_back({"Explosive Orb", "Fire explosive projectiles", 150, ItemCategory::WEAPON, false, false});
    
    // UPGRADES
    items.push_back({"Fire Rate +10%", "Increase weapon fire rate", 75, ItemCategory::UPGRADE, false, true});
    items.push_back({"Damage +25%", "Increase bullet damage", 100, ItemCategory::UPGRADE, false, true});
    items.push_back({"Movement Speed +20%", "Move faster across the battlefield", 60, ItemCategory::UPGRADE, false, true});
    items.push_back({"Bullet Speed +15%", "Bullets travel faster", 80, ItemCategory::UPGRADE, false, true});
    items.push_back({"Clip Size +5", "More bullets before reload", 90, ItemCategory::UPGRADE, false, true});
    
    // ABILITIES
    items.push_back({"Shield Barrier", "Create a protective shield", 75, ItemCategory::ABILITY, false, false});
    items.push_back({"Rapid Fire", "Temporary boost to fire rate", 120, ItemCategory::ABILITY, false, false});
    items.push_back({"Time Slow", "Slow down enemies temporarily", 150, ItemCategory::ABILITY, false, false});
    items.push_back({"Health Recovery", "Restore one unit of health", 110, ItemCategory::ABILITY, false, false});
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
