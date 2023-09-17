#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "base_player.h"

namespace checkers
{
	class game_state
	{
		// flag indicating if the game is frozen
		bool m_game_freeze;
		// flag indicating anything changed in the game
		bool m_any_changes;
		// flag indicating that first player won
		bool m_first_won;
		// flag indicating that second player won
		bool m_second_won;
		// flag indicating if the game is completed
		bool m_completed;
		// flag indicating if the game is played in console
		bool m_console_game = false;
		// flag indicating if anything selected
		bool m_selected;
		// pointer to selected piece on the board
		piece* m_selected_piece;
		// pointer to piece moving in the last move
		piece* m_moving_piece;
		// flag indicating if there is one or more captures, not allowing other moves
		bool m_available_capture;
		// pointer to player making move
		base_player* m_current_player;
		// pointer to player making move
		base_player* m_next_player;
		// reference to the output stream
		std::ostream& m_os;

	public:
		// create the game state
		game_state(std::ostream& os);
		// copies the game state
		game_state(const game_state& source_game_state);
		// deletes the game state
		~game_state();
		// switches first_turn flag, indicating that it is move of the first player
		void switch_turn(void);
		// sets and returns current player
		base_player* set_current_player(base_player* player);
		// returns current player
		base_player* get_current_player(void);
		// sets and returns next players
		base_player* set_next_player(base_player* player);
		// returns next players
		base_player* get_next_player(void);
		// sets and returns flag indicating that the first player won
		bool set_first_won(bool flag);
		// returns flag indicating that the first player won
		bool get_first_won(void);
		// sets and returns flag indicating that the second player won
		bool set_second_won(bool flag);
		// returns flag indicating that the second player won
		bool get_second_won(void);
		// sets and returns flag indicating if the game is frozen
		bool set_game_freeze(bool flag);
		// returns flag indicating if the game is frozen
		bool get_game_freeze(void);
		// sets and returns flag indicating that there were changes to the game
		bool set_any_changes(bool flag);
		// returns flag indicating that there were changes to the game
		bool get_any_changes(void);
		// checks, if the flags were set to winning 
		bool check_completion(void);
		// checks, if the lists of pieces are empty
		bool check_lists(void);
		// resets game completion along with winning flags
		void reset_completion(void);
		// resets game freeze and sets flag indicating changes to true
		void reset_state(void);
		// returns if the game is played in console
		bool is_console_game(void);
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
	};
}

#endif