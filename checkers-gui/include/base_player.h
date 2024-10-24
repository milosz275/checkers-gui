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
	class piece;
	class king;

	/**
	 * @brief Class representing player in checkers game
	 * 
	 * This class is used to store information about player in checkers game
	 * 
	 * @see base_player::base_player
	 * 		- constructor
	 * @see base_player::base_player
	 * 		- copy constructor
	 * @see base_player::~base_player
	 * 		- destructor
	 * @see base_player::get_coordinates
	 * 		- loads coordinates
	 * @see base_player::get_sign
	 * 		- returns player's sign
	 * @see base_player::get_name
	 * 		- returns player nickname
	 * @see base_player::is_first
	 * 		- returns if a player is the first, meaning starts from the bottom of the board
	 * @see base_player::set_first
	 * 		- sets and returns flag indicating if a player is first
	 * @see base_player::get_list
	 * 		- returns the list of possible moves of the player
	 * @see base_player::set_list
	 * 		- sets and returns the list of possible moves of the player
	 * @see base_player::get_pieces
	 * 		- returns living pieces 
	 * @see base_player::get_captured_pieces
	 * 		- returns dead pieces
	 * @see base_player::set_captured_pieces
	 * 		- sets and returns dead pieces
	 * @see base_player::get_combo
	 * 		- returns if the player can move again
	 * @see base_player::set_combo
	 * 		- sets the combo flag and returns it
	 * @see base_player::get_kings
	 * 		- returns kings count
	 * @see base_player::add_capture
	 * 		- decreases pieces count, increases captured pieces count
	 * @see base_player::change_to_king
	 * 		- checks if the target piece could change into king and does it, if yes
	 * @see base_player::print_player
	 * 		- prints pieces of the player
	 * @see operator<<
	 * 		- prints player
	 */
	class base_player
	{
		// sign distinguishing the player from others
		char m_sign;
		// nickname of the player
		std::string m_name;
		// count of pieces which has been already taken
		int m_captured_pieces;
		// flag indicating the right to move again after capture
		bool m_combo;
		// flag indicating if the first player is currently making a move
		bool m_is_first;
		// pointer to player's list of pieces
		std::list<piece*>* m_piece_list;
	public:
		// creates the player of a given sign and name
		base_player(char sign, std::string name);
		// copies the player
		base_player(const base_player& player);
		// deletes player
		virtual ~base_player();
		// loads coordinates
		virtual std::pair<int, int> get_coordinates(void) = 0;
		// returns player's sign
		char get_sign(void);
		// returns player nickname
		std::string get_name(void);
		// returns if a player is the first, meaning starts from the bottom of the board
		bool is_first(void);
		// sets and returns flag indicating if a player is first
		bool set_first(bool is_first);
		// returns the list of possible moves of the player
		std::list<piece*>* get_list(void);
		// sets and returns the list of possible moves of the player
		std::list<piece*>* set_list(std::list<piece*>* piece_list);
		// returns living pieces 
		int get_pieces(void);
		// returns dead pieces
		int get_captured_pieces(void);
		// sets and returns dead pieces
		int set_captured_pieces(int pieces);
		// returns if the player can move again
		bool get_combo(void);
		// sets the combo flag and returns it
		bool set_combo(bool combo);
		// returns kings count
		int get_kings(void);
		// decreases pieces count, increases captured pieces count
		void add_capture(void);
		// checks if the target piece could change into king and does it, if yes
		bool change_to_king(piece* target, std::vector<std::vector<piece*>>* board);
		// prints pieces of the player
		void print_player(std::ostream& os = std::cout);

		friend std::ostream& operator<<(std::ostream& os, const base_player* player);
	};

	std::ostream& operator<<(std::ostream& os, const base_player* player);
}

#endif
