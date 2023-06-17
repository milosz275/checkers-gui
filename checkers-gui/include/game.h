#ifndef GAME_H
#define GAME_H

#include "dependencies.h"
#include "global_variables.h"

namespace checkers
{
	class king;
	class game
	{
		// main game board
		std::vector<std::vector<piece*>>* m_board;
		//
		gui* m_gui;
		//
		event_handler* m_event_handler;
		//
		game_state* m_game_state;
		//
		bool m_game_freeze;
		//
		bool m_any_changes;
		//
		//bool m_signaled_bot;
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
		// default input stream
		std::istream& m_is = std::cin;
		// default output stream
		std::ostream& m_os = std::cout;
		// 
		std::ostream& m_log = std::clog;
		//
		std::ofstream m_log_file;
	protected:
		//
		void select_piece(void);
		//
		void move_selected_piece(void);
		//
		void move_piece(piece* piece_to_move, std::vector<std::vector<piece*>>* board, int x, int y);
		
		//
		void make_capture(std::vector<std::vector<piece*>>* board, piece* moving_piece, piece* delete_piece, int new_x, int new_y, std::list<piece*>* dead_list);
		//
		void delete_piece(piece* piece_to_delete, std::vector<std::vector<piece*>>* board, base_player* owner);
		// OLD - delete; check game completion respecting amount of pieces in the piece lists
		bool check_game_completion_no_pieces(void);
		// check game completion respecting list with pieces and its evaluated possible moves
		bool check_game_completion_no_possible_moves(std::list<piece*>* list);
		// copy pieces from one board to another
		void copy_board(std::vector<std::vector<piece*>>* source_board, std::vector<std::vector<piece*>>* copy_of_board, base_player* owner);
		// print basic info about the game
		void debug_info(std::ostream& os);
		// switches first_turn flag, indicating that it is move of the first player
		void switch_turn(void);
		// OLD - delete; turn on the flag for the bot
		void signal_the_bot(void);
		// gets move coordinates from the current player
		std::pair<int, int> get_coordinates(void);
		// gets coordinates of click through the gui
		std::pair<int, int> get_click_coordinates(void);
		// gets coordinates from game's input stream
		std::pair<int, int> get_coordinates_from_stream(void);
		// populates the board with pieces for each player
		void populate_board(int rows);
		// populates the board for testing purposes
		void populate_board_debug(void);
		// load pieces from file of a given name
		bool load_pieces_from_file(const std::string file_name);
		// save pieces to file of a given name
		void save_to_file(std::string file_name);
		// adds new piece to the specific piece list, board and player at wanted coords
		void add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y, bool is_alive, gui* ui = nullptr);
		//
		void add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y, bool is_alive, bool force_king, gui* ui = nullptr);
		//// adds new piece to the specific piece list, board and player based on given piece from other board
		//void add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, piece* based_on);
		// prints result to the given stream
		void print_results(std::ostream& os);
		// prints alive pieces to the given stream
		void print_pieces(std::list<piece*>* list);
		// evaluate all possible moves of the given player, returns true if there is at least on possible capture
		bool evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, base_player* player, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece);
		// evaluate possible moves of one piece
		bool evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, base_player* player);
		// evaluate possible moves of one king
		bool evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, base_player* player, int last_capture_direction, std::list<piece*>* dead_list);
		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<piece*>* list);
		// clears list of dead pieces printed in multicapture
		void clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list);
		// deletes given piece from given list
		void delete_from_list(std::list<piece*>* list, piece* piece_to_delete);

		std::list<available_move*>& get_available_moves(void);

		bool is_game_over(void);
	public:
		// create the game of given size and target frames per second
		game(int fps = 60, std::istream& is = std::cin, std::ostream& os = std::cout);
		// copies the game (without GUI)
		game(const game& game);
		// deletes the game
		~game();
		// executes the game
		void loop(void);
		//
		int get_score(void);
		//
		bool get_completion(void);
		//
		bool get_game_freeze(void);
		//
		bool get_available_capture(void);
		//
		const base_player* get_player_1(void);
		//
		const base_player* get_player_2(void);
		//
		const base_player* get_current_player(void);
		//
		const piece* get_selected_piece(void);
		//
		std::list<piece*>* get_pieces(void);
		// returns main game board
		std::vector<std::vector<piece*>>* get_board(void);
		//
		std::list<piece*>* get_to_delete_list(void);
		// returns board to the stream
		friend std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
		//
		friend class event_handler;
		friend class gamestate;
		friend class bot;
	};
	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
}

#endif
