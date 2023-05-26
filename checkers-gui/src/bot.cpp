#include "include/bot.h"

namespace checkers
{
	bot::bot(char sign, const game* game) : base_player(sign, "Bot") { m_game = game; m_depth = 1; m_x = -1; m_y = -1; m_counter_select = 0; m_counter_move = 0;  }

	bot::bot(const bot& bot) : base_player(bot) { m_game = bot.m_game; m_depth = bot.m_depth; m_x = bot.m_x; m_y = bot.m_y; m_counter_select = bot.m_counter_select; m_counter_move = bot.m_counter_move; }

	bot::~bot() {}

	std::tuple<int, int> bot::get_coordinates(void)
	{
		assert(m_game->m_current_player == this);

		if (m_game->m_selected_piece == nullptr) // bot has not selected any piece yet
		{
			// best move evaluation
			// for each piece and its move copy the board and lists and evaluate saving iterations and score after
			// find one piece that corresponds to the best move and save capture coords in m_x, m_y
			
			++m_counter_select;
			m_counter_move = 0;
			if (m_counter_select > 1)
				throw std::runtime_error("Bot is stuck in a select loop");

			// before checking every combination, copy to allow moving the pieces on the copied board
			game local_game(*m_game);

			std::cout << "board after copying:" << std::endl;
			std::cout << local_game.get_board() << std::endl;

			int best_score = std::numeric_limits<int>::min();
			std::tuple<int, int> best_move;

			// check if there is at least one move available
			bool at_least_one_move_available = false;
			int piece_index = 0;
			all_of(local_game.m_current_player->get_list()->begin(), local_game.m_current_player->get_list()->end(), [&at_least_one_move_available, &piece_index](piece* p)
				{
					if (!(p->get_av_list()->empty()))
					{
						at_least_one_move_available = true;
						return false;
					}
					++piece_index;
					return true;
				});
			assert(at_least_one_move_available);

			// for every available move or capture of every piece of current player
			for_each(std::next(local_game.m_current_player->get_list()->begin(), piece_index), local_game.m_current_player->get_list()->end(), [this, &local_game, &best_move, &best_score](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &local_game, p, &best_move, &best_score](available_move* move)
						{
							game game_copy(local_game);
							std::vector<std::vector<piece*>>* board = game_copy.get_board();

							if (dynamic_cast<available_capture*>(move)) // this move is a capture (delete captured)
							{
								// mark found capture
								available_capture* capture = dynamic_cast<available_capture*>(move);
								int x_d = capture->get_x_d();
								int y_d = capture->get_y_d();
								(*board)[x_d][y_d] = nullptr;
							}

							// move the piece (move and capture)
							int x = move->get_x();
							int y = move->get_y();
							
							int original_x = p->get_x();
							int original_y = p->get_y();

							(*board)[p->get_x()][p->get_y()] = nullptr;
							p->set_x(x);
							p->set_y(y);
							(*board)[x][y] = p;

							// minimax calculation
							int score = minimax(&game_copy, m_depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);

							// check
							if (score > best_score)
							{
								best_score = score;
								best_move = std::make_tuple(original_x, original_y);
								m_x = x;
								m_y = y;
							}
						});
				});
			std::cout << "bot: best found move from: x: " << std::get<0>(best_move) << "; y: " << std::get<1>(best_move) << std::endl;
			std::cout << "saved to move to: x: " << m_x << "; y: " << m_y << std::endl;
			return best_move;
		}
		else // bot selected the piece and makes planned move
		{
			// for debug purposes
			//system("PAUSE");
			// reset the counter
			++m_counter_move;
			m_counter_select = 0;
			if (m_counter_move > 1)
				throw std::runtime_error("Bot is stuck in a move loop");
#ifdef _DEBUG
			m_game->m_os << "Bot is making planned move" << std::endl;
#endif
			return std::make_tuple(m_x, m_y); 
		}
	}

	int bot::minimax(game* game, int depth, int alpha, int beta, int is_maximizing)
	{

		return 1;
	}
}