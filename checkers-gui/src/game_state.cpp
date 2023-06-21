#include "include/game_state.h"

namespace checkers
{
	game_state::game_state() : m_game_freeze(false), m_any_changes(false), m_first_won(false), m_second_won(false), m_current_player(nullptr), m_next_player(nullptr) {}

	game_state::game_state(const game_state& source_game_state) : m_game_freeze(source_game_state.m_game_freeze), m_any_changes(source_game_state.m_any_changes), m_first_won(source_game_state.m_first_won), m_second_won(source_game_state.m_second_won), m_current_player(nullptr), m_next_player(nullptr) {}

	game_state::~game_state() {}

	void game_state::switch_turn(void) { base_player* tmp = m_current_player; m_current_player = m_next_player; m_next_player = tmp; }

	base_player* game_state::set_current_player(base_player* player) { assert(m_current_player != player); return m_current_player = player; }
	
	base_player* game_state::get_current_player(void) { return m_current_player; }

	base_player* game_state::set_next_player(base_player* player) { assert(m_next_player != player); return m_next_player = player; }

	base_player* game_state::get_next_player(void) { return m_next_player; }
	
	bool game_state::set_first_won(bool flag) { return m_first_won = flag; }
	
	bool game_state::get_first_won(void) { return m_first_won; }
	
	bool game_state::set_second_won(bool flag) { return m_second_won = flag; }
	
	bool game_state::get_second_won(void) { return m_second_won; }
	
	bool game_state::set_game_freeze(bool flag) { return m_game_freeze = flag; }
	
	bool game_state::get_game_freeze(void) { return m_game_freeze; }
	
	bool game_state::set_any_changes(bool flag) { return m_any_changes = flag; }
	
	bool game_state::get_any_changes(void) { return m_any_changes; }

	bool game_state::check_completion(void) { if (m_first_won || m_second_won) return m_completed = true; }

	bool game_state::reset_completion(void) { m_first_won = false; m_second_won = false; }
	
	bool game_state::reset_state(void) { m_game_freeze = false; m_any_changes = true; }
}