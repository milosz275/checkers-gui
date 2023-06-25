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

		bool m_console_game;
		// player 1
		base_player* m_player_1;
		// player 2
		base_player* m_player_2;

		// piece list of player 1
		std::list<piece*> m_p_list_1;
		// piece list of player 2
		std::list<piece*> m_p_list_2;
		// list of pieces to delete after multicapture (combo), indicates multicapture
		std::list<piece*> m_to_delete_list;
		// flag indicating finished game
		bool m_is_finished;
		// flag indicating that first player won

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
		// handling pieces:

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
		// adds new piece to the specific piece list, board and player at wanted coords and specifies gui to be displayed on
		void add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y, bool is_alive, bool force_king, gui* ui = nullptr);
		//
		void delete_piece(piece* piece_to_delete, std::vector<std::vector<piece*>>* board, base_player* owner);
		// copy pieces from one board to another
		void copy_board(std::vector<std::vector<piece*>>* source_board, std::vector<std::vector<piece*>>* copy_of_board, base_player* owner_1, base_player* owner_2);


		// handling elementary piece action:

		//
		void move_piece(piece* piece_to_move, std::vector<std::vector<piece*>>* board, int x, int y);
		//
		void make_capture(std::vector<std::vector<piece*>>* board, piece* moving_piece, piece* delete_piece, int new_x, int new_y, std::list<piece*>* dead_list);
		//
		void select_piece(void);
		//
		void move_selected_piece(void);
		


		// handle desirable input:

		// gets move coordinates from the current player
		std::pair<int, int> get_coordinates(void);
		// gets coordinates of click through the gui
		std::pair<int, int> get_click_coordinates(void);
		// gets coordinates from game's input stream
		std::pair<int, int> get_coordinates_from_stream(void);
		
		
		
		// handle game logic:
		
		// evaluate all possible moves of the given player, returns true if there is at least on possible capture
		bool evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece);
		// evaluate possible moves of one piece
		bool evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive);
		// evaluate possible moves of one king
		bool evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list);
		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<piece*>* list);
		// clears list of dead pieces printed in multicapture
		void clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list);
		// deletes given piece from given list
		void delete_from_list(std::list<piece*>* list, piece* piece_to_delete);

		// tmp
		bool check_game_completion_no_possible_moves(std::list<piece*>* list);


		

	public:
		// create the game of given size and target frames per second
		game(int fps = 60, std::istream& is = std::cin, std::ostream& os = std::cout);
		// copies the game (without GUI)
		game(const game& game);
		// deletes the game
		~game();

		// 
		gui* get_gui(void);
		//
		event_handler* get_event_handler(void);
		//
		game_state* get_game_state(void);
		//
		void select_piece(int x, int y);
		//
		void move_selected_piece(int x, int y);
		// executes the game
		void loop(void);
		// returns current game score (white pieces - black pieces)
		int get_score(void);
		//
		int get_last_capture_direction(void);
		//
		int set_last_capture_direction(int direction);
		// returns first player (lower)
		base_player* get_player_1(void);
		// returs second player (upper)
		base_player* get_player_2(void);
		//
		bool get_selected(void);
		//
		bool set_selected(bool flag);
		// returns piece selected to move
		piece* get_selected_piece(void);
		//
		piece* set_selected_piece(piece* p);
		// returns moving
		piece* get_moving_piece(void);
		//
		piece* set_moving_piece(piece* p);
		// returns currently evaluated possibility for at least one possible capture
		bool get_available_capture(void);
		//
		bool set_available_capture(bool flag);
		// returns list of pieces of the current player
		std::list<piece*>* get_pieces(void);
		// returns main game board
		std::vector<std::vector<piece*>>* get_board(void);
		// returns list of dead pieces (pieces captured during multicapture)
		std::list<piece*>* get_to_delete_list(void);
		//
		std::ostream& get_os(void);
		//
		std::ostream& get_log(void);
		

		// debug section:

		// prints result to the given stream
		void print_results(std::ostream& os);
		// prints alive pieces to the given stream
		void print_pieces(std::list<piece*>* list);
		// print basic info about the game
		void debug_info(std::ostream& os);

		//
		friend class event_handler;
		// returns board to the stream
		friend std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
	};
	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
}

#endif
