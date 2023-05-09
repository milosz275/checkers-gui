#include "include/bot.h"

namespace checkers
{
	bot::bot(char sign, game* game) : base_player(sign, "Bot") { m_game = game; m_considered_moves_ahead = 3; m_x = -1; m_y = -1; }

	bot::~bot() {}

	std::tuple<int, int> bot::get_coordinates(void)
	{
		// best move evaluation

		if (m_x == -1)
		{
			m_x = 0;
			return std::make_tuple<int, int>(2, 7);
		}
		else
			return std::make_tuple<int, int>(1, 8);
	}
}