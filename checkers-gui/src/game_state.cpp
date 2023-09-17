#include "game_state.h"
#include "piece.h"

namespace checkers
{
	game_state::game_state(std::ostream& os) : m_game_freeze(false), m_any_changes(false), m_first_won(false), m_second_won(false), m_current_player(nullptr), m_next_player(nullptr), m_completed(false), m_os(os),
		m_selected(false), m_selected_piece(nullptr), m_moving_piece(nullptr), m_available_capture(false) {}

	game_state::game_state(const game_state& source_game_state) : m_game_freeze(source_game_state.m_game_freeze), m_any_changes(source_game_state.m_any_changes), m_first_won(source_game_state.m_first_won), m_second_won(source_game_state.m_second_won),
		m_current_player(nullptr), m_next_player(nullptr), m_completed(source_game_state.m_completed), m_os(source_game_state.m_os)
	{
		
	}

	game_state::~game_state() {}

	void game_state::switch_turn(void)
	{
		base_player* tmp = m_current_player;
		m_current_player = m_next_player;
		m_next_player = tmp;
		
		//m_selected = false;
		//m_selected_piece = nullptr;
		//m_moving_piece = nullptr;
#ifdef _DEBUG
		m_os << "Switched turn" << std::endl;
#endif
	}

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

	bool game_state::check_completion(void)
	{
		if (m_completed)
		{
#ifdef _DEBUG
			m_os << "Checking completion: game already completed!" << std::endl;
#endif
			return true;
		}
		else if (m_first_won || m_second_won)
		{
#ifdef _DEBUG
			m_os << "Checking completion: Flags already set. Setting completed" << std::endl;
#endif
			m_completed = true;
			return true;
		}
		else
			return false;
	}

	bool game_state::check_lists(void)
	{
		if (m_completed)
		{
#ifdef _DEBUG
			m_os << "Game state: check lists: game is already completed" << std::endl;
#endif
			return true;
		}
		if (m_first_won || m_second_won)
		{
#ifdef _DEBUG
			m_os << "Game state: check lists: at least one flag is already set to win" << std::endl;
#endif
			return true;
		}

		// piece count
		bool at_least_one_piece_1 = false;
		bool at_least_one_piece_2 = false;

		if (m_current_player->is_first())
		{
			at_least_one_piece_1 = m_current_player->get_list()->empty();
			at_least_one_piece_2 = m_next_player->get_list()->empty();
		}
		else
		{
			at_least_one_piece_1 = m_next_player->get_list()->empty();
			at_least_one_piece_2 = m_current_player->get_list()->empty();
		}

		if (!at_least_one_piece_1)
			m_second_won = true;
		if (!at_least_one_piece_2)
			m_first_won = true;
		if (!at_least_one_piece_1 && !at_least_one_piece_2)
			m_os << "WARNING: Setting two winning flags at the same time" << std::endl;
		if (!at_least_one_piece_1 || !at_least_one_piece_2)
		{
			m_completed = true;
			return true;
		}

		// moves
		bool at_least_one_move = false;
		assert(m_current_player);
		for (piece* p : *m_current_player->get_list())
		{
			if (!p->get_av_list()->empty())
			{
				at_least_one_move = true;
				break;
			}
		}
		if (!at_least_one_move)
		{
			m_os << "Checking completion: No more moves possible. Setting completed" << std::endl;
			if (m_current_player->is_first())
				m_second_won = true;
			else
				m_first_won = true;
			m_completed = true;
			return true;
		}
		return m_completed;
	}

	void game_state::reset_completion(void) { m_first_won = false; m_second_won = false; m_completed = false; }
	
	void game_state::reset_state(void) { m_game_freeze = false; m_any_changes = true; }

	bool game_state::is_console_game(void) { return m_console_game; }

	bool game_state::get_selected(void) { return m_selected; }

	bool game_state::set_selected(bool flag) { return m_selected = flag; }

	piece* game_state::get_selected_piece(void) { return m_selected_piece; }

	piece* game_state::set_selected_piece(piece* p) { return m_selected_piece = p; }

	piece* game_state::get_moving_piece(void) { return m_moving_piece; }

	piece* game_state::set_moving_piece(piece* p) { return m_moving_piece = p; }

	bool game_state::get_available_capture(void) { return m_available_capture; }

	bool game_state::set_available_capture(bool flag) { return m_available_capture = flag; }
}