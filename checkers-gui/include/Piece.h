#ifndef PIECE_H
#define PIECE_H

#include <iostream>
#include <list>
#include "AvailableMove.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>

class Piece
{
	int x;
	int y;
	char sign;
	bool is_captured;
	bool is_king;
	std::list<AvailableMove*>* av_list;
public:
	Piece(char s, int x1, int y1);
	int get_x(void) { return x; }
	int get_y(void) { return y; }
	int set_x(int x1) { return x = x1; }
	int set_y(int y1) { return y = y1; }
	char get_sign(void) { return sign; }

	friend std::ostream& operator<<(std::ostream& os, const Piece* piece);
	std::list<AvailableMove*>* get_av_list(void);
	void draw(sf::RenderWindow& window);
};

std::ostream& operator<<(std::ostream& os, const Piece* piece);

#endif