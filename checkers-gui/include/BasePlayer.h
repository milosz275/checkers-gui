#ifndef BASEPLAYER_H
#define BASEPLAYER_H
#include <iostream>
#include <string>
#include <algorithm>
#include <list>

namespace Checkers
{
	class BasePlayer
	{
		// sign distinguishing the player from others
		char sign;
		// nickname of the player
		std::string name;
		// count of pieces of the player
		int pieces;
		// count of pieces which has been already taken
		int captured_pieces;
		// flag indicating the right to move again after capture
		bool combo;
	public:
		// returns player's sign
		char get_sign(void);
		// returns player nickname
		std::string get_name(void);
		// increases piece count by x
		void add_piece(int x = 1);
		// returns living pieces 
		int get_pieces(void);
		// returns dead pieces
		int get_captured_pieces(void);
		// returns if the player can move again
		bool get_combo(void);
		// sets the combo flag and returns it
		bool set_combo(bool c);
		// decreases pieces count, increases captured pieces count
		void capture(void);
		// prints pieces of the player
		void print_player(void);
		// creates the player of a given sign and name
		BasePlayer(char s, std::string n);
		// deletes player
		virtual ~BasePlayer();
	};
}

#endif