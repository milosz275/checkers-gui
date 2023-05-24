#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cassert>
#include <tuple>

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

	// declare to avoid compile conflict
	class king;

	class game
	{
		// main game board
		std::vector<std::vector<piece*>>* m_board;
		// indicates if the players should use stream to choose pieces and moves
		bool m_console_game;
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
		// list of pieces to delete after multicapture (combo), indicates multicapture
		std::list<piece*> m_to_delete_list;
		// flag indicating finished game
		bool m_is_finished;
		// flag indicating that first player won
		bool m_first_won;
		// flag indicating that second player won
		bool m_second_won;
		//
		bool m_selected;
		// pointer to selected piece on the board
		piece* m_selected_piece;
		// pointer to piece moving in the last move
		piece* m_moving_piece;
		// flag indicating if there is one or more captures, not allowing other moves
		bool m_available_capture;
		//
		int m_last_capture_direction;
		// frames per second in the game
		const int m_fps;
		// fraction of second that is time of one frame
		float m_frame_duration = 1.0f / m_fps;
		// default input stream
		std::istream& m_is = std::cin;
		// default output stream
		std::ostream& m_os = std::cout;
		// SFML object drawing checkerboard
		std::vector<std::vector<sf::RectangleShape>> m_tiles;
		// SFML clock
		sf::Clock m_clock;
		// SFML settings
		sf::ContextSettings m_settings;
		// SFML window
		sf::RenderWindow m_window;
		// SFML event
		sf::Event m_event;

	public:
		// create the game of given size and target frames per second
		game(int fps = 24, std::istream& is = std::cin, std::ostream& os = std::cout);
		// copies the game (without GUI)
		game(const game& game);
		// deletes the game
		~game();

		// todo
		void handle_events(void);
		//
		void draw_board(void);
		//
		void select_piece(void);
		
		//
		void move_piece(piece* piece_to_move, std::vector<std::vector<piece*>>* board, int x, int y);
		
		//
		void make_capture(std::vector<std::vector<piece*>>* board, piece* moving_piece, piece* delete_piece, int new_x, int new_y, std::list<piece*>* dead_list);
		//
		void delete_piece(piece* piece_to_delete, std::vector<std::vector<piece*>>* board, base_player* owner);
		//
		void check_game_completion(void);
		
		//
		void copy_board(std::vector<std::vector<piece*>>* source_board, std::vector<std::vector<piece*>>* copy_of_board, base_player* owner);

		//
		void debug_info(void);

		// switches first_turn flag, indicating that it is move of the first player
		void switch_turn(void);
		// returns main game board
		std::vector<std::vector<piece*>>* get_board(void);
		
		////
		//std::vector<std::vector<piece*>>* copy_board(std::vector<std::vector<piece*>>* source_board);

		
		// gets move coordinates from the current player
		std::tuple<int, int> get_coordinates(void);
		// gets coordinates of click in the window
		std::tuple<int, int> get_click_coordinates(void);
		
		
		// gets coordinates from game's input stream
		std::tuple<int, int> get_coordinates_from_stream(void);
		// populates the board with pieces for each player
		void populate_board(int rows);
		// populates the board for testing purposes
		void populate_board_debug(void);
		// adds new piece to the specific piece list, board and player at wanted coords
		void add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y, bool is_alive);
		// adds new piece to the specific piece list, board and player based on given piece from other board
		void add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, piece* based_on);
		//
		void test_loop(void);
		// executes the game
		void loop(void);
		// prints result to the given stream
		void print_results(std::ostream& os);
		// prints alive pieces to the given stream
		void print_pieces(std::list<piece*>* list);
		// draws main game board in the given window
		void draw(sf::RenderWindow& window);
		// highlights selected piece of given coords (brown)
		void highlight_selected(sf::RenderWindow& window, int x, int y);
		// higlight selected piece of given coords (green)
		void highlight_available(sf::RenderWindow& window, int x, int y);
		
		// evaluate possible moves of the given player, returns true if there is at least on possible capture
		bool evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, base_player* player, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece);
		// 
		bool evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, base_player* player);
		//
		bool evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, base_player* player, int last_capture_direction, std::list<piece*>* dead_list);

		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<piece*>* list);
		// clears list of dead pieces printed in multicapture
		void clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list);
		// deletes given piece from given list
		void delete_from_list(std::list<piece*>* list, piece* piece_to_delete);

		friend std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
		friend class bot;
	};
	// returns board to the stream
	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
}

#endif
