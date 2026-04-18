cmake_minimum_required(VERSION 3.14)
project(SHOOTER_GAME LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(thirdparty/ftxui)

add_executable(Shooter_game
    src/core/main.cpp
    src/core/game.cpp
    src/data/highscore.cpp
    src/entities/bullet.cpp
    src/entities/enemy.cpp
    src/entities/player.cpp
    src/ui/hud.cpp
    src/ui/menu.cpp
    src/ui/shop.cpp
    src/ui/shop_renderer.cpp
    src/weapons/weapon.cpp
)

target_link_libraries(Shooter_game PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

if(MSVC)
    target_compile_options(Shooter_game PRIVATE /W4)
else()
    target_compile_options(Shooter_game PRIVATE -Wall -Wextra)
endif()
