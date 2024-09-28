# Compiler
CXX = g++

# Project name (output file)
TARGET = emulator

# Source files
SRC = main.cpp chip8.cpp

# SDL2 flags
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LDFLAGS = $(shell sdl2-config --libs)


# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11 $(SDL2_CFLAGS)

# Linker flags
LDFLAGS = $(SDL2_LDFLAGS)

# Build the target
$(TARGET): $(SRC)
	@echo $(SDL2_CFLAGS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean the project (removes the output file)
.PHONY: clean
clean:
	rm -f $(TARGET)

