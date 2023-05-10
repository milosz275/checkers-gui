#include "include/bot.h"

namespace checkers
{
	bot::bot(char sign, game* game) : base_player(sign, "Bot") { m_game = game; m_considered_moves_ahead = 3; m_x = -1; m_y = -1; m_counter = 0; }

	bot::~bot() {}

	std::tuple<int, int> bot::get_coordinates(void)
	{
		assert(m_game->m_current_player == this);
		++m_counter;
		if (m_counter > 3)
			throw std::runtime_error("Bot is stuck in a loop");

		if (m_game->m_selected_piece == NULL) // bot has not selected any piece yet
		{
			// best move evaluation
			// for each piece and its move copy the board and lists and evaluate saving iterations and score after
			// find one piece that corresponds to the best move and save capture coords in m_x, m_y

		}
		else // bot selected the piece and makes planned move
		{
			// reset the counter
			m_counter = 0;
#ifdef _DEBUG
			m_game->m_os << "Bot is making planned move" << std::endl;
#endif
			return std::make_tuple(m_x, m_y); 
		}


		return std::make_tuple(1, 8);
	}
}