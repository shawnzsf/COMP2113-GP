#include "shop_renderer.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

ShopRenderer::ShopRenderer() {}

std::string ShopRenderer::GetWeaponName(const std::string& weapon_type) {
    if (weapon_type == "BASIC") return "Basic";
    if (weapon_type == "DUAL") return "Dual";
    if (weapon_type == "TRI") return "Tri";
    return "Unknown";
}

std::string ShopRenderer::GetBulletName(const std::string& bullet_type) {
    if (bullet_type == "NORMAL") return "Basic";
    if (bullet_type == "EXPLOSIVE") return "Explosive";
    if (bullet_type == "PIERCING") return "Piercing";
    return "Unknown";
}

Element ShopRenderer::Render(Game& game, Shop& shop, ItemCategory category,
                          int selected_item,
                          const std::vector<ShopItem>& items) {
    Elements shop_lines;
    shop_lines.push_back(text(""));
    shop_lines.push_back(text("╔════════════════════════════ SHOP MENU ════════════════════════════╗") | bold | color(Color::Yellow) | center);
    shop_lines.push_back(text(""));

    // Display cash and current status
    Elements status_line;
    status_line.push_back(text("💰 Cash: $" + std::to_string(game.GetCash())) | color(Color::Green) | bold);
    status_line.push_back(text(" | "));
    status_line.push_back(text("Weapon: " + std::string(
        game.GetWeaponType() == WeaponType::BASIC ? "Basic" :
        game.GetWeaponType() == WeaponType::DUAL ? "Dual" :
        game.GetWeaponType() == WeaponType::TRI ? "Tri" : "Explosive"
    )) | color(Color::Cyan));
    if (game.HasShield()) {
        status_line.push_back(text(" | 🛡 Shield"));
    }
    shop_lines.push_back(hbox(std::move(status_line)) | center);
    shop_lines.push_back(text(""));
    shop_lines.push_back(separator());

    // Display categories
    Elements category_line;
    const char* categories[] = {"⚔ WEAPONS", "💥 BULLETS", "📦 ITEMS", "✨ ABILITIES"};
    ItemCategory category_values[] = {ItemCategory::WEAPON, ItemCategory::BULLET, ItemCategory::ITEM, ItemCategory::ABILITY};

    for (int i = 0; i < 4; ++i) {
        std::string cat_text = categories[i];
        if (category_values[i] == category) {
            category_line.push_back(text(" [" + cat_text + "] ") | bgcolor(Color::Blue) | color(Color::White) | bold);
        } else {
            category_line.push_back(text(" " + cat_text + " ") | color(Color::GrayDark));
        }
        if (i < 3) category_line.push_back(text(" | "));
    }
    shop_lines.push_back(hbox(std::move(category_line)) | center);
    shop_lines.push_back(text(""));
    shop_lines.push_back(separator());

    // Display items in current category
    if (!items.empty()) {
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            std::string prefix = (static_cast<int>(i) == selected_item) ? "→ " : "  ";

            // Determine item status
            std::string status_text;
            Color item_color = Color::White;
            bool is_selectable = !item.owned;

            if (item.owned && !item.can_stack) {
                status_text = " [OWNED - ENTER to use]";
                item_color = Color::Green;
                is_selectable = true;
            } else if (item.owned && item.can_stack) {
                status_text = " [OWNED - ENTER to upgrade]";
                item_color = Color::Cyan;
                is_selectable = shop.CanAfford(item, game.GetCash());
            } else if (item.category == ItemCategory::BULLET && item.can_stack) {
                bool has_prerequisites = true;
                if (item.name == "Explosive Damage +1" || item.name == "Explosive Radius +1") {
                    has_prerequisites = game.HasExplosive();
                } else if (item.name == "Piercing Damage +1" || item.name == "Piercing Penetra. +1") {
                    has_prerequisites = game.HasPiercing();
                }
                if (has_prerequisites && shop.CanAfford(item, game.GetCash())) {
                    status_text = " [$" + std::to_string(item.cost) + "]";
                    item_color = Color::Cyan;
                    is_selectable = true;
                } else if (!has_prerequisites) {
                    status_text = " [REQUIRES UNLOCK]";
                    item_color = Color::Red;
                    is_selectable = false;
                } else {
                    status_text = " [$" + std::to_string(item.cost) + " - UNAVAILABLE]";
                    item_color = Color::Red;
                    is_selectable = false;
                }
            } else if (shop.CanAfford(item, game.GetCash())) {
                status_text = " [$" + std::to_string(item.cost) + "]";
                item_color = item.can_stack ? Color::Cyan : Color::Yellow;
            } else {
                status_text = " [$" + std::to_string(item.cost) + " - UNAVAILABLE]";
                item_color = Color::Red;
                is_selectable = false;
            }

            auto item_element = text(prefix + item.name) | color(item_color);
            if (static_cast<int>(i) == selected_item && is_selectable) {
                item_element = item_element | bold | bgcolor(Color::DarkBlue);
            }
            shop_lines.push_back(item_element);
            shop_lines.push_back(text("     " + item.description + status_text) | color(Color::GrayLight));
        }
    }

    shop_lines.push_back(text(""));
    shop_lines.push_back(text(""));
    shop_lines.push_back(separator());
    shop_lines.push_back(text("←→ Switch category | ↑↓ Select item | Enter to buy/upgrade | P to Resume Game") | color(Color::GrayLight) | center);
    shop_lines.push_back(text(""));

    return vbox(std::move(shop_lines)) | border | color(Color::Green) | bgcolor(Color::RGB(16, 16, 16)) | flex;
}

