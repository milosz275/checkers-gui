#ifndef PIECE_H
#define PIECE_H

#include <iostream>
#include <list>

#include "game.h"
#include "available_move.h"
#include "sfml.h"

namespace checkers
{
	class gui;
	class piece
	{
	protected:
		// x coordinate of the piece
		int m_x;
		// y coordinates of the piece
		int m_y;
		// list containing all evaluated possible moves for the piece
		std::list<available_move*>* m_av_list;
		// object that is printed in the window
		sfml::circle_shape* m_shape = nullptr;
		// pointer to the piece's owner
		base_player* m_owner;
		// flag indicating if the piece is not already captured
		bool m_is_alive;
		// pointer to gui where the piece should be shown
		gui* m_gui = nullptr;

	public:
		// creates the piece with given coordinates
		piece(int x, int y, bool is_alive, base_player* owner);
		// creates the piece with given coordinates and gui
		piece(int x, int y, bool is_alive, base_player* owner, gui* gui);
		// copies the piece
		piece(const piece& piece);
		// deletes the piece
		virtual ~piece();
		// setups visual traits of the piece
		virtual void setup_shape(void);
		// returns x coordinate of the piece
		int get_x(void);
		// returns y coordinate of the piece
		int get_y(void);
		// sets x to x1 and returns it
		int set_x(int x);
		// sets y to y1 and returns it
		int set_y(int y);
		// returns if the piece is alive
		bool is_alive(void);
		// returns the piece's owner
		base_player* get_owner(void);
		// sets and returns the piece's owner
		base_player* set_owner(base_player* owner);
		// returns the shape of the piece
		sfml::circle_shape& get_shape(void);
		// returns the list of all evaluated moves
		std::list<available_move*>* get_av_list(void);
		// returns gui of the piece
		gui* get_gui(void);
		// draws the piece to the window
		void draw(void);

		// lets to print the piece in given stream
		friend std::ostream& operator<<(std::ostream& os, const piece* piece);
	};

	std::ostream& operator<<(std::ostream& os, const piece* piece);
}

#endif