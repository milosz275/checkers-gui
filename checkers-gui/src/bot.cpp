#include "include/bot.h"

namespace checkers
{
	bot::bot(char sign, const game* game) : base_player(sign, "Bot") { m_game = game; m_depth = 1; m_counter_select = 0; m_counter_move = 0; m_saved_move = std::pair<int, int>(std::make_pair(-1, -1)); }

	bot::bot(const bot& bot) : base_player(bot) { m_game = bot.m_game; m_depth = bot.m_depth; m_saved_move = bot.m_saved_move; m_counter_select = bot.m_counter_select; m_counter_move = bot.m_counter_move; }

	bot::~bot() {}

	std::pair<int, int> bot::get_coordinates(void)
	{
		assert(m_game->m_current_player == this);
		assert(m_game->m_game_freeze);

		if (!(m_game->m_selected_piece)) // bot has not selected any piece yet
		{
			// select loop detection
			++m_counter_select;
			m_counter_move = 0;
			if (m_counter_select > 1)
			{
				std::cout << "Bot is stuck in a select loop" << std::endl;
				system("pause");
				throw std::runtime_error("Bot is stuck in a select loop");
			}

			// copy the game
			game game_copy(*m_game);
			assert(!game_copy.m_player_1->get_list()->empty());
			assert(!game_copy.m_player_2->get_list()->empty());

			// check if there is at least one move available
			bool at_least_one_move_available = false;
			int piece_index = 0;
			all_of(game_copy.m_current_player->get_list()->begin(), game_copy.m_current_player->get_list()->end(), [&at_least_one_move_available, &piece_index](piece* p)
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

			// delegate finding coords through game copy to another method
			std::pair<int, int> best_move = find_best_move(&game_copy);
			
			std::cout << "bot: best found move from: x: " << std::get<0>(best_move) << "; y: " << std::get<1>(best_move) << std::endl;
			std::cout << "saved to move to: x: " << std::get<0>(m_saved_move) << "; y: " << std::get<1>(m_saved_move) << std::endl;
			return best_move;
		}
		else // bot selected the piece and makes planned move
		{
			// move loop detection
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
			return m_saved_move;
			}
	}

	std::pair<int, int> bot::find_best_move(game* game_copy)
	{
		//return std::make_tuple(1, 1);
		// in this method, all the data is assumed to be correct
		bool is_first = this->is_first();
		bool at_least_one_capture_available = game_copy->m_available_capture;
		std::list<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> list_of_games;

		// make sure evaluation was conducted properly
		if (at_least_one_capture_available)
		{
			int* highest_capture = nullptr;
			for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [&highest_capture](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&highest_capture](available_move* a)
						{
							assert(dynamic_cast<available_capture*>(a));
							if (highest_capture)
								assert(dynamic_cast<available_capture*>(a)->get_max_score() == *highest_capture);
							else
								highest_capture = new int(dynamic_cast<available_capture*>(a)->get_max_score());
								
						});
				});
		}
		else
		{
			for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [](available_move* a)
						{
							assert(dynamic_cast<available_move*>(a));
						});
				});
		}

		// fill in the game list
		if (at_least_one_capture_available)
		{
			// recursively go through multicaptures and add the game copy at the end
			add_to_game_copy_list(list_of_games, game_copy, nullptr, nullptr);
		}
		else
		{
			for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [&game_copy, &list_of_games](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&game_copy, &list_of_games, &p](available_move* a)
						{
							game* local_copy = new game(*game_copy);
							std::vector<std::vector<piece*>>* board = local_copy->get_board();
							int old_x = p->get_x();
							int old_y = p->get_y();
							piece* moving_piece = (*board)[old_x][old_y];
							int new_x = a->get_x();
							int new_y = a->get_y();
							local_copy->move_piece(moving_piece, board, new_x, new_y);
							list_of_games.push_back(std::make_tuple(local_copy, std::make_pair(old_x, old_y), std::make_pair(new_x, new_y)));
						});
				});
		}

		// now, list of games includes all game states after one round
		std::pair<int, int> best_move;
		int best_score = std::numeric_limits<int>::min();
		for_each(list_of_games.begin(), list_of_games.end(), [this, i = 0, &best_score, &best_move](std::tuple<game*, std::pair<int, int>, std::pair<int, int>>& game_and_coords) mutable
			{
				// 
				game* g = std::get<0>(game_and_coords);
				
				int local_score = g->get_score();
				if (local_score > best_score)
				{
					best_score = local_score;
					best_move = std::get<1>(game_and_coords);
					m_saved_move = std::get<2>(game_and_coords);
				}
			});
		return best_move;
	}
	
	void bot::add_to_game_copy_list(std::list<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>>& list_of_games, game* game_copy, std::pair<int, int>* source_coords, std::pair<int, int>* destination_coords)
	{
		// make checks
		bool at_least_one_capture_available = game_copy->m_available_capture;
		if (at_least_one_capture_available)
		{
			for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [](available_move* a)
						{
							assert(dynamic_cast<available_capture*>(a));
						});
				});
		}

		// make game copies	
		if (at_least_one_capture_available)
		{
			for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [&source_coords, &destination_coords, &game_copy, &list_of_games, this](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&source_coords, &destination_coords, &game_copy, &p, &list_of_games, this](available_move* a)
						{
							game* local_copy = new game(*game_copy);
							available_capture* b = dynamic_cast<available_capture*>(a);
							std::vector<std::vector<piece*>>* board = local_copy->get_board();
							int old_x = p->get_x();
							int old_y = p->get_y();
							piece* moving_piece = (*board)[old_x][old_y];
							local_copy->m_moving_piece = moving_piece;
							int new_x = b->get_x();
							int new_y = b->get_y();

							int deleted_x = b->get_x_d();
							int deleted_y = b->get_y_d();
							//std::cout << board << std::endl;
							// save capture direction: 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left
							if (deleted_x > new_x && deleted_y < new_y)
								local_copy->m_last_capture_direction = 3;
							else if (deleted_x < new_x && deleted_y < new_y)
								local_copy->m_last_capture_direction = 2;
							else if (deleted_x > new_x && deleted_y > new_y)
								local_copy->m_last_capture_direction = 1;
							else if (deleted_x < new_x && deleted_y > new_y)
								local_copy->m_last_capture_direction = 0;
							else
								throw std::runtime_error("Capturing in wrong direction");
							
							piece* to_delete_piece = (*board)[b->get_x_d()][b->get_y_d()];
						
							std::list<piece*>* to_delete_list = local_copy->get_to_delete_list();
							local_copy->make_capture(board, moving_piece, to_delete_piece, new_x, new_y, to_delete_list);
							int dummy = 0;
							local_copy->m_available_capture = local_copy->evaluate(local_copy->m_current_player->get_list(), local_copy->m_board, &dummy, dummy, local_copy->m_current_player, local_copy->m_last_capture_direction, &local_copy->m_to_delete_list, local_copy->m_moving_piece);

							bool changed_argument = false;
							if (!source_coords)
							{
								changed_argument = true;
								source_coords = new std::pair<int, int>(std::make_pair(old_x, old_y));
								destination_coords = new std::pair<int, int>(std::make_pair(new_x, new_y));
							}

							// recursively go through all captures
							add_to_game_copy_list(list_of_games, local_copy, source_coords, destination_coords);

							if (changed_argument)
							{
								source_coords = nullptr;
								destination_coords = nullptr;
							}
						});
				});

		}
		else
		{
			game_copy->m_moving_piece = nullptr;
			list_of_games.push_back(std::make_tuple(game_copy, *source_coords, *destination_coords));
		}
	}

	//void bot::add_to_game_copy_list(std::list<game*>& list_of_games, game* game_copy)
	//{
	//	// make checks
	//	bool at_least_one_capture_available = game_copy->m_available_capture;
	//	if (at_least_one_capture_available)
	//	{
	//		for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [](piece* p)
	//			{
	//				for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [](available_move* a)
	//					{
	//						assert(dynamic_cast<available_capture*>(a));
	//					});
	//			});
	//	}
	//	else
	//	{
	//		for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [](piece* p)
	//			{
	//				for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [](available_move* a)
	//					{
	//						assert(dynamic_cast<available_move*>(a));
	//					});
	//			});
	//	}
	//	
	//	// make game copies	
	//	if (at_least_one_capture_available)
	//	{
	//		for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [&game_copy, &list_of_games, this](piece* p)
	//			{
	//				for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&game_copy, &p, &list_of_games, this](available_move* a)
	//					{
	//						game* local_copy = new game(*game_copy);
	//						available_capture* b = dynamic_cast<available_capture*>(a);
	//						std::vector<std::vector<piece*>>* board = local_copy->get_board();
	//						piece* moving_piece = (*board)[p->get_x()][p->get_y()];
	//						piece* to_delete_piece = (*board)[b->get_x_d()][b->get_y_d()];
	//						int new_x = b->get_x();
	//						int new_y = b->get_y();
	//						std::list<piece*>* to_delete_list = local_copy->get_to_delete_list();
	//						local_copy->make_capture(board, moving_piece, to_delete_piece, new_x, new_y, to_delete_list);

	//						// recursively go through all captures
	//						add_to_game_copy_list(list_of_games, local_copy);
	//					});
	//			});

	//	}
	//	else 
	//	{
	//		for_each(game_copy->m_current_player->get_list()->begin(), game_copy->m_current_player->get_list()->end(), [&game_copy, &list_of_games](piece* p)
	//			{
	//				for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&game_copy, &list_of_games, &p](available_move* a)
	//					{
	//						game* local_copy = new game(*game_copy);
	//						std::vector<std::vector<piece*>>* board = local_copy->get_board();
	//						piece* moving_piece = (*board)[p->get_x()][p->get_y()];
	//						int new_x = a->get_x();
	//						int new_y = a->get_y();
	//						local_copy->move_piece(moving_piece, board, new_x, new_y);

	//						// end recursion adding last games in the search tree
	//						list_of_games.push_back(local_copy);
	//					});
	//			});
	//	}
	//}

	int minimax(game* game_copy, int depth, int alpha, int beta, int is_maximizing)
	{
		return game_copy->get_score();
	}

	/*
	std::tuple<int, int> bot::get_coordinates(void)
	{
		assert(m_game->m_current_player == this);
		assert(m_game->m_game_freeze);

		bool is_first = this->is_first();

		if (!(m_game->m_selected_piece)) // bot has not selected any piece yet
		{
			// loop detection
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
			assert(!local_game.m_player_1->get_list()->empty());
			assert(!local_game.m_player_2->get_list()->empty());

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

			// actual code
			std::tuple<int, int> best_move;
			int best_score = std::numeric_limits<int>::min();
			int alpha = std::numeric_limits<int>::min();
			int beta = std::numeric_limits<int>::max();

			// make all captures 
			bool at_least_one_capture_available = local_game.m_available_capture;
			while (at_least_one_capture_available)
			{
				// for every available move or capture of every piece of current player
				for_each(std::next(local_game.m_current_player->get_list()->begin(), piece_index), local_game.m_current_player->get_list()->end(), [this, &local_game, &best_move, &best_score, &alpha, &beta, at_least_one_capture_available, &is_first](piece* p)
					{
						for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &local_game, p, &best_move, &best_score, &alpha, &beta, &at_least_one_capture_available, &is_first](available_move* move)
							{
								// check if that when it is a capture, there is its corresponding game evaluated correctly (has available capture)
								bool is_capture = dynamic_cast<available_capture*>(move);
								assert(at_least_one_capture_available && is_capture || !is_capture);

								// copy the game for each move made
								game game_copy(local_game);
								std::vector<std::vector<piece*>>* board = game_copy.get_board();
								base_player* player = nullptr;
								base_player* opponent = nullptr;

								if (is_first)
								{
									player = game_copy.m_player_1;
									opponent = game_copy.m_player_2;
								}
								else
								{
									player = game_copy.m_player_2;
									opponent = game_copy.m_player_1;
								}
#ifdef _DEBUG
								// make sure pieces are owned correctly
								for_each(player->get_list()->begin(), player->get_list()->end(), [this, &player](piece* p) { assert(p->get_owner() == player); });
								for_each(opponent->get_list()->begin(), opponent->get_list()->end(), [this, &opponent](piece* p) { assert(p->get_owner() == opponent); });
#endif
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
								}

								// move the piece (move and capture)
								piece* moving_piece = (*board)[original_x][original_y];
								assert(moving_piece);
								game_copy.move_piece(moving_piece, board, x, y);
								(*board)[x][y] = moving_piece;

								// minimax calculation
								int score = minimax(&game_copy, m_depth, alpha, beta, false);

								// increment if first move was a capture
								if (is_capture)
									score++;

								// check
								if (score > best_score)
								{
									best_score = score;
									best_move = std::make_tuple(original_x, original_y);
									m_x = x;
									m_y = y;
									std::cout << "setting new highest score move: from: x" << original_x << "; y: " << original_y << "; to: x: " << x << "; y: " << y << std::endl;
								}
								else if (score == best_score)
								{
									std::cout << "equal as best score" << std::endl;
								}
							});
					});
			}

			// go through possible moves, for every available move or capture of every piece of current player
			for_each(std::next(local_game.m_current_player->get_list()->begin(), piece_index), local_game.m_current_player->get_list()->end(), [this, &local_game, &best_move, &best_score, &alpha, &beta, at_least_one_capture_available, &is_first](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &local_game, p, &best_move, &best_score, &alpha, &beta, &at_least_one_capture_available, &is_first](available_move* move)
						{
							// check if that when it is a capture, there is its corresponding game evaluated correctly (has available capture)
							bool is_capture = dynamic_cast<available_capture*>(move);
							assert(at_least_one_capture_available && is_capture || !is_capture);

							// copy the game for each move made
							game game_copy(local_game);
							std::vector<std::vector<piece*>>* board = game_copy.get_board();
							base_player* player = nullptr;
							base_player* opponent = nullptr;

							if (is_first)
							{
								player = game_copy.m_player_1;
								opponent = game_copy.m_player_2;
							}
							else
							{
								player = game_copy.m_player_2;
								opponent = game_copy.m_player_1;
							}
#ifdef _DEBUG
							// make sure pieces are owned correctly
							for_each(player->get_list()->begin(), player->get_list()->end(), [this, &player](piece* p) { assert(p->get_owner() == player); });
							for_each(opponent->get_list()->begin(), opponent->get_list()->end(), [this, &opponent](piece* p) { assert(p->get_owner() == opponent); });
#endif
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
							}

							// move the piece (move and capture)
							piece* moving_piece = (*board)[original_x][original_y];
							assert(moving_piece);
							game_copy.move_piece(moving_piece, board, x, y);
							(*board)[x][y] = moving_piece;

							// minimax calculation
							int score = minimax(&game_copy, m_depth, alpha, beta, false);

							// increment if first move was a capture
							if (is_capture)
								score++;

							// check
							if (score > best_score)
							{
								best_score = score;
								best_move = std::make_tuple(original_x, original_y);
								m_x = x;
								m_y = y;
								std::cout << "setting new highest score move: from: x" << original_x << "; y: " << original_y << "; to: x: " << x << "; y: " << y << std::endl;
							}
							else if (score == best_score)
							{
								std::cout << "equal as best score" << std::endl;
							}
						});
				});
			}

			// TODO: choose from the best moves list

			std::cout << "bot: best found move from: x: " << std::get<0>(best_move) << "; y: " << std::get<1>(best_move) << std::endl;
			std::cout << "saved to move to: x: " << m_x << "; y: " << m_y << std::endl;
			return best_move;
		}
		else // bot selected the piece and makes planned move
		{
			// detect the loop
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
		if (depth == 0 || input_game->get_completion())
			return input_game->get_score();
		else
		{
			minimax(input_game, depth - 1, alpha, beta, true);
		}
	}
	*/
//	std::tuple<int, int> bot::get_coordinates(void)
//	{
//		assert(m_game->m_current_player == this);
//		assert(m_game->m_game_freeze);
//
//		bool is_first = this->is_first();
//
//		if (!(m_game->m_selected_piece)) // bot has not selected any piece yet
//		{
//			// loop detection
//			++m_counter_select;
//			m_counter_move = 0;
//			if (m_counter_select > 1)
//			{
//				std::cout << "Bot is stuck in a select loop" << std::endl;
//				system("pause");
//				throw std::runtime_error("Bot is stuck in a select loop");
//			}
//
//			// before checking every combination, copy to allow moving the pieces on the copied board
//			game local_game(*m_game);
//			assert(!local_game.get_player_1()->get_list()->empty());
//			assert(!local_game.get_player_2()->get_list()->empty());
//
//			// check if there is at least one move available
//			bool at_least_one_move_available = false;
//			int piece_index = 0;
//			all_of(local_game.m_current_player->get_list()->begin(), local_game.m_current_player->get_list()->end(), [&at_least_one_move_available, &piece_index](piece* p)
//				{
//					if (!(p->get_av_list()->empty()))
//					{
//						at_least_one_move_available = true;
//						return false;
//					}
//					++piece_index;
//					return true;
//				});
//			assert(at_least_one_move_available);
//
//			// actual code
//			std::tuple<int, int> best_move;
//			int best_score = std::numeric_limits<int>::min();
//			int alpha = std::numeric_limits<int>::min();
//			int beta = std::numeric_limits<int>::max();
//
//			bool at_least_one_capture_available = m_game->m_available_capture;
//
//			// for every available move or capture of every piece of current player
//			for_each(std::next(local_game.m_current_player->get_list()->begin(), piece_index), local_game.m_current_player->get_list()->end(), [this, &local_game, &best_move, &best_score, &alpha, &beta, at_least_one_capture_available, &is_first](piece* p)
//				{
//					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &local_game, p, &best_move, &best_score, &alpha, &beta, &at_least_one_capture_available, &is_first](available_move* move)
//						{
//							// check if that when it is a capture, there is its corresponding game evaluated correctly (has available capture)
//							bool is_capture = dynamic_cast<available_capture*>(move);
//							assert(at_least_one_capture_available && is_capture || !is_capture);
//
//							// copy the game for each move made
//							game game_copy(local_game);
//							std::vector<std::vector<piece*>>* board = game_copy.get_board();
//							base_player* player = nullptr;
//							base_player* opponent = nullptr;
//
//							if (is_first)
//							{
//								player = game_copy.get_player_1();
//								opponent = game_copy.get_player_2();
//							}
//							else
//							{
//								player = game_copy.get_player_2();
//								opponent = game_copy.get_player_1();
//							}
//#ifdef _DEBUG
//							// make sure pieces are owned correctly
//							for_each(player->get_list()->begin(), player->get_list()->end(), [this, &player](piece* p) { assert(p->get_owner() == player); });
//							for_each(opponent->get_list()->begin(), opponent->get_list()->end(), [this, &opponent](piece* p) { assert(p->get_owner() == opponent); });
//#endif
//							// coords where from the piece comes from
//							int original_x = p->get_x();
//							int original_y = p->get_y();
//
//							// new coords for the piece
//							int x = move->get_x();
//							int y = move->get_y();
//
//							// make capture
//							if (is_capture) // this move is a capture (delete captured)
//							{
//								// mark found capture
//								available_capture* capture = dynamic_cast<available_capture*>(move);
//								int x_d = capture->get_x_d();
//								int y_d = capture->get_y_d();
//								piece* piece_to_delete = (*board)[x_d][y_d];
//								assert(piece_to_delete);
//
//								game_copy.delete_piece(piece_to_delete, board, opponent);
//								opponent->add_capture();
//							}
//
//							// move the piece (move and capture)
//							piece* moving_piece = (*board)[original_x][original_y];
//							assert(moving_piece);
//							game_copy.move_piece(moving_piece, board, x, y);
//							(*board)[x][y] = moving_piece;
//
//							// minimax calculation
//							int score = minimax(&game_copy, m_depth, alpha, beta, false);
//
//							// increment if first move was a capture
//							if (is_capture)
//								score++;
//
//							// check
//							if (score > best_score)
//							{
//								best_score = score;
//								best_move = std::make_tuple(original_x, original_y);
//								m_x = x;
//								m_y = y;
//								std::cout << "setting new highest score move: from: x" << original_x << "; y: " << original_y << "; to: x: " << x << "; y: " << y << std::endl;
//							}
//							else if (score == best_score)
//							{
//								std::cout << "equal as best score" << std::endl;
//							}
//						});
//				});
//
//			// TODO: choose from the best moves list
//
//			std::cout << "bot: best found move from: x: " << std::get<0>(best_move) << "; y: " << std::get<1>(best_move) << std::endl;
//			std::cout << "saved to move to: x: " << m_x << "; y: " << m_y << std::endl;
//			return best_move;
//		}
//		else // bot selected the piece and makes planned move
//		{
//			// detect the loop
//			++m_counter_move;
//			m_counter_select = 0;
//			if (m_counter_move > 1)
//			{
//				std::cout << "Bot is stuck in a move loop" << std::endl;
//				system("pause");
//				throw std::runtime_error("Bot is stuck in a move loop");
//			}
//#ifdef _DEBUG
//			m_game->m_os << "Bot is making planned move" << std::endl;
//#endif
//			return std::make_tuple(m_x, m_y);
//		}
//	}
//
//	int bot::minimax(game* input_game, int depth, int alpha, int beta, int is_maximizing)
//	{
//		if (depth == 0 || input_game->get_completion())
//			return input_game->get_score();
//		else
//		{
//			// copy the game once again and make move not considering who the current player is
//			bool at_least_one_capture_available = input_game->m_available_capture;
//			bool is_first = true;
//			if (input_game->m_current_player == input_game->m_player_2)
//				is_first = false;
//
//			if (is_maximizing)
//			{
//				int max_score = std::numeric_limits<int>::min();
//				// for every available move or capture of every piece of current player
//				for_each(input_game->m_current_player->get_list()->begin(), input_game->m_current_player->get_list()->end(), [this, &input_game, &max_score, &alpha, &beta, at_least_one_capture_available, &is_first](piece* p)
//					{
//						for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &input_game, p, &max_score, &alpha, &beta, &at_least_one_capture_available, &is_first](available_move* move)
//							{
//								// check if that when it is a capture, there is its corresponding game evaluated correctly (has available capture)
//								bool is_capture = dynamic_cast<available_capture*>(move);
//								assert(at_least_one_capture_available && is_capture || !is_capture);
//
//								// copy the game for each move made
//								game game_copy(*input_game);
//								std::vector<std::vector<piece*>>* board = game_copy.get_board();
//								base_player* player = nullptr;
//								base_player* opponent = nullptr;
//
//								if (is_first)
//								{
//									player = game_copy.get_player_1();
//									opponent = game_copy.get_player_2();
//								}
//								else
//								{
//									player = game_copy.get_player_2();
//									opponent = game_copy.get_player_1();
//								}
//#ifdef _DEBUG
//								// make sure pieces are owned correctly
//								for_each(player->get_list()->begin(), player->get_list()->end(), [this, &player](piece* p) { assert(p->get_owner() == player); });
//								for_each(opponent->get_list()->begin(), opponent->get_list()->end(), [this, &opponent](piece* p) { assert(p->get_owner() == opponent); });
//#endif
//								// coords where from the piece comes from
//								int original_x = p->get_x();
//								int original_y = p->get_y();
//
//								// new coords for the piece
//								int x = move->get_x();
//								int y = move->get_y();
//
//								// make capture
//								if (is_capture) // this move is a capture (delete captured)
//								{
//									// mark found capture
//									available_capture* capture = dynamic_cast<available_capture*>(move);
//									int x_d = capture->get_x_d();
//									int y_d = capture->get_y_d();
//									piece* piece_to_delete = (*board)[x_d][y_d];
//									assert(piece_to_delete);
//
//									game_copy.delete_piece(piece_to_delete, board, opponent);
//									opponent->add_capture();
//								}
//
//								// move the piece (move and capture)
//								piece* moving_piece = (*board)[original_x][original_y];
//								assert(moving_piece);
//								game_copy.move_piece(moving_piece, board, x, y);
//								(*board)[x][y] = moving_piece;
//
//								// minimax calculation
//								int score = minimax(&game_copy, m_depth, alpha, beta, false);
//
//								// increment if first move was a capture
//								if (is_capture)
//									score++;
//
//								// check
//								if (score > best_score)
//								{
//									best_score = score;
//									best_move = std::make_tuple(original_x, original_y);
//									m_x = x;
//									m_y = y;
//									std::cout << "setting new highest score move: from: x" << original_x << "; y: " << original_y << "; to: x: " << x << "; y: " << y << std::endl;
//								}
//								else if (score == best_score)
//								{
//									std::cout << "equal as best score" << std::endl;
//								}
//							});
//					});
//
//
//
//
//
//				for (const auto& move : get_possible_moves(board, bot_pieces)) {
//					make_move(board, move);
//					int eval = minimax_alpha_beta(board, depth - 1, alpha, beta, false);
//					max_eval = std::max(max_eval, eval);
//					alpha = std::max(alpha, eval);
//					undo_move(board, move);
//					if (beta <= alpha) {
//						break;
//					}
//			}
//			else // not maximizing
//			{
//
//			}
//		}
//
//
//	}
	// oldest take
//	std::tuple<int, int> bot::get_coordinates(void)
//	{
//		assert(m_game->m_current_player == this);
//		assert(m_game->m_game_freeze);
//
//		bool is_first = this->is_first();
//
//		if (!(m_game->m_selected_piece)) // bot has not selected any piece yet
//		{
//			// loop detection
//			++m_counter_select;
//			m_counter_move = 0;
//			if (m_counter_select > 1)
//			{	
//				std::cout << "Bot is stuck in a select loop" << std::endl;
//				system("pause");
//				throw std::runtime_error("Bot is stuck in a select loop");
//			}
//
//			// before checking every combination, copy to allow moving the pieces on the copied board
//			game local_game(*m_game);
//			assert(!local_game.get_player_1()->get_list()->empty());
//			assert(!local_game.get_player_2()->get_list()->empty());
//
//			// check if there is at least one move available
//			bool at_least_one_move_available = false;
//			int piece_index = 0;
//			all_of(local_game.m_current_player->get_list()->begin(), local_game.m_current_player->get_list()->end(), [&at_least_one_move_available, &piece_index](piece* p)
//				{
//					if (!(p->get_av_list()->empty()))
//					{
//						at_least_one_move_available = true;
//						return false;
//					}
//					++piece_index;
//					return true;
//				});
//			assert(at_least_one_move_available);
//
//			// actual code
//			std::tuple<int, int> best_move;
//			int best_score = std::numeric_limits<int>::min();
//			int alpha = std::numeric_limits<int>::min();
//			int beta = std::numeric_limits<int>::max();
//
//			bool at_least_one_capture_available = m_game->m_available_capture;
//
//			// for every available move or capture of every piece of current player
//			for_each(std::next(local_game.m_current_player->get_list()->begin(), piece_index), local_game.m_current_player->get_list()->end(), [this, &local_game, &best_move, &best_score, &alpha, &beta, at_least_one_capture_available, &is_first](piece* p)
//				{
//					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &local_game, p, &best_move, &best_score, &alpha, &beta, &at_least_one_capture_available, &is_first](available_move* move)
//						{
//							bool is_capture = dynamic_cast<available_capture*>(move);
//							assert(at_least_one_capture_available && is_capture || !is_capture);
//
//							// copy the game for each move made
//							game game_copy(local_game);
//							std::vector<std::vector<piece*>>* board = game_copy.get_board();
//							base_player* player = nullptr;
//							base_player* opponent = nullptr;
//
//							if (is_first)
//							{
//								player = game_copy.get_player_1();
//								opponent = game_copy.get_player_2();
//							}
//							else
//							{
//								player = game_copy.get_player_2();
//								opponent = game_copy.get_player_1();
//							}
//#ifdef _DEBUG
//							// make sure pieces are owned correctly
//							for_each(player->get_list()->begin(), player->get_list()->end(), [this, &player](piece* p) { assert(p->get_owner() == player); });
//							for_each(opponent->get_list()->begin(), opponent->get_list()->end(), [this, &opponent](piece* p) { assert(p->get_owner() == opponent); });
//#endif
//							// coords where from the piece comes from
//							int original_x = p->get_x();
//							int original_y = p->get_y();
//
//							// new coords for the piece
//							int x = move->get_x();
//							int y = move->get_y();
//
//							// make capture
//							if (is_capture) // this move is a capture (delete captured)
//							{
//								// mark found capture
//								available_capture* capture = dynamic_cast<available_capture*>(move);
//								int x_d = capture->get_x_d();
//								int y_d = capture->get_y_d();
//								piece* piece_to_delete = (*board)[x_d][y_d];
//								assert(piece_to_delete);
//								
//								game_copy.delete_piece(piece_to_delete, board, opponent);
//								opponent->add_capture();
//							}
//
//							// move the piece (move and capture)
//							piece* moving_piece = (*board)[original_x][original_y];
//							assert(moving_piece);
//							game_copy.move_piece(moving_piece, board, x, y);
//							(*board)[x][y] = moving_piece;
//
//							// minimax calculation
//							int score = minimax(&game_copy, m_depth, alpha, beta, false);
//
//							// increment if first move was a capture
//							if (is_capture)
//								score++;
//
//							// check
//							if (score > best_score)
//							{
//								best_score = score;
//								best_move = std::make_tuple(original_x, original_y);
//								m_x = x;
//								m_y = y;
//								std::cout << "setting new highest score move: from: x" << original_x << "; y: " << original_y << "; to: x: " << x << "; y: " << y << std::endl;
//							}
//							else if (score == best_score)
//							{
//								std::cout << "equal as best score" << std::endl;
//							}
//						});
//				});
//
//			// TODO: choose from the best moves list
//
//			std::cout << "bot: best found move from: x: " << std::get<0>(best_move) << "; y: " << std::get<1>(best_move) << std::endl;
//			std::cout << "saved to move to: x: " << m_x << "; y: " << m_y << std::endl;
//			return best_move;
//		}
//		else // bot selected the piece and makes planned move
//		{
//			//system("pause");
//
//			// for debug purposes
//			//system("PAUSE");
//			// reset the counter
//			++m_counter_move;
//			m_counter_select = 0;
//			if (m_counter_move > 1)
//			{
//				std::cout << "Bot is stuck in a move loop" << std::endl;
//				system("pause");
//				throw std::runtime_error("Bot is stuck in a move loop");
//			}
//#ifdef _DEBUG
//			m_game->m_os << "Bot is making planned move" << std::endl;
//#endif
//			return std::make_tuple(m_x, m_y); 
//		}
//	}
//
//	int bot::minimax(game* input_game, int depth, int alpha, int beta, int is_maximizing)
//	{
//		if (depth == 0)
//			return input_game->get_score();
//		else
//		{
//			//
//			game game_copy(*input_game);
//
//			//minimax(game_copy, depth - 1, )
//		}
//
//		
//	}
}