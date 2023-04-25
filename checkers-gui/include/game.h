#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#include "include/base_player.h"
#include "include/player.h"
#include "include/bot.h"
#include "include/piece.h"
#include "include/available_capture.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

namespace checkers
{
	// board size
	static const int s_size = 10;
	// map containing letters and their corresponding integers
	static std::map<char, int> s_coords{ {'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}, {'e', 5}, {'f', 6}, {'g', 7}, {'h', 8}, {'i', 9}, {'j', 10} };
	// square/tile size
	static const float s_square_size = 100.0;
	// radius of one piece
	static const float s_radius = s_square_size / 2.5;

	class king;

	class game
	{
		// main game board
		std::vector<std::vector<piece*>>* m_board;
		// player 1
		base_player* m_player_1;
		// player 2
		base_player* m_player_2;
		// pointer to player making move
		base_player* m_current_player;
		// piece list of player 1
		std::list<piece*> m_p_list_1;
		// piece list of player 2
		std::list<piece*> m_p_list_2;
		// list of pieces to delete after multicapture (combo)
		std::list<piece*> m_to_delete_list;
		// flag indicating finished game
		bool m_is_finished;
		// flag indicating that first player won
		bool m_first_won;
		// flag indicating that second player won
		bool m_second_won;
		// pointer to selected piece on the board
		piece* m_selected_piece;
		// flag indicating if there is one or more captures, not allowing other moves
		bool m_available_capture;
		//
		const int m_fps;
		//
		float m_frame_duration = 1.0f / m_fps;
		//
		sf::RectangleShape m_tile;
		// 
		sf::Clock m_clock;
		// 
		sf::ContextSettings m_settings;
		//
		sf::RenderWindow m_window;
		//
		sf::Event m_event;

	public:
		
		// create the game of given size and target frames per second
		game(int fps = 12);
		// deletes the game
		~game();
		// rotates the vector of vectors board, sets the is rotated flag to opposite
		//void rotate_board(void);
		// switches first_turn flag, indicating that it is move of the first player
		void switch_turn(void);
		// returns main game board
		std::vector<std::vector<piece*>>* get_board(void);
		// executes the game
		void loop(void);
		// prints result to given stream
		void print_results(std::ostream& os = std::cout);
		//
		void print_pieces(std::list<piece*>* list, std::ostream& os = std::cout);
		// draws main game board in the given window
		void draw(sf::RenderWindow& window);
		// highlights selected piece of given coords (brown)
		void highlight_selected(sf::RenderWindow& window, int x, int y);
		// higlight selected piece of given coords (green)
		void highlight_available(sf::RenderWindow& window, int x, int y);

		// evaluate possible moves of the given player, returns true if there is at least on possible capture
		bool evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board_p, int* counter, base_player* player);
		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<piece*>* list);
		// 
		void clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list);
		// deletes given piece from given list
		void delete_from_list(std::list<piece*>* list, piece* piece_to_delete);

		friend std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
	};
	// returns board to the stream
	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
}

#endif
