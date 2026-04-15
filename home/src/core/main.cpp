#include "game.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"

#include <atomic>
#include <chrono>
#include <thread>

int main() {
    using namespace ftxui;

    Game game;
    auto screen = ScreenInteractive::Fullscreen();

    // Create a Renderer that handles both game rendering and UI
    auto renderer = Renderer([&] {
        Canvas c(Game::WIDTH, Game::HEIGHT);
        game.Draw(c);

        // Main game canvas with border
        Element game_display = canvas(c) 
                             | border 
                             | color(Color::White);

        if (game.IsGameOver()) {
            // Game over overlay
            game_display = dbox({
                game_display,
                filler() | size(HEIGHT, EQUAL, 1),
            });
        }

        return game_display;
    });

    // Event handler for keyboard input and quit logic
    auto component = renderer | CatchEvent([&](Event event) {
        // Handle quit event
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            if (game.IsGameOver()) {
                screen.ExitLoopClosure()();
                return true;
            }
        }

        // Handle ESC key to quit anytime
        if (event == Event::Escape) {
            screen.ExitLoopClosure()();
            return true;
        }

        // Pass all other events to the game
        return game.HandleEvent(event);
    });

    std::atomic<bool> running = true;
    std::thread ticker([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            screen.Post([&] {
                game.Update();
                screen.RequestAnimationFrame();
            });
        }
    });

    // Run the interactive game loop
    screen.Loop(component);

    running = false;
    ticker.join();

    return 0;
}
