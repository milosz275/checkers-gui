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
	// x coordinate of the piece
	int x;
	// y coordinates of the piece
	int y;
	// sign corresponding to piece's owner
	char sign;
	// flag indicating if the piece was captured
	bool is_captured;
	// flag indicating if the piece is the king: change to another class?
	bool is_king;
	// list containing all evaluated possible moves for the piece
	std::list<AvailableMove*>* av_list;
public:
	// creates the piece of given sign and coordinates
	Piece(char s, int x1, int y1);
	// deletes the piece
	~Piece();
	// returns x coordinate of the piece
	int get_x(void);
	// returns y coordinate of the piece
	int get_y(void);
	// sets x to x1 and returns it
	int set_x(int x1);
	// sets y to y1 and returns it
	int set_y(int y1);
	// returns sign of the piece
	char get_sign(void);
	// lets to print the piece in given stream
	friend std::ostream& operator<<(std::ostream& os, const Piece* piece);
	// returns the list of all evaluated moves
	std::list<AvailableMove*>* get_av_list(void);
	// draws the piece to the window
	void draw(sf::RenderWindow& window);
};

std::ostream& operator<<(std::ostream& os, const Piece* piece);

#endif