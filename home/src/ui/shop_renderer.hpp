#pragma once
#include "ftxui/dom/elements.hpp"
#include "../core/game.hpp"
#include "shop.hpp"

class ShopRenderer {
public:
    ShopRenderer();

    // Render the shop UI
    ftxui::Element Render(Game& game, Shop& shop, ItemCategory category,
                        int selected_item,
                        const std::vector<ShopItem>& items);

    // Render shop only (used for paused state)
    ftxui::Element RenderShopOnly(Game& game, Shop& shop, ItemCategory category,
                              int selected_item,
                              const std::vector<ShopItem>& items,
                              bool in_upgrade_submenu,
                              int upgrade_submenu_selected,
                              const std::vector<std::string>& upgrade_options,
                              const std::string& current_upgrade_item);

private:
    std::string GetWeaponName(const std::string& weapon_type);
    std::string GetBulletName(const std::string& bullet_type);
};