# Terminal Ball Drop Game

A physics-based puzzle game where you guide a ball from the start (S) to the exit (E) by placing tools.

## How to Play

### Controls
- **WASD / Arrow Keys**: Move cursor
- **1**: Select Plate (Left /)
- **2**: Select Plate (Right \)
- **3**: Select Bounce Pad
- **4**: Select Wall
- **Space**: Place/Remove tool at cursor position
- **Enter**: Start simulation
- **ESC**: Stop simulation (return to placement)
- **R**: Restart (after win/lose)

### Tools
- **Plate (/)**: Deflects ball to the left
- **Plate (\)**: Deflects ball to the right
- **Bounce Pad**: Gives ball a strong upward bounce
- **Wall**: Stops and bounces ball back

### Goal
Place tools to guide the falling ball from S to E without it going out of bounds.

## Building

### Prerequisites
- C++ compiler (g++)
- ncurses library

### Install Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install libncurses-dev

# Fedora
sudo dnf install ncurses-devel

# Arch Linux
sudo pacman -S ncurses
```

### Build
```bash
make clean && make
```

### Run
```bash
./game
```

## Project Structure
```
├── main.cpp          # Main game loop
├── Makefile         # Build script
├── include/         # Header files
│   ├── game.h
│   ├── input.h
│   ├── physics.h
│   └── render.h
└── src/            # Source files
    ├── game.cpp
    ├── input.cpp
    ├── physics.cpp
    └── render.cpp
```

## Physics
- Gravity
- Air resistance
- Collision detection with tools and exit
- Energy loss
