/**
 * @file main.cpp
 * @brief Main entry point - game loop and state management
 */

#include "game.hpp"
#include "../data/highscore.hpp"
#include "../ui/hud.hpp"
#include "../ui/shop.hpp"
#include "../ui/menu.hpp"
#include "../ui/shop_renderer.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"

#include <atomic>
#include <chrono>
#include <thread>
#include <string>

int main() {
    using namespace ftxui;

    Game game;
    HighScore highscore;
    HUD hud;
    Shop shop;
    MenuRenderer menu_renderer(highscore);
    ShopRenderer shop_renderer;

    GameState current_state = GameState::MainMenu;
    int selected_menu_item = 0;
    int selected_difficulty = 1;  // Default to Medium
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
        return menu_renderer.RenderMainMenu(selected_menu_item);
    });

    // Scoreboard renderer
    auto scoreboard_renderer = Renderer([&] {
        return menu_renderer.RenderScoreboard();
    });

    // Controls renderer
    auto controls_renderer = Renderer([&] {
        return menu_renderer.RenderControls();
    });

    // Game Over renderer
    auto gameover_renderer = Renderer([&] {
        return menu_renderer.RenderGameOver(game.GetScore(), game.GetWave());
    });

    // Difficulty selection renderer
    auto difficulty_renderer = Renderer([&] {
        return menu_renderer.RenderDifficultySelect(selected_difficulty);
    });

    // Game renderer
    auto game_renderer = Renderer([&] {
        // If paused, show full-screen shop
        if (paused) {
            return shop_renderer.RenderShopOnly(game, shop, current_shop_category,
                                    shop_selected_item, current_shop_items,
                                    in_upgrade_submenu, upgrade_submenu_selected,
                                    upgrade_options, current_upgrade_item);
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
            case GameState::DifficultySelect:
                return difficulty_renderer->Render();
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
                        current_state = GameState::DifficultySelect;
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
                                        item.quantity++;
                                    } else if (item.name == "Health Pack") {
                                        game.BuyHealthPack(item.cost);
                                        item.quantity++;
                                    } else if (item.name == "Shield Pack") {
                                        game.BuyShieldPack(item.cost);
                                        item.quantity++;
                                    } else if (item.name == "Damage Boost") {
                                        game.BuyDamageBoost(item.cost);
                                        item.quantity++;
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

            case GameState::DifficultySelect: {
                if (event == Event::ArrowUp) {
                    selected_difficulty = (selected_difficulty - 1 + 3) % 3;
                    return true;
                }
                if (event == Event::ArrowDown) {
                    selected_difficulty = (selected_difficulty + 1) % 3;
                    return true;
                }
                if (event == Event::Return) {
                    // Set difficulty and start game
                    if (selected_difficulty == 0) {
                        game.SetDifficulty(DifficultyLevel::Easy);
                    } else if (selected_difficulty == 1) {
                        game.SetDifficulty(DifficultyLevel::Medium);
                    } else {
                        game.SetDifficulty(DifficultyLevel::Hard);
                    }
                    current_state = GameState::Playing;
                    paused = false;
                    shop_selected_item = 0;
                    game = Game();  // Reset game
                    return true;
                }
                if (event == Event::Escape) {
                    current_state = GameState::MainMenu;
                    return true;
                }
                break;
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