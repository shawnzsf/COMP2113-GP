# Makefile for Terminal Ball Drop Game (Linux/ncurses)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
TARGET = game
LIBS = -lncurses

# Source files
SRCS = main.cpp \
       src/render.cpp \
       src/physics.cpp \
       src/game.cpp \
       src/input.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile header dependencies
src/render.o: src/render.cpp include/render.h
src/physics.o: src/physics.cpp include/physics.h include/render.h
src/game.o: src/game.cpp include/game.h include/render.h
src/input.o: src/input.cpp include/input.h include/physics.h
main.o: main.cpp include/render.h include/game.h include/input.h include/physics.h

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)

# Run the game
run: $(TARGET)
	./$(TARGET)

# Rebuild everything
rebuild: clean all

.PHONY: all clean run rebuild