Element ShopRenderer::RenderShopOnly(Game& game, Shop& shop, ItemCategory category,
                                   int selected_item,
                                   const std::vector<ShopItem>& items,
                                   bool in_upgrade_submenu,
                                   int upgrade_submenu_selected,
                                   const std::vector<std::string>& upgrade_options,
                                   const std::string& current_upgrade_item) {
    Elements shop_lines;
    shop_lines.push_back(text(""));
    shop_lines.push_back(text("╔════════════════════════════ SHOP MENU ════════════════════════════╗") | bold | color(Color::Yellow) | center);
    shop_lines.push_back(text(""));

    // Display cash and current status
    Elements status_line;
    status_line.push_back(text("💰 Cash: $" + std::to_string(game.GetCash())) | color(Color::Green) | bold);
    status_line.push_back(text(" | "));
    status_line.push_back(text("Weapon: " + std::string(
        game.GetWeaponType() == WeaponType::BASIC ? "Basic" :
        game.GetWeaponType() == WeaponType::DUAL ? "Dual" :
        game.GetWeaponType() == WeaponType::TRI ? "Tri" : "Explosive"
    )) | color(Color::Cyan));
    if (game.HasShield()) {
        status_line.push_back(text(" | 🛡 Shield"));
    }
    shop_lines.push_back(hbox(std::move(status_line)) | center);
    shop_lines.push_back(text(""));
    shop_lines.push_back(separator());

    // Display categories
    Elements category_line;
    const char* categories[] = {"⚔ WEAPONS", "💥 BULLETS", "📦 ITEMS", "✨ ABILITIES"};
    ItemCategory category_values[] = {ItemCategory::WEAPON, ItemCategory::BULLET, ItemCategory::ITEM, ItemCategory::ABILITY};

    for (int i = 0; i < 4; ++i) {
        std::string cat_text = categories[i];
        if (category_values[i] == category) {
            category_line.push_back(text(" [" + cat_text + "] ") | bgcolor(Color::Blue) | color(Color::White) | bold);
        } else {
            category_line.push_back(text(" " + cat_text + " ") | color(Color::GrayDark));
        }
        if (i < 3) category_line.push_back(text(" | "));
    }
    shop_lines.push_back(hbox(std::move(category_line)) | center);
    shop_lines.push_back(text(""));
    shop_lines.push_back(separator());

    // If in upgrade submenu, show upgrade options
    if (in_upgrade_submenu && !upgrade_options.empty()) {
        shop_lines.push_back(text("Upgrading: " + current_upgrade_item) | bold | color(Color::Yellow) | center);
        shop_lines.push_back(text(""));
        for (size_t j = 0; j < upgrade_options.size(); ++j) {
            std::string prefix = (static_cast<int>(j) == upgrade_submenu_selected) ? "→ " : "  ";
            auto opt_element = text(prefix + upgrade_options[j]) | color(Color::Cyan);
            if (static_cast<int>(j) == upgrade_submenu_selected) {
                opt_element = opt_element | bold | bgcolor(Color::DarkBlue);
            }
            shop_lines.push_back(opt_element);
        }
        shop_lines.push_back(text(""));
        shop_lines.push_back(text("Press ENTER to select upgrade, ESC to go back") | color(Color::GrayLight) | center);
    } else {
        // Display items in current category
        if (!items.empty()) {
            for (size_t i = 0; i < items.size(); ++i) {
                const auto& item = items[i];
                std::string prefix = (static_cast<int>(i) == selected_item) ? "→ " : "  ";

                std::string status_text;
                Color item_color = Color::White;
                bool is_selectable = !item.owned;

                if (item.owned && !item.can_stack) {
                    status_text = " [OWNED - ENTER to use]";
                    item_color = Color::Green;
                    is_selectable = true;
                } else if (item.owned && item.can_stack) {
                    status_text = " [OWNED - ENTER to upgrade]";
                    item_color = Color::Cyan;
                    is_selectable = shop.CanAfford(item, game.GetCash());
                } else if (item.category == ItemCategory::BULLET && item.can_stack) {
                    bool has_prerequisites = true;
                    if (item.name == "Explosive Damage +1" || item.name == "Explosive Radius +1") {
                        has_prerequisites = game.HasExplosive();
                    } else if (item.name == "Piercing Damage +1" || item.name == "Piercing Penetra. +1") {
                        has_prerequisites = game.HasPiercing();
                    }
                    if (has_prerequisites && shop.CanAfford(item, game.GetCash())) {
                        status_text = " [$" + std::to_string(item.cost) + "]";
                        item_color = Color::Cyan;
                        is_selectable = true;
                    } else if (!has_prerequisites) {
                        status_text = " [REQUIRES UNLOCK]";
                        item_color = Color::Red;
                        is_selectable = false;
                    } else {
                        status_text = " [$" + std::to_string(item.cost) + " - UNAVAILABLE]";
                        item_color = Color::Red;
                        is_selectable = false;
                    }
                } else if (shop.CanAfford(item, game.GetCash())) {
                    status_text = " [$" + std::to_string(item.cost) + "]";
                    item_color = item.can_stack ? Color::Cyan : Color::Yellow;
                } else {
                    status_text = " [$" + std::to_string(item.cost) + " - UNAVAILABLE]";
                    item_color = Color::Red;
                    is_selectable = false;
                }

                auto item_element = text(prefix + item.name) | color(item_color);
                if (static_cast<int>(i) == selected_item && is_selectable) {
                    item_element = item_element | bold | bgcolor(Color::DarkBlue);
                }
                shop_lines.push_back(item_element);
                shop_lines.push_back(text("     " + item.description + status_text) | color(Color::GrayLight));
            }
        }
    }

    shop_lines.push_back(text(""));
    shop_lines.push_back(text(""));
    shop_lines.push_back(separator());
    shop_lines.push_back(text("←→ Switch category | ↑↓ Select item | Enter to buy/upgrade | P to Resume Game") | color(Color::GrayLight) | center);
    shop_lines.push_back(text(""));

    return vbox(std::move(shop_lines)) | border | color(Color::Green) | bgcolor(Color::RGB(16, 16, 16)) | flex;
}