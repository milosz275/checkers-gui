#ifndef BASEPLAYER_H
#define BASEPLAYER_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <cassert>

namespace checkers
{
	// declare to avoid compile conflict
	class piece;
	class king;

	class base_player
	{
		// sign distinguishing the player from others
		char m_sign;
		// nickname of the player
		std::string m_name;
		// count of pieces of the player
		int m_pieces;
		// count of pieces which has been already taken
		int m_captured_pieces;
		// flag indicating the right to move again after capture
		bool m_combo;
		// flag indicating if the first player is currently making a move
		bool m_is_first;
		// pointer to the next player (next round)
		base_player* m_next_player;
		// pointer to player's list of pieces
		std::list<piece*>* m_piece_list;
	public:
		// creates the player of a given sign and name
		base_player(char sign, std::string name);
		// deletes player
		virtual ~base_player();
		// loads coordinates
		virtual std::tuple<int, int> get_coordinates(void) = 0;
		// returns player's sign
		char get_sign(void);
		// returns player nickname
		std::string get_name(void);
		// returns if a player is the first, meaning starts from the bottom of the board
		bool is_first(void);
		// sets and returns flag indicating if a player is first
		bool set_first(bool is_first);
		// returns pointer to the next player (next round)
		base_player* get_next_player(void);
		// sets and returns pointer to the next player
		base_player* set_next_player(base_player* next_player);
		// returns the list of possible moves of the player
		std::list<piece*>* get_list(void);
		// sets and returns the list of possible moves of the player
		std::list<piece*>* set_list(std::list<piece*>* piece_list);
		// increases piece count by x
		void add_piece(int count = 1);
		// returns living pieces 
		int get_pieces(void);
		// returns dead pieces
		int get_captured_pieces(void);
		// returns if the player can move again
		bool get_combo(void);
		// sets the combo flag and returns it
		bool set_combo(bool combo);
		// decreases pieces count, increases captured pieces count
		void make_capture(void);
		// checks if the target piece could change into king and does it, if yes
		void change_to_king(piece* target, std::vector<std::vector<piece*>>* board);
		// prints pieces of the player
		void print_player(std::ostream& os = std::cout);
	};
}

#endif