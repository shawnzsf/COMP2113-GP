#include "game.hpp"
#include "../data/highscore.hpp"
#include "../ui/hud.hpp"
#include "../ui/shop.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"

#include <atomic>
#include <chrono>
#include <thread>
#include <string>

// Change the bullet icon to unicode
// Add an hud that shows the player's current weapon, score, wave, and cash. Also show cooldowns for abilities and weapon upgrades. The hud should be placed at the top of the screen and should be visually distinct from the gameplay area. This allows players to easily keep track of their status and make informed decisions during gameplay. For example, if the player sees that their current weapon is on cooldown, they might choose to use an ability or switch to a different weapon if they have one available. Additionally, showing the player's cash can help them decide when to visit the shop and what items or upgrades they can afford. Overall, a well-designed hud enhances the player's experience by providing important information at a glance without cluttering the screen.
// The current pasuse menu can also be increased, since a more comprehensive shopping system will be implemented, make the pause menu a full shop menu, where players can buy weapons, upgrades, and abilities. The shop should have a clear layout that categorizes items (e.g., weapons, upgrades, abilities) and displays the player's current cash and the cost of each item. Players should be able to navigate the shop using the arrow keys and select items to purchase with the Enter key. Additionally, there should be an option to exit the shop and resume the game. This allows players to make strategic decisions about their purchases without feeling rushed, as they can take their time to browse through the available options and plan their upgrades based on their current needs in the game.
// add weapon system, switch between different weapons, weapon can be upgraded, fire rate, etc. some amount of money to unlock weapon then upgrade
// Create a bullet system. Remove the explosive weapon, it should belong to the bullet system. We will have three types of bullets: basic, explosive (like the current explosive weapon), and piercing (laser-like, fast and can penetrate enemies). Each bullet type has different behavior and can be upgraded separately. For example, basic bullets can be upgraded to increase damage, explosive bullets can be upgraded to increase blast radius, and piercing bullets can be upgraded to penetrate more enemies. This adds more depth and strategy to the gameplay, as players can choose which bullet type to focus on based on their playstyle and the current wave of enemies. Player should be offered the option to upgrade specific property of the bullet, for example, For explosive bullet, you can choose to upgrade blast radius or damage. For piercing bullet, you can choose to upgrade penetration or speed. This allows for more customization and strategic choices in how players want to enhance their weapons.
// item system, temporarily increase the player's speed, or increase health, etc. One time use, or last for a few seconds. Can also be bought in the shop.
// ability system, abilities should also be bought and upgraded, including stuff like shield, or freeze the enemies
// Improve the spawning system; for each wave, you have a fixed level of toughness (e.g. for wave 1 is 5), and each regular (1)/elite (3)/boss (6) contribute differently to that toughness. For example, earlier waves can only have regular/elite enemies that could contribute to the overall toughness. In later waves boss will also be available for filling the toughness. And the toughness will increase over time. This ensures the waves feel more balanced and challenging as you progress.
// The overall difficulty should increase over time, for every like 5 or 10 waves, increase the enemy strength, so that the same enemies will have more health (boss also having higher damage) and give more points. This ensures the game remains challenging even if you have good weapons and upgrades.

enum class GameState {
    MainMenu,
    Playing,
    Scoreboard,
    Controls,
    GameOver
};

int main() {
    using namespace ftxui;

    Game game;
    HighScore highscore;
    HUD hud;
    Shop shop;
    GameState current_state = GameState::MainMenu;
    int selected_menu_item = 0;
    std::string player_name = "Player";
    bool paused = false;
    
    // Shop state tracking
    ItemCategory current_shop_category = ItemCategory::WEAPON;
    int shop_selected_item = 0;
    std::vector<ShopItem> current_shop_items = shop.GetItemsByCategory(current_shop_category);

    // Submenu state for showing upgrade options
    bool in_upgrade_submenu = false;
    int upgrade_submenu_selected = 0;
    std::vector<std::string> upgrade_options;
    std::string current_upgrade_item = "";

    auto screen = ScreenInteractive::Fullscreen();

    // Main menu renderer
    auto main_menu_renderer = Renderer([&] {
        std::vector<std::string> menu_items = {"Start Game", "Scoreboard", "Controls"};

        Elements menu_elements;
        menu_elements.push_back(text("  ███████╗██████╗  █████╗  ██████╗███████╗    ███████╗██╗  ██╗ ██████╗  ██████╗ ████████╗███████╗██████╗ ") | bold | color(Color::Red) | center);
        menu_elements.push_back(text("  ██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝    ██╔════╝██║  ██║██╔═══██╗██╔═══██╗╚══██╔══╝██╔════╝██╔══██╗") | bold | color(Color::Red) | center);
        menu_elements.push_back(text("  ███████╗██████╔╝███████║██║     █████╗      ███████╗███████║██║   ██║██║   ██║   ██║   █████╗  ██████╔╝") | bold | color(Color::Red) | center);
        menu_elements.push_back(text("  ╚════██║██╔═══╝ ██╔══██║██║     ██╔══╝      ╚════██║██╔══██║██║   ██║██║   ██║   ██║   ██╔══╝  ██╔══██╗") | bold | color(Color::Red) | center);
        menu_elements.push_back(text("  ███████║██║     ██║  ██║╚██████╗███████╗    ███████║██║  ██║╚██████╔╝╚██████╔╝   ██║   ███████╗██║  ██║") | bold | color(Color::Red) | center);
        menu_elements.push_back(text("  ╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝╚══════╝    ╚══════╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝    ╚═╝   ╚══════╝╚═╝  ╚═╝") | bold | color(Color::Red) | center);
        menu_elements.push_back(text(""));

        for (size_t i = 0; i < menu_items.size(); ++i) {
            auto button = text("    " + menu_items[i] + "    ") | bold | center;
            if (static_cast<int>(i) == selected_menu_item) {
                button = button | bgcolor(Color::Blue) | color(Color::White) | bold;
            } else {
                button = button | bgcolor(Color::DarkBlue) | color(Color::White);
            }
            menu_elements.push_back(button | hcenter);
            if (i + 1 < menu_items.size()) {
                menu_elements.push_back(text(""));
            }
        }

        menu_elements.push_back(text(""));
        menu_elements.push_back(text("Use ↑↓ to navigate, Enter to select") | color(Color::GrayLight) | center);
        menu_elements.push_back(text("ESC to quit") | color(Color::GrayLight) | center);

        return vbox(std::move(menu_elements)) | center | border | color(Color::White) | bgcolor(Color::RGB(16, 16, 16));
    });

    // Scoreboard renderer
    auto scoreboard_renderer = Renderer([&] {
        const auto& top_scores = highscore.GetTopScores();

        Elements scoreboard_elements;
        scoreboard_elements.push_back(text("HIGH SCORES") | bold | color(Color::Green) | center);
        scoreboard_elements.push_back(separator());
        scoreboard_elements.push_back(text(""));

        if (top_scores.empty()) {
            scoreboard_elements.push_back(text("No scores yet!") | color(Color::GrayLight) | center);
        } else {
            for (size_t i = 0; i < top_scores.size(); ++i) {
                const auto& entry = top_scores[i];
                std::string rank = std::to_string(i + 1) + ".";
                std::string score_line = rank + " " + entry.player_name + " - " +
                                       std::to_string(entry.score) + " (Wave " +
                                       std::to_string(entry.wave) + ")";
                scoreboard_elements.push_back(text(score_line) | color(Color::White));
            }
        }

        scoreboard_elements.push_back(text(""));
        scoreboard_elements.push_back(text("Press ESC to return to menu") | color(Color::GrayLight) | center);

        return vbox(std::move(scoreboard_elements)) | center | border | color(Color::White);
    });

    // Controls renderer
    auto controls_renderer = Renderer([&] {
        Elements controls_elements;
        controls_elements.push_back(text("CONTROLS & GAMEPLAY") | bold | color(Color::Yellow) | center);
        controls_elements.push_back(separator());
        controls_elements.push_back(text(""));

        controls_elements.push_back(text("Gameplay Overview:") | bold | color(Color::Cyan));
        controls_elements.push_back(text("- Survive waves of enemies while upgrading weapons and bullets." ) | color(Color::GrayLight));
        controls_elements.push_back(text("- Press P during play to open the Shop and upgrade/buy items.") | color(Color::GrayLight));
        controls_elements.push_back(text("- Press Q during play to quit back to the main menu.") | color(Color::GrayLight));
        controls_elements.push_back(text(""));

        controls_elements.push_back(text("Movement & Shooting:") | bold | color(Color::Cyan));
        controls_elements.push_back(text("- Use the keyboard to move the player ship and aim.") | color(Color::GrayLight));
        controls_elements.push_back(text("- Press SPACE to shoot.") | color(Color::GrayLight));
        controls_elements.push_back(text(""));

        controls_elements.push_back(text("Weapons & Bullets:") | bold | color(Color::Cyan));
        controls_elements.push_back(text("- Z/X/C: Switch between weapon types.") | color(Color::GrayLight));
        controls_elements.push_back(text("- 1/2/3: Switch bullet types (Basic, Explosive, Piercing).") | color(Color::GrayLight));
        controls_elements.push_back(text("- Explosive bullets cannot be used with Dual or Tri Shot weapons.") | color(Color::GrayLight));
        controls_elements.push_back(text(""));
        controls_elements.push_back(text("Weapon Types:") | bold | color(Color::Cyan));
        controls_elements.push_back(text("- Basic Pistol: single-shot damage.") | color(Color::GrayLight));
        controls_elements.push_back(text("- Dual Shot: fires two bullets at once.") | color(Color::GrayLight));
        controls_elements.push_back(text("- Tri Shot: fires three bullets in a spread.") | color(Color::GrayLight));
        controls_elements.push_back(text("- Explosive: area damage on impact.") | color(Color::GrayLight));
        controls_elements.push_back(text("- Piercing: bullets pass through enemies.") | color(Color::GrayLight));
        controls_elements.push_back(text(""));

        controls_elements.push_back(text("Shop Controls:") | bold | color(Color::Cyan));
        controls_elements.push_back(text("- ← / → : Switch shop categories.") | color(Color::GrayLight));
        controls_elements.push_back(text("- ↑ / ↓ : Select item.") | color(Color::GrayLight));
        controls_elements.push_back(text("- ENTER : Buy or upgrade selected item.") | color(Color::GrayLight));
        controls_elements.push_back(text("- P or ESC : Resume the game.") | color(Color::GrayLight));
        controls_elements.push_back(text(""));

        controls_elements.push_back(text("Press ESC to return to the main menu.") | color(Color::GrayLight));

        return vbox(std::move(controls_elements)) | center | border | color(Color::White);
    });

    // Game Over renderer
    auto gameover_renderer = Renderer([&] {
        Elements go_elements;

        // ASCII Art header
        go_elements.push_back(text("") | size(HEIGHT, EQUAL, 2));
        go_elements.push_back(text("  ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ ") | bold | color(Color::Red) | center);
        go_elements.push_back(text(" ██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗") | bold | color(Color::Red) | center);
        go_elements.push_back(text(" ██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝") | bold | color(Color::Red) | center);
        go_elements.push_back(text(" ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║██║   ██║██╔══╝  ██╔══██╗") | bold | color(Color::Red) | center);
        go_elements.push_back(text(" ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝╚██████╔╝███████╗██║  ██║") | bold | color(Color::Red) | center);
        go_elements.push_back(text("  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═════╝ ╚══════╝╚═╝  ╚═╝") | bold | color(Color::Red) | center);
        go_elements.push_back(text("") | size(HEIGHT, EQUAL, 1));

        // Stats box
        go_elements.push_back(text("╔═══════════════════════════════════════════╗") | color(Color::Yellow) | center);
        go_elements.push_back(text("║                 SUMMARY                   ║") | bold | color(Color::Red) | center);
        go_elements.push_back(text("╠═══════════════════════════════════════════╣") | color(Color::Yellow) | center);

        std::string score_line = "║      FINAL SCORE:    " + std::to_string(game.GetScore()) + std::string(20 - std::to_string(game.GetScore()).length(), ' ') + " ║";
        go_elements.push_back(text(score_line) | color(Color::Cyan) | center);

        std::string wave_line = "║      WAVES SURVIVED: " + std::to_string(game.GetWave()) + std::string(18 - std::to_string(game.GetWave()).length(), ' ') + "   ║";
        go_elements.push_back(text(wave_line) | color(Color::Cyan) | center);

        go_elements.push_back(text("╚═══════════════════════════════════════════╝") | color(Color::Yellow) | center);
        go_elements.push_back(text("") | size(HEIGHT, EQUAL, 1));

        // Menu options
        go_elements.push_back(text("[ Q ] Return to Main Menu") | bold | color(Color::White) | center);
        go_elements.push_back(text("[ R ] Restart Game") | bold | color(Color::Yellow) | center);
        go_elements.push_back(text("") | size(HEIGHT, EQUAL, 2));

        return vbox(std::move(go_elements)) | center | color(Color::White) | bgcolor(Color::RGB(10, 10, 15));
    });

    // Game renderer
    auto game_renderer = Renderer([&] {
        // If paused, show full-screen shop
        if (paused) {
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
                if (category_values[i] == current_shop_category) {
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
            if (!current_shop_items.empty()) {
                for (size_t i = 0; i < current_shop_items.size(); ++i) {
                    const auto& item = current_shop_items[i];
                    std::string prefix = (static_cast<int>(i) == shop_selected_item) ? "→ " : "  ";

                    // Determine item status
                    std::string status_text;
                    Color item_color = Color::White;
                    bool is_selectable = !item.owned;

                    if (item.owned && !item.can_stack) {
                        // Non-stackable owned items (weapons, abilities, unlocks)
                        status_text = " [OWNED - ENTER to use]";
                        item_color = Color::Green;
                        is_selectable = true;
                    } else if (item.owned && item.can_stack) {
                        // Stackable owned items (upgrades) - show as upgradeable
                        status_text = " [OWNED - ENTER to upgrade]";
                        item_color = Color::Cyan;
                        is_selectable = shop.CanAfford(item, game.GetCash());
                    } else if (item.category == ItemCategory::BULLET && item.can_stack) {
                        // Bullet upgrades - check prerequisites
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
                    if (static_cast<int>(i) == shop_selected_item && is_selectable) {
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

            }  // end else for non-submenu

            return vbox(std::move(shop_lines)) | border | color(Color::Green) | bgcolor(Color::RGB(16, 16, 16)) | flex;
        }
        
        // Normal gameplay display (when not paused)
        Canvas c(Game::WIDTH, Game::HEIGHT);
        game.Draw(c);

        Element game_display = canvas(c) | border | color(Color::White);

        // Create the HUD display at the top
        Element hud_display = hud.Render(game, game.GetShootCooldown(), game.GetMaxShootCooldown());

        // Stack HUD on top of game display
        Element combined_display = vbox({
            hud_display | size(HEIGHT, LESS_THAN, 9),
            separator(),
            game_display | flex_grow,
        });

        return combined_display;
    });

    // Main renderer that switches based on state
    auto main_renderer = Renderer([&] {
        switch (current_state) {
            case GameState::MainMenu:
                return main_menu_renderer->Render();
            case GameState::Scoreboard:
                return scoreboard_renderer->Render();
            case GameState::Controls:
                return controls_renderer->Render();
            case GameState::Playing:
                return game_renderer->Render();
            case GameState::GameOver:
                return gameover_renderer->Render();
            default:
                return text("Unknown state") | center;
        }
    });

    // Event handler
    auto component = main_renderer | CatchEvent([&](Event event) {
        switch (current_state) {
            case GameState::MainMenu: {
                if (event == Event::ArrowUp) {
                    selected_menu_item = (selected_menu_item - 1 + 3) % 3;
                    return true;
                }
                if (event == Event::ArrowDown) {
                    selected_menu_item = (selected_menu_item + 1) % 3;
                    return true;
                }
                if (event == Event::Return) {
                    if (selected_menu_item == 0) { // Start Game
                        current_state = GameState::Playing;
                        paused = false;
                        shop_selected_item = 0;
                        game = Game(); // Reset game
                    } else if (selected_menu_item == 1) { // Scoreboard
                        current_state = GameState::Scoreboard;
                    } else if (selected_menu_item == 2) { // Controls
                        current_state = GameState::Controls;
                    }
                    return true;
                }
                if (event == Event::Escape) {
                    screen.ExitLoopClosure()();
                    return true;
                }
                break;
            }

            case GameState::Scoreboard: {
                if (event == Event::Escape) {
                    current_state = GameState::MainMenu;
                    paused = false;
                    return true;
                }
                break;
            }

            case GameState::Controls: {
                if (event == Event::Escape || event == Event::Character('q') || event == Event::Character('Q')) {
                    current_state = GameState::MainMenu;
                    return true;
                }
                break;
            }

            case GameState::Playing: {
                if (event == Event::Character('q') || event == Event::Character('Q')) {
                    // Save progress before returning to the menu
                    if (highscore.IsHighScore(game.GetScore())) {
                        highscore.AddScore(player_name, game.GetScore(), game.GetWave());
                    }
                    current_state = GameState::MainMenu;
                    paused = false;
                    return true;
                }

                if (event == Event::Character('p') || event == Event::Character('P')) {
                    paused = !paused;
                    if (paused) {
                        // Reset shop to Weapons category when pausing
                        current_shop_category = ItemCategory::WEAPON;
                        current_shop_items = shop.GetItemsByCategory(current_shop_category);
                        shop_selected_item = 0;
                    }
                    return true;
                }

                if (paused) {
                    // If in upgrade submenu, handle differently
                    if (in_upgrade_submenu) {
                        if (event == Event::ArrowUp) {
                            upgrade_submenu_selected = (upgrade_submenu_selected - 1 + static_cast<int>(upgrade_options.size())) % static_cast<int>(upgrade_options.size());
                            return true;
                        }
                        if (event == Event::ArrowDown) {
                            upgrade_submenu_selected = (upgrade_submenu_selected + 1) % static_cast<int>(upgrade_options.size());
                            return true;
                        }
                        if (event == Event::Return) {
                            // Apply the selected upgrade
                            int cost = 0;
                            if (current_upgrade_item == "Basic Damage +1") {
                                cost = 50;
                                game.UpgradeBasicBulletDamage(cost);
                            } else if (current_upgrade_item == "Basic Speed +1") {
                                cost = 120;
                                game.UpgradeBasicBulletSpeed(cost);
                            } else if (current_upgrade_item == "Explosive Damage +1") {
                                cost = 60;
                                game.UpgradeExplosiveDamage(cost);
                            } else if (current_upgrade_item == "Explosive Radius +1") {
                                cost = 60;
                                game.UpgradeExplosiveRadius(cost);
                            } else if (current_upgrade_item == "Piercing Damage +1") {
                                cost = 50;
                                game.UpgradePiercingDamage(cost);
                            } else if (current_upgrade_item == "Piercing Penetra. +1") {
                                cost = 60;
                                game.UpgradePiercingPenetration(cost);
                            }
                            in_upgrade_submenu = false;
                            return true;
                        }
                        if (event == Event::Escape) {
                            in_upgrade_submenu = false;
                            upgrade_submenu_selected = 0;
                            return true;
                        }
                        return true;
                    }

                    // Handle category switching with Left/Right arrows
                    if (event == Event::ArrowLeft) {
                        if (current_shop_category == ItemCategory::WEAPON) {
                            current_shop_category = ItemCategory::ABILITY;
                        } else if (current_shop_category == ItemCategory::BULLET) {
                            current_shop_category = ItemCategory::WEAPON;
                        } else if (current_shop_category == ItemCategory::ITEM) {
                            current_shop_category = ItemCategory::BULLET;
                        } else {
                            current_shop_category = ItemCategory::ITEM;
                        }
                        current_shop_items = shop.GetItemsByCategory(current_shop_category);
                        shop_selected_item = 0;
                        return true;
                    }
                    if (event == Event::ArrowRight) {
                        if (current_shop_category == ItemCategory::WEAPON) {
                            current_shop_category = ItemCategory::BULLET;
                        } else if (current_shop_category == ItemCategory::BULLET) {
                            current_shop_category = ItemCategory::ITEM;
                        } else if (current_shop_category == ItemCategory::ITEM) {
                            current_shop_category = ItemCategory::ABILITY;
                        } else {
                            current_shop_category = ItemCategory::WEAPON;
                        }
                        current_shop_items = shop.GetItemsByCategory(current_shop_category);
                        shop_selected_item = 0;
                        return true;
                    }

                    // Handle item selection with Up/Down arrows
                    if (event == Event::ArrowUp) {
                        shop_selected_item = (shop_selected_item - 1 + static_cast<int>(current_shop_items.size())) % static_cast<int>(current_shop_items.size());
                        return true;
                    }
                    if (event == Event::ArrowDown) {
                        shop_selected_item = (shop_selected_item + 1) % static_cast<int>(current_shop_items.size());
                        return true;
                    }

                    // Handle purchase with Enter
                    if (event == Event::Return) {
                        if (!current_shop_items.empty() && shop_selected_item < static_cast<int>(current_shop_items.size())) {
                            auto& item = current_shop_items[shop_selected_item];
                            int cash = game.GetCash();

                            // If item is owned, handle differently based on type
                            if (item.owned) {
                                if (item.category == ItemCategory::ABILITY) {
                                    // Activate owned abilities
                                    if (item.name == "Shield Barrier") {
                                        game.BuyShield(item.cost);
                                    } else if (item.name == "Rapid Fire") {
                                        // Rapid fire is permanent once bought, but could be activated again
                                    } else if (item.name == "Time Slow") {
                                        game.ActivateTimeSlow();
                                    } else if (item.name == "Freeze") {
                                        game.ActivateFreeze();
                                    }
                                    return true;
                                } else if (item.category == ItemCategory::WEAPON) {
                                    // Switch to owned weapon
                                    if (item.name == "Dual Shot") {
                                        game.SetWeaponType(WeaponType::DUAL);
                                    } else if (item.name == "Tri Shot") {
                                        game.SetWeaponType(WeaponType::TRI);
                                    }
                                    // If current bullet type is not allowed, switch to normal
                                    if (!game.CanUseBulletType(game.GetBulletType())) {
                                        game.SetBulletType(BulletType::NORMAL);
                                    }
                                    return true;
                                }
                                return true;  // Other owned items - nothing to do
                            }

                            // Handle bullet upgrades (can be purchased multiple times)
                            if (item.category == ItemCategory::BULLET && item.can_stack) {
                                // Show upgrade options for bullet upgrades
                                current_upgrade_item = item.name;
                                upgrade_options.clear();
                                if (item.name == "Basic Damage +1") {
                                    upgrade_options.push_back("+1 Damage (Cost: $50)");
                                } else if (item.name == "Basic Speed +1") {
                                    upgrade_options.push_back("+1 Speed (Cost: $120)");
                                } else if (item.name == "Explosive Damage +1") {
                                    if (!game.HasExplosive()) return true;  // Can't upgrade if not unlocked
                                    upgrade_options.push_back("+1 Damage (Cost: $60)");
                                } else if (item.name == "Explosive Radius +1") {
                                    if (!game.HasExplosive()) return true;  // Can't upgrade if not unlocked
                                    upgrade_options.push_back("+1 Radius (Cost: $60)");
                                } else if (item.name == "Piercing Damage +1") {
                                    if (!game.HasPiercing()) return true;  // Can't upgrade if not unlocked
                                    upgrade_options.push_back("+1 Damage (Cost: $50)");
                                } else if (item.name == "Piercing Penetra. +1") {
                                    if (!game.HasPiercing()) return true;  // Can't upgrade if not unlocked
                                    upgrade_options.push_back("+1 Penetration (Cost: $60)");
                                }
                                if (!upgrade_options.empty()) {
                                    upgrade_options.push_back("CONFIRM UPGRADE");
                                    in_upgrade_submenu = true;
                                    upgrade_submenu_selected = 0;
                                }
                                return true;
                            }

                            // Not owned - try to buy
                            if (shop.CanAfford(item, cash)) {
                                // Handle different types of items
                                if (item.category == ItemCategory::WEAPON) {
                                    if (item.name == "Dual Shot") {
                                        game.BuyWeapon(WeaponType::DUAL, item.cost);
                                        item.owned = true;
                                    } else if (item.name == "Tri Shot") {
                                        game.BuyWeapon(WeaponType::TRI, item.cost);
                                        item.owned = true;
                                    }
                                } else if (item.category == ItemCategory::BULLET) {
                                    if (item.name == "Unlock Explosive") {
                                        game.BuyExplosiveBullet(item.cost);
                                        item.owned = true;
                                    } else if (item.name == "Unlock Piercing") {
                                        game.BuyPiercingBullet(item.cost);
                                        item.owned = true;
                                    }
                                } else if (item.category == ItemCategory::ITEM) {
                                    // One-time use items - apply immediately
                                    if (item.name == "Speed Boost") {
                                        game.BuySpeedBoost(item.cost);
                                    } else if (item.name == "Health Pack") {
                                        game.BuyHealthPack(item.cost);
                                    } else if (item.name == "Shield Pack") {
                                        game.BuyShieldPack(item.cost);
                                    } else if (item.name == "Damage Boost") {
                                        game.BuyDamageBoost(item.cost);
                                    }
                                } else if (item.category == ItemCategory::ABILITY) {
                                    if (item.name == "Shield Barrier") {
                                        game.BuyShield(item.cost);
                                        item.owned = true;
                                    } else if (item.name == "Rapid Fire") {
                                        game.BuyRapidFire(item.cost);
                                        item.owned = true;
                                    } else if (item.name == "Time Slow") {
                                        game.BuyTimeSlow(item.cost);
                                        item.owned = true;
                                    } else if (item.name == "Freeze") {
                                        game.BuyFreeze(item.cost);
                                        item.owned = true;
                                    }
                                }
                            }
                        }
                        return true;
                    }

                    if (event == Event::Escape) {
                        paused = false;
                        return true;
                    }
                    if (event == Event::Character('p') || event == Event::Character('P')) {
                        paused = false;
                        return true;
                    }
                    return true;
                }

                bool handled = game.HandleEvent(event);

                if (game.IsGameOver()) {
                    current_state = GameState::GameOver;
                    // Save score if it's a high score
                    if (highscore.IsHighScore(game.GetScore())) {
                        highscore.AddScore(player_name, game.GetScore(), game.GetWave());
                    }
                }

                return handled;
            }

            case GameState::GameOver: {
                if (event == Event::Character('q') || event == Event::Character('Q') ||
                    event == Event::Escape) {
                    current_state = GameState::MainMenu;
                    return true;
                }
                if (event == Event::Character('r') || event == Event::Character('R')) {
                    // Restart game
                    game = Game();
                    shop.Reset();  // Reset shop items
                    current_state = GameState::Playing;
                    return true;
                }
                return false;  // Don't pass events to game when dead
            }
        }

        return false;
    });

    std::atomic<bool> running = true;
    std::thread ticker([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
            if (current_state == GameState::Playing && !paused) {
                screen.Post([&] {
                    game.Update();
                    screen.RequestAnimationFrame();
                });
            }
        }
    });

    // Run the interactive loop
    screen.Loop(component);

    running = false;
    ticker.join();

    return 0;
}
