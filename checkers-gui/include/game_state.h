#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "base_player.h"

namespace checkers
{
	class game_state
	{
		//
		bool m_game_freeze;
		//
		bool m_any_changes;
		//
		bool m_first_won;
		// flag indicating that second player won
		bool m_second_won;
		//
		bool m_completed;
		// pointer to player making move
		base_player* m_current_player;
		// pointer to player making move
		base_player* m_next_player;
	public:
		//
		game_state();
		//
		game_state(const game_state& source_game_state);
		//
		~game_state();
		// switches first_turn flag, indicating that it is move of the first player
		void switch_turn(void);
		//
		base_player* set_current_player(base_player* player);
		//
		base_player* get_current_player(void);
		//
		base_player* set_next_player(base_player* player);
		//
		base_player* get_next_player(void);
		//
		bool set_first_won(bool flag);
		//
		bool get_first_won(void);
		//
		bool set_second_won(bool flag);
		//
		bool get_second_won(void);
		//
		bool set_game_freeze(bool flag);
		//
		bool get_game_freeze(void);
		//
		bool set_any_changes(bool flag);
		//
		bool get_any_changes(void);
		//
		bool check_completion(void);
		//
		void reset_completion(void);
		//
		void reset_state(void);
	};
}

#endif