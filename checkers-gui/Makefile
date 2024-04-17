# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++20 -Wall -Wextra

# Include directories
INCLUDES = -Iinclude -I../dependecies/SFML/include

# Libraries
LIBS = -L../dependencies/SFML/lib -lsfml-system -lsfml-graphics -lsfml-audio -lsfml-window

# Source files
SRCS = $(wildcard src/*.cpp)

# Object files
OBJS = $(SRCS:.cpp=.o)
OBJS := $(addprefix build/, $(OBJS))

# Executable name
MAIN = main

.PHONY: default all debug clean depend

default: all

all: $(MAIN)
	@echo "Checkers GUI has been compiled"

debug: CFLAGS += -g -DDEBUG -D_DEBUG
debug: $(MAIN)
	@echo "Checkers GUI has been compiled in debug mode"

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

build/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) build/src/*.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# Include dependency files
-include $(SRCS:.cpp=.d)
