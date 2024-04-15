# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++20 -Wall -Wextra

# Include directories
INCLUDES = -Iinclude -I../dependecies/SFML/include

# Libraries
LIBS = -L../dependencies/SFML/lib -lsfml-system -lsfml-graphics -lsfml-audio -lsfml-window

# Source files
SRCS = src/available_capture.cpp src/available_move.cpp src/base_player.cpp src/bot.cpp src/event_handler.cpp src/game.cpp src/game_state.cpp src/king.cpp src/main.cpp src/piece.cpp src/player.cpp src/gui.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)
OBJS := $(addprefix build/, $(OBJS))

# Executable name
MAIN = checkers-gui

.PHONY: depend clean

all: $(MAIN)
	@echo "Checkers GUI has been compiled"

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

build/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) build/*.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# Include dependency files
-include $(SRCS:.cpp=.d)
