#ifndef PIECE_H
#define PIECE_H

#include <iostream>
#include <list>

#include "include/game.h"
#include "include/available_move.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>

namespace checkers
{
	class piece
	{
	protected:
		// x coordinate of the piece
		int m_x;
		// y coordinates of the piece
		int m_y;
		// sign corresponding to piece's owner
		char m_sign;
		// list containing all evaluated possible moves for the piece
		std::list<available_move*>* m_av_list;
		// object that is printed in the window
		sf::CircleShape m_shape;
		// pointer to the piece's owner
		base_player* m_owner;

		bool m_is_alive;
	public:
		// creates the piece of given sign and coordinates
		piece(char sign, int x, int y, bool is_alive, base_player* owner);
		// copies the piece
		piece(const piece& piece);
		// deletes the piece
		virtual ~piece();
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

		bool is_alive(void);
		// returns the piece's owner
		base_player* get_owner(void);
		// sets and returns the piece's owner
		base_player* set_owner(base_player* owner);
		//
		sf::CircleShape& get_shape(void);
		// lets to print the piece in given stream
		friend std::ostream& operator<<(std::ostream& os, const piece* piece);
		// returns the list of all evaluated moves
		std::list<available_move*>* get_av_list(void);
		// draws the piece to the window
		void draw(sf::RenderWindow& window);

		friend class gui;
	};

	std::ostream& operator<<(std::ostream& os, const piece* piece);
}

#endif