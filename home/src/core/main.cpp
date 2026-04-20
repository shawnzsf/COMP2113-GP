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

    // Shop state tracking – store global index + copy of item for display
    ItemCategory current_shop_category = ItemCategory::WEAPON;
    int shop_selected_item = 0;
    std::vector<std::pair<int, ShopItem>> current_shop_items; // <global_index, item_copy>

    // Helper to refresh the displayed items from the original shop data
    auto refresh_shop_items = [&]() {
        current_shop_items.clear();
        const auto& all = shop.GetAllItems();
        for (size_t i = 0; i < all.size(); ++i) {
            if (all[i].category == current_shop_category) {
                current_shop_items.push_back({(int)i, all[i]});
            }
        }
        // Keep selected index within bounds
        if (shop_selected_item >= (int)current_shop_items.size())
            shop_selected_item = std::max(0, (int)current_shop_items.size() - 1);
    };

    // Initial refresh
    refresh_shop_items();

    // Submenu state (unused now, kept for potential future expansion)
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
            // Build a temporary vector of just the ShopItem parts for rendering
            std::vector<ShopItem> render_items;
            render_items.reserve(current_shop_items.size());
            for (const auto& p : current_shop_items)
                render_items.push_back(p.second);

            return shop_renderer.RenderShopOnly(game, shop, current_shop_category,
                                    shop_selected_item, render_items,
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
            game_display | center,
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
                        shop_selected_item = 0;
                        refresh_shop_items();
                    }
                    return true;
                }

                if (paused) {
                    // No upgrade submenu for now – direct purchase handles upgrades

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
                        refresh_shop_items();
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
                        refresh_shop_items();
                        shop_selected_item = 0;
                        return true;
                    }

                    // Handle item selection with Up/Down arrows
                    if (event == Event::ArrowUp) {
                        if (!current_shop_items.empty()) {
                            shop_selected_item = (shop_selected_item - 1 + (int)current_shop_items.size()) % (int)current_shop_items.size();
                        }
                        return true;
                    }
                    if (event == Event::ArrowDown) {
                        if (!current_shop_items.empty()) {
                            shop_selected_item = (shop_selected_item + 1) % (int)current_shop_items.size();
                        }
                        return true;
                    }

                    // Handle purchase / activation with Enter
                    if (event == Event::Return) {
                        if (current_shop_items.empty()) return true;
                        auto& pair = current_shop_items[shop_selected_item];
                        int global_index = pair.first;
                        ShopItem& item_copy = pair.second; // copy for display, not for modification
                        int cash = game.GetCash();

                        // If item is owned and not stackable, it's an activation (weapon switch or ability use)
                        if (item_copy.owned && !item_copy.can_stack && item_copy.max_upgrade_level == 0) {
                            // Activate owned weapon or ability
                            if (item_copy.category == ItemCategory::WEAPON) {
                                if (item_copy.name == "Dual Shot") {
                                    game.SetWeaponType(WeaponType::DUAL);
                                } else if (item_copy.name == "Tri Shot") {
                                    game.SetWeaponType(WeaponType::TRI);
                                }
                                // If current bullet type is not allowed, switch to normal
                                if (!game.CanUseBulletType(game.GetBulletType())) {
                                    game.SetBulletType(BulletType::NORMAL);
                                }
                            } else if (item_copy.category == ItemCategory::ABILITY) {
                                if (item_copy.name == "Shield Barrier") {
                                    game.BuyShield(0); // already owned, just activate
                                } else if (item_copy.name == "Rapid Fire") {
                                    game.BuyRapidFire(0);
                                } 
                                
                                // else if (item_copy.name == "Time Slow") {
                                //     game.ActivateTimeSlow();   }
                                
                            }
                            return true;
                        }

                        // Otherwise, attempt to purchase the item (or upgrade it)
                        if (shop.PurchaseItemByIndex(global_index, cash)) {
                            // Purchase succeeded – apply game-specific effects
                            // The shop's internal item has been updated (quantity, upgrade_level, owned)
                            // Now we need to apply the effect to the game (e.g., unlock bullet, give temporary boost, etc.)
                            // Reload the fresh copy of the item to know what was bought
                            const auto& all = shop.GetAllItems();
                            const ShopItem& bought = all[global_index];

                            if (bought.category == ItemCategory::WEAPON) {
                                if (bought.name == "Dual Shot") {
                                    game.BuyWeapon(WeaponType::DUAL, bought.cost);
                                } else if (bought.name == "Tri Shot") {
                                    game.BuyWeapon(WeaponType::TRI, bought.cost);
                                }
                            } else if (bought.category == ItemCategory::BULLET) {
                                if (bought.name == "Unlock Explosive") {
                                    game.BuyExplosiveBullet(bought.cost);
                                } else if (bought.name == "Unlock Piercing") {
                                    game.BuyPiercingBullet(bought.cost);
                                } else if (bought.name == "Basic Damage +1") {
                                    game.UpgradeBasicBulletDamage(bought.cost);
                                } else if (bought.name == "Basic Speed +1") {
                                    game.UpgradeBasicBulletSpeed(bought.cost);
                                } else if (bought.name == "Explosive Damage +1") {
                                    game.UpgradeExplosiveDamage(bought.cost);
                                } else if (bought.name == "Explosive Radius +1") {
                                    game.UpgradeExplosiveRadius(bought.cost);
                                } else if (bought.name == "Piercing Damage +1") {
                                    game.UpgradePiercingDamage(bought.cost);
                                } else if (bought.name == "Piercing Penetra. +1") {
                                    game.UpgradePiercingPenetration(bought.cost);
                                }
                            } else if (bought.category == ItemCategory::ITEM) {
                                // Consumable items – apply immediate effect
                                if (bought.name == "Speed Boost") {
                                    game.BuySpeedBoost(bought.cost);
                                } else if (bought.name == "Health Pack") {
                                    game.BuyHealthPack(bought.cost);
                                } else if (bought.name == "Shield Pack") {
                                    game.BuyShieldPack(bought.cost);
                                } else if (bought.name == "Damage Boost") {
                                    game.BuyDamageBoost(bought.cost);
                                }
                            } else if (bought.category == ItemCategory::ABILITY) {
                                if (bought.name == "Shield Barrier") {
                                    game.BuyShield(bought.cost);
                                } else if (bought.name == "Rapid Fire") {
                                    game.BuyRapidFire(bought.cost);
                                }
                                // } else if (bought.name == "Time Slow") {
                                //     game.BuyTimeSlow(bought.cost);
                                // } 
                                else if (bought.name == "Freeze") {
                                    game.BuyFreeze(bought.cost);
                                }
                            }

                            // Refresh the displayed items to reflect updated quantities/owned flags
                            refresh_shop_items();
                            // Keep selection on the same logical item (may shift if item count changed)
                            if (shop_selected_item >= (int)current_shop_items.size())
                                shop_selected_item = std::max(0, (int)current_shop_items.size() - 1);
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
                    game = Game();  // Reset game
                    shop.Reset();   // Reset shop items for new game
                    refresh_shop_items();
                    shop_selected_item = 0;
                    current_shop_category = ItemCategory::WEAPON;
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
                    refresh_shop_items();
                    current_state = GameState::Playing;
                    paused = false;
                    return true;
                }
                return false;
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