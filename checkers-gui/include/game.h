#ifndef GAME_H
#define GAME_H

#include "dependencies.h"
#include "global_variables.h"

const int bot_search_depth = 1;

namespace checkers
{
	class king;

	/**
	 * @brief Class representing game
	 * 
	 * This class is used to represent the game of checkers
	 * 
	 * @see game::game
	 * 		- constructor
	 * @see game::game
	 * 		- copy constructor
	 * @see game::~game
	 * 		- destructor
	 * @see game::get_gui
	 * 		- returns gui
	 * @see game::get_event_handler
	 * 		- returns event handler
	 * @see game::get_game_state
	 * 		- returns game state
	 * @see game::loop
	 * 		- executes the game
	 * @see game::get_is_finished
	 * 		- returns flag indicating if the game is finished
	 * @see game::get_score
	 * 		- returns current game score (white pieces - black pieces)
	 * @see game::get_last_capture_direction
	 * 		- returns integer representing direction of last capture
	 * @see game::set_last_capture_direction
	 * 		- sets and returns integer representing direction of last capture
	 * @see game::get_player_1
	 * 		- returns first player (lower)
	 * @see game::get_player_2
	 * 		- returs second player (upper)
	 * @see game::set_player_1
	 * 		- sets and returns player 1
	 * @see game::set_player_2
	 * 		- sets and returns player 2
	 * @see game::get_list_1
	 * 		- returns piece list of player 1
	 * @see game::get_list_2
	 * 		- returns piece list of player 2
	 * @see game::get_selected
	 * 		- returns flag if anything was selected
	 * @see game::set_selected
	 * 		- sets and returns flag if anything was selected
	 * @see game::get_selected_piece
	 * 		- returns piece selected to move
	 * @see game::set_selected_piece
	 * 		- sets and returns piece selected to move
	 * @see game::get_moving_piece
	 * 		- returns moving piece
	 * @see game::set_moving_piece
	 * 		- sets and returns moving piece
	 * @see game::get_available_capture
	 * 		- returns currently evaluated possibility for at least one possible capture
	 * @see game::set_available_capture
	 * 		- sets and returns evaluation for the game
	 * @see game::get_pieces
	 * 		- returns list of pieces of the current player
	 * @see game::get_board
	 * 		- returns main game board
	 * @see game::get_to_delete_list
	 * 		- returns list of dead pieces (pieces captured during multicapture)
	 * @see game::get_os
	 * 		- returns output stream
	 * @see game::get_log
	 * 		- returns log stream
	 * @see game::get_log_file
	 * 		- returns reference to opened log file
	 * @see game::move_piece
	 * 		- runs a partial method of moving one specific piece
	 * @see game::make_capture
	 * 		- makes a singular capture using move_piece and delete_piece methods
	 * @see game::evaluate
	 * 		- evaluate all possible moves of the given player, returns true if there is at least on possible capture
	 * @see game::clear_list
	 * 		- clears available moves list for every piece in pieces list (gets through lists in list)
	 * @see game::clear_to_delete_list
	 * 		- clears list of dead pieces printed in multicapture
	 * @see game::print_results
	 * 		- prints result to the given stream
	 * @see game::print_pieces
	 * 		- prints alive pieces to the given stream
	 * @see game::debug_info
	 * 		- print basic info about the game
	 * @see game::debug_ptrs
	 * 		- checks pointers
	 * @see game::debug_setup
	 * 		- delivers game ready for debugging
	 * @see game::event_handler
	 * 		- gives the handler full access to the game
	 * @see operator<<
	 * 		- returns board to the stream
	 */
	class game
	{
		// main game board
		std::vector<std::vector<piece*>>* m_board;
		// graphical user interface
		gui* m_gui;
		// event handler
		event_handler* m_event_handler;
		// state of the game
		game_state* m_game_state;
		// flag indicating if the game is played in console
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
		// flag indicating if anything selected
		bool m_selected;
		// pointer to selected piece on the board
		piece* m_selected_piece;
		// pointer to piece moving in the last move
		piece* m_moving_piece;
		// flag indicating if there is one or more captures, not allowing other moves
		bool m_available_capture;
		// integer representing last capture direction
		int m_last_capture_direction;
		// frames per second in the game
		const int m_fps;
		// default input stream
		std::istream& m_is = std::cin;
		// default output stream
		std::ostream& m_os = std::cout;
		// default log stream
		std::ostream& m_log = std::clog;
		// contains opened log file
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


