#ifndef PIECE_H
#define PIECE_H

#include <iostream>
#include <list>
#include "Game.h"
#include "AvailableMove.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>

namespace Checkers
{
	class Piece
	{
		// x coordinate of the piece
		int m_x;
		// y coordinates of the piece
		int m_y;
		// sign corresponding to piece's owner
		char m_sign;
		// flag indicating if the piece was captured
		bool m_is_captured;
		// flag indicating if the piece is the king: change to another class?
		bool m_is_king;
		// list containing all evaluated possible moves for the piece
		std::list<AvailableMove*>* m_av_list;
	public:
		// creates the piece of given sign and coordinates
		Piece(char sign, int x, int y);
		// deletes the piece
		~Piece();
		// returns x coordinate of the piece
		int get_x(void);
		// returns y coordinate of the piece
		int get_y(void);
		// sets x to x1 and returns it
		int set_x(int x);
		// sets y to y1 and returns it
		int set_y(int y);
		// returns sign of the piece
		char get_sign(void);
		// sets flag indicating if the piece is captured and returns set value
		bool set_captured(bool captured = true);
		// returns true, if piece is already captured, false respectively
		bool get_is_captured(void);
		// lets to print the piece in given stream
		friend std::ostream& operator<<(std::ostream& os, const Piece* piece);
		// returns the list of all evaluated moves
		std::list<AvailableMove*>* get_av_list(void);
		// draws the piece to the window
		void draw(sf::RenderWindow& window);
	};

	std::ostream& operator<<(std::ostream& os, const Piece* piece);
}

#endif