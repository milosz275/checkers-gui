#include "include/bot.h"

namespace checkers
{
	bot::bot(char sign, game* game) : base_player(sign, "Bot") { m_game = game; m_considered_moves_ahead = 3; m_x = -1; m_y = -1; m_counter = 0; }

	bot::bot(const bot& bot) : base_player(bot) { m_game = bot.m_game; m_considered_moves_ahead = bot.m_considered_moves_ahead; m_x = bot.m_x; m_y = bot.m_y; m_counter = bot.m_counter; }

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
			std::cout << "copying the game..." << std::endl;
			game game_copy(*m_game);

			std::cout << "game copy:" << std::endl;
			std::cout << game_copy.get_board() << std::endl;

		}
		else // bot selected the piece and makes planned move
		{
			// for debug purposes
			system("PAUSE");
			// reset the counter
			m_counter = 0;
#ifdef _DEBUG
			m_game->m_os << "Bot is making planned move" << std::endl;
#endif
			return std::make_tuple(m_x, m_y); 
		}


		return std::make_tuple(1, 8);
	}

	int bot::minimax(game* game, int depth, int alpha, int beta, int is_maximizing)
	{

		return 1;
	}
}