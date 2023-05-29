#include "include/bot.h"

namespace checkers
{
	bot::bot(char sign, const game* game) : base_player(sign, "Bot") { m_game = game; m_depth = 1; m_x = -1; m_y = -1; m_counter_select = 0; m_counter_move = 0;  }

	bot::bot(const bot& bot) : base_player(bot) { m_game = bot.m_game; m_depth = bot.m_depth; m_x = bot.m_x; m_y = bot.m_y; m_counter_select = bot.m_counter_select; m_counter_move = bot.m_counter_move; }

	bot::~bot() {}

	std::tuple<int, int> bot::get_coordinates(void)
	{
		assert(m_game->m_current_player == this);
		assert(m_game->m_game_freeze);

		bool is_first = this->is_first();

		if (!(m_game->m_selected_piece)) // bot has not selected any piece yet
		{
			//system("pause");


			// best move evaluation
			// for each piece and its move copy the board and lists and evaluate saving iterations and score after
			// find one piece that corresponds to the best move and save capture coords in m_x, m_y
			
			++m_counter_select;
			m_counter_move = 0;
			if (m_counter_select > 1)
			{	
				std::cout << "Bot is stuck in a select loop" << std::endl;
				system("pause");
				throw std::runtime_error("Bot is stuck in a select loop");
			}

			// before checking every combination, copy to allow moving the pieces on the copied board
			game local_game(*m_game);
			assert(!local_game.get_player_1()->get_list()->empty());
			assert(!local_game.get_player_2()->get_list()->empty());

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

			bool at_least_one_capture_available = m_game->m_available_capture;

			// for every available move or capture of every piece of current player
			for_each(std::next(local_game.m_current_player->get_list()->begin(), piece_index), local_game.m_current_player->get_list()->end(), [this, &local_game, &best_move, &best_score, at_least_one_capture_available, &is_first](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &local_game, p, &best_move, &best_score, &at_least_one_capture_available, &is_first](available_move* move)
						{
							bool is_capture = dynamic_cast<available_capture*>(move);
							assert(at_least_one_capture_available && is_capture || !is_capture);

							game game_copy(local_game);

							std::vector<std::vector<piece*>>* board = game_copy.get_board();

							base_player* player = nullptr;
							base_player* opponent = nullptr;

							if (is_first)
							{
								player = game_copy.get_player_1();
								opponent = game_copy.get_player_2();
							}
							else
							{
								player = game_copy.get_player_2();
								opponent = game_copy.get_player_1();
							}

							for_each(player->get_list()->begin(), player->get_list()->end(), [this, &player](piece* p) { assert(p->get_owner() == player); });
							for_each(opponent->get_list()->begin(), opponent->get_list()->end(), [this, &opponent](piece* p) { assert(p->get_owner() == opponent); });

							// coords where from the piece comes from
							int original_x = p->get_x();
							int original_y = p->get_y();

							// new coords for the piece
							int x = move->get_x();
							int y = move->get_y();

							// make capture
							if (is_capture) // this move is a capture (delete captured)
							{
								// mark found capture
								available_capture* capture = dynamic_cast<available_capture*>(move);
								int x_d = capture->get_x_d();
								int y_d = capture->get_y_d();
								piece* piece_to_delete = (*board)[x_d][y_d];
								assert(piece_to_delete);
								
								game_copy.delete_piece(piece_to_delete, board, opponent);
								opponent->add_capture();

								//(*board)[x_d][y_d] = nullptr;
							}
							//else // make normal move

							// move the piece (move and capture)
							piece* moving_piece = (*board)[original_x][original_y];
							assert(moving_piece);
							game_copy.move_piece(moving_piece, board, x, y);
							(*board)[x][y] = moving_piece;
							
							/*(*board)[p->get_x()][p->get_y()] = nullptr;
							p->set_x(x);
							p->set_y(y);
							(*board)[x][y] = p;*/

							// minimax calculation
							int score = minimax(&game_copy, m_depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);

							if (is_capture)
								score++;

							// check
							if (score > best_score)
							{
								best_score = score;
								best_move = std::make_tuple(original_x, original_y);
								m_x = x;
								m_y = y;
								std::cout << "setting new highest" << std::endl;
							}
							else if (score == best_score)
							{
								std::cout << "equal as best score" << std::endl;
							}
						});
				});
			std::cout << "bot: best found move from: x: " << std::get<0>(best_move) << "; y: " << std::get<1>(best_move) << std::endl;
			std::cout << "saved to move to: x: " << m_x << "; y: " << m_y << std::endl;
			return best_move;
		}
		else // bot selected the piece and makes planned move
		{
			//system("pause");

			// for debug purposes
			//system("PAUSE");
			// reset the counter
			++m_counter_move;
			m_counter_select = 0;
			if (m_counter_move > 1)
			{
				std::cout << "Bot is stuck in a move loop" << std::endl;
				system("pause");
				throw std::runtime_error("Bot is stuck in a move loop");
			}
#ifdef _DEBUG
			m_game->m_os << "Bot is making planned move" << std::endl;
#endif
			return std::make_tuple(m_x, m_y); 
		}
	}

	int bot::minimax(game* input_game, int depth, int alpha, int beta, int is_maximizing)
	{
		if (depth == 0)
			return input_game->get_score();
		else
		{
			//
			game game_copy(*input_game);

			//minimax(game_copy, depth - 1, )
		}

		
	}
}