CC = g++
CFLAGS = -Wall -Werror -std=c++20
INCLUDES = -I.

SRC_DIR = src
BUILD_DIR = build

#SFML_INCLUDE_DIR = /usr/local/include
#SFML_LIB_DIR = /usr/local/lib
#SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

SFML_INCLUDE_DIR = c:/SFML-2.5.1/include
SFML_LIB_DIR = c:/SFML-2.5.1/lib
SFML_LIBS = sfml-graphics.lib sfml-window.lib sfml-system.lib sfml-audio.lib sfml-network.lib

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/game.o $(BUILD_DIR)/base_player.o $(BUILD_DIR)/player.o $(BUILD_DIR)/bot.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/king.o $(BUILD_DIR)/available_move.o $(BUILD_DIR)/available_capture.o

TARGET = output

SRC_FILES = main.cpp game.cpp base_player.cpp player.cpp bot.cpp piece.cpp king.cpp available_move.cpp available_capture.cpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)