		// automatic piece handling:

		// is runned by event handler, selects piece using input call methods
		void select_piece(void);
		// is runned by event handler, moves already selected piece using input methods
		void move_selected_piece(void);
		// deletes given piece from given list
		void delete_from_list(std::list<piece*>* list, piece* piece_to_delete);


		// handle desirable input:

		// gets move coordinates from the current player
		std::pair<int, int> get_coordinates(void);
		// gets coordinates of click through the gui
		std::pair<int, int> get_click_coordinates(void);
		// gets coordinates from game's input stream
		std::pair<int, int> get_coordinates_from_stream(void);
		
		
		// protected section of evaluation:
		
		// evaluate possible moves of one piece
		bool evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive);
		// evaluate possible moves of one king
		bool evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list);

	public:
		// create the game of given size and target frames per second
		game(int fps = 60, std::istream& is = std::cin, std::ostream& os = std::cout);
		// copies the game (without GUI)
		game(const game& game);
		// deletes the game
		~game();

		// returns gui
		gui* get_gui(void);
		// returns event handler
		event_handler* get_event_handler(void);
		// returns game state
		game_state* get_game_state(void);
		// executes the game
		void loop(void);
		// returns flag indicating if the game is finished
		bool get_is_finished(void);
		// returns current game score (white pieces - black pieces)
		int get_score(void);
		// returns integer representing direction of last capture
		int get_last_capture_direction(void);
		// sets and returns integer representing direction of last capture
		int set_last_capture_direction(int direction);
		// returns first player (lower)
		base_player* get_player_1(void);
		// returs second player (upper)
		base_player* get_player_2(void);
		// sets and returns player 1
		base_player* set_player_1(base_player* player);
		// sets and returns player 2
		base_player* set_player_2(base_player* player);
		//returns piece list of player 1
		std::list<piece*>& get_list_1(void);
		// returns piece list of player 2
		std::list<piece*>& get_list_2(void);
		// returns flag if anything was selected
		bool get_selected(void);
		// sets and returns flag if anything was selected
		bool set_selected(bool flag);
		// returns piece selected to move
		piece* get_selected_piece(void);
		// sets and returns piece selected to move
		piece* set_selected_piece(piece* p);
		// returns moving piece
		piece* get_moving_piece(void);
		// sets and returns moving piece
		piece* set_moving_piece(piece* p);
		// returns currently evaluated possibility for at least one possible capture
		bool get_available_capture(void);
		// sets and returns evaluation for the game
		bool set_available_capture(bool flag);
		// returns list of pieces of the current player
		std::list<piece*>* get_pieces(void);
		// returns main game board
		std::vector<std::vector<piece*>>* get_board(void);
		// returns list of dead pieces (pieces captured during multicapture)
		std::list<piece*>* get_to_delete_list(void);
		// returns output stream
		std::ostream& get_os(void);
		// returns log stream
		std::ostream& get_log(void);
		// returns reference to opened log file
		std::ofstream& get_log_file(void);


		// public game management:

		// runs a partial method of moving one specific piece
		void move_piece(piece* piece_to_move, std::vector<std::vector<piece*>>* board, int x, int y);
		// makes a singular capture using move_piece and delete_piece methods
		void make_capture(std::vector<std::vector<piece*>>* board, piece* moving_piece, piece* delete_piece, int new_x, int new_y, std::list<piece*>* dead_list);
		// evaluate all possible moves of the given player, returns true if there is at least on possible capture
		bool evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece);
		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<piece*>* list);
		// clears list of dead pieces printed in multicapture
		void clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list);


		// debug section:

		// prints result to the given stream
		void print_results(std::ostream& os);
		// prints alive pieces to the given stream
		void print_pieces(std::list<piece*>* list);
		// print basic info about the game
		void debug_info(std::ostream& os);
		// checks pointers
		void debug_ptrs(void);
		// delivers game ready for debugging
		void debug_setup(void);

		// gives the handler full access to the game
		friend class event_handler;
		// returns board to the stream
		friend std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
	};
	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board);
}

#endif
