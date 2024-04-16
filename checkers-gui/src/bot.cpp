#include "bot.h"

namespace checkers
{
	bot::bot(char sign, int depth, std::ostream& os)
		: base_player(sign, "Bot"), m_os(os) { m_game = nullptr; assert(depth >= 1); m_depth = depth; m_counter_select = 0; m_counter_move = 0; m_saved_move = std::pair<int, int>(std::make_pair(-1, -1)); }

	bot::~bot() {}

	game* bot::get_game(void) { return m_game; }

	game* bot::set_game(game* game) { return m_game = game; }

	std::pair<int, int> bot::get_coordinates(void)
	{
		assert(m_game);
		assert(m_game->get_game_state()->get_current_player()->is_first() == is_first());
		assert(m_game->get_game_state()->get_game_freeze());
#ifdef _DEBUG
		m_os << "Getting coords from bot" << std::endl;
#endif
		if (!(m_game->get_selected_piece())) // bot has not selected any piece yet
		{
#ifdef _DEBUG
			m_os << "Bot selects" << std::endl;
#endif
			// select loop detection
			++m_counter_select;
			m_counter_move = 0;
			if (m_counter_select > 1)
			{
#ifdef _DEBUG
				m_os << "Bot is stuck in a select loop" << std::endl;
				system("pause");
#endif
				throw std::runtime_error("Bot is stuck in a select loop");
			}
			
			// copy the game
			game game_copy(*m_game);
			assert(!game_copy.get_player_1()->get_list()->empty());
			assert(!game_copy.get_player_2()->get_list()->empty());
			
			// check if there is at least one move available
			bool at_least_one_move_available = false;
			int piece_index = 0;
			for (piece* p : *game_copy.get_game_state()->get_current_player()->get_list())
			{
				if (!(p->get_av_list()->empty()))
				{
					at_least_one_move_available = true;
					break;
				}
				++piece_index;
			}
			assert(at_least_one_move_available);

			std::pair<std::pair<int, int>, std::pair<int, int>> best_move = find_best_move(&game_copy, m_depth, s_min_score, s_max_score, true);
			std::pair<int, int> selected_coords = std::get<0>(best_move);
			std::pair<int, int> saved_coords = std::get<1>(best_move);
			m_saved_move = saved_coords;
			return selected_coords;
		}
		else // bot selected the piece and makes planned move
		{
#ifdef _DEBUG
			m_os << "Bot moves" << std::endl;
#endif
			// move loop detection
			++m_counter_move;
			m_counter_select = 0;
			if (m_counter_move > 1)
			{
#ifdef _DEBUG
				m_os << "Bot is stuck in a move loop" << std::endl;
				system("pause");
#endif
				throw std::runtime_error("Bot is stuck in a move loop");
			}
#ifdef _DEBUG
			m_os << "Bot is making planned move: x: " << std::get<0>(m_saved_move) << "; y: " << std::get<1>(m_saved_move) << std::endl;
#endif
			bool there_is_that_move = false;
			for_each(get_list()->begin(), get_list()->end(), [this, &there_is_that_move](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &there_is_that_move](available_move* a)
						{
							if (a->get_x() == std::get<0>(m_saved_move) && a->get_y() == std::get<1>(m_saved_move))
							{
								there_is_that_move = true;
							}
						});
				});
			assert(there_is_that_move);

			// reset saved move and return the chosen one
			std::pair<int, int> move = m_saved_move;
			m_saved_move = std::make_pair(-1, -1);
			return move;
		}
	}

	std::pair<std::pair<int, int>, std::pair<int, int>> bot::find_best_move(game* game_copy, int depth, int min_score, int max_score, bool is_maximizing)
	{
		std::vector<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> best_moves = find_best_games(game_copy, depth, min_score, max_score, is_maximizing);

		if (!best_moves.empty())
		{
			// select randomly
			std::tuple<game*, std::pair<int, int>, std::pair<int, int>> chosen_move = best_moves[rand() % best_moves.size()];
			
			std::pair<int, int> selected_coords = std::get<1>(chosen_move);
			std::pair<int, int> saved_coords = std::get<2>(chosen_move);
#ifdef _DEBUG
			m_os << "Bot: Find best move: randomly choosing from \'" << best_moves.size() << "\' games of equal score" << std::endl;
			m_os << "-Best found move from: x: " << selected_coords.first << "; y: " << selected_coords.second << std::endl;
			m_os << "-Saved to move to: x: " << saved_coords.first << "; y: " << saved_coords.second << std::endl;
#endif
			return std::make_pair(selected_coords, saved_coords);
		}
		else
		{
#ifdef _DEBUG
			m_os << "Bot: Find best move: Returned list of games was empty. Choosing -1" << std::endl;
#endif
			return std::make_pair(std::make_pair(-1, -1), std::make_pair(-1, -1));
		}
	}

	std::vector<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> bot::find_best_games(game* game_copy, int depth, int alpha, int beta, bool maximizing_player)
	{
#ifdef _DEBUG
		m_os << "Current depth: " << depth << std::endl;
#endif
		if (depth == 0 || game_copy->get_game_state()->check_completion())
		{
#ifdef _DEBUG
			m_os << "Bot: Find best games: algorithm hit boundary condition: depth or complete game" << std::endl;
#endif
			std::vector<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> result;
			result.emplace_back(game_copy, std::make_pair(-1, -1), std::make_pair(-1, -1));
			return result;
		}
		else if (depth < m_depth)
		{
#ifdef _DEBUG
			m_os << "Bot: Find best games: switching turn" << std::endl;
#endif
			game_copy->get_game_state()->switch_turn();
			// TODO: move to game state
			game_copy->set_selected(false);
			game_copy->set_selected_piece(nullptr);
			game_copy->set_moving_piece(nullptr);

			game_copy->clear_list(&game_copy->get_list_1());
			game_copy->clear_list(&game_copy->get_list_2());
			int dummy = 0;
			game_copy->set_available_capture(game_copy->evaluate(game_copy->get_game_state()->get_current_player()->get_list(), game_copy->get_board(), &dummy, dummy, game_copy->get_last_capture_direction(), game_copy->get_to_delete_list(), nullptr));
		}
		
		std::vector<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> best_moves;
		bool is_first = this->is_first();
		bool at_least_one_capture_available = game_copy->get_available_capture();
		std::list<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> list_of_games;
		
		// fill in the game list
		if (at_least_one_capture_available)
		{
			// recursively go through multicaptures and add the game copy at the end
			game_copy->get_game_state()->get_current_player()->set_combo(true);
			add_to_game_copy_list(list_of_games, game_copy, nullptr, nullptr);
		}
		else
		{
			game_copy->set_moving_piece(nullptr);
			for_each(game_copy->get_game_state()->get_current_player()->get_list()->begin(), game_copy->get_game_state()->get_current_player()->get_list()->end(), [&game_copy, &list_of_games](piece* p)
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
#ifdef _DEBUG
		// print scores after each simulation
		for_each(list_of_games.begin(), list_of_games.end(), [i = 0, this](std::tuple<game*, std::pair<int, int>, std::pair<int, int>>& game_and_coords) mutable
			{
				m_os << "simulation score (no recursion): " << ++i << ": " << std::get<0>(game_and_coords)->get_score() << std::endl;
			});
#endif
		// recursively go through all games again
		for_each(list_of_games.begin(), list_of_games.end(), [this, &depth, &maximizing_player](std::tuple<game*, std::pair<int, int>, std::pair<int, int>>& game_and_coords) mutable
			{
				game* game_copy = std::get<0>(game_and_coords);
				std::pair<std::pair<int, int>, std::pair<int, int>> best_move = find_best_move(game_copy, depth - 1, s_min_score, s_max_score, !maximizing_player);
				std::pair<int, int> selected_coords = std::get<0>(best_move);
				std::pair<int, int> saved_coords = std::get<1>(best_move);

				if (selected_coords != std::make_pair(-1, -1) && saved_coords != std::make_pair(-1, -1))
				{
					std::vector<std::vector<piece*>>* board = game_copy->get_board();
					
					int x = std::get<0>(selected_coords);
					int y = std::get<1>(selected_coords);
					int new_x = std::get<0>(saved_coords);
					int new_y = std::get<1>(saved_coords);
					
					piece* moving_piece = (*board)[x][y];
					assert(moving_piece);
					game_copy->set_selected_piece(moving_piece);
					
					bool there_is_found_move = false;
					for (available_move* a : *moving_piece->get_av_list())
					{
						assert(a);
						if (a->get_x() == new_x && a->get_y() == new_y)
						{
							available_capture* found_capture = dynamic_cast<available_capture*>(a);
							if (found_capture)
							{
								int x_d = found_capture->get_x_d();
								int y_d = found_capture->get_y_d();

								if (x_d > x && y_d < y)
									game_copy->set_last_capture_direction(3);
								else if (x_d < x && y_d < y)
									game_copy->set_last_capture_direction(2);
								else if (x_d > x && y_d > y)
									game_copy->set_last_capture_direction(1);
								else if (x_d < x && y_d > y)
									game_copy->set_last_capture_direction(0);
								else
									throw std::runtime_error("Capturing in wrong direction");

								// check game completion
								if (game_copy->get_game_state()->get_next_player()->get_list()->empty())
								{
									if (game_copy->get_game_state()->get_current_player()->is_first())
										game_copy->get_game_state()->set_first_won(true);
									else
										game_copy->get_game_state()->set_second_won(true);
#ifdef _DEBUG
									game_copy->get_os() << "Setting the end of the game flags: current player has no pieces" << std::endl;
#endif
								}

								// if there is no multicapture, set new moving piece
								if (game_copy->get_to_delete_list()->empty())
									game_copy->set_moving_piece(game_copy->get_selected_piece());

								// create new piece which represents dead piece during multicapture, it is indifferent whether it was normal piece or king
								game_copy->get_to_delete_list()->push_back(new piece(x_d, y_d, false, game_copy->get_game_state()->get_next_player(), nullptr));
								game_copy->get_game_state()->get_current_player()->set_combo(true);
#ifdef _DEBUG
								game_copy->get_os() << game_copy->get_game_state()->get_current_player()->get_name() << " combo" << std::endl;
#endif
							}
							game_copy->move_piece(game_copy->get_selected_piece(), game_copy->get_board(), new_x, new_y);
							(*game_copy->get_board())[x][y] = game_copy->get_selected_piece();
							game_copy->set_selected(false);

							there_is_found_move = true;
							break;
						}
					}
					assert(there_is_found_move);
				}
			});
#ifdef _DEBUG	
		// print scores after each simulation
		for_each(list_of_games.begin(), list_of_games.end(), [i = 0, this](std::tuple<game*, std::pair<int, int>, std::pair<int, int>>& game_and_coords) mutable
			{
				m_os << "simulation score (after recursion): " << ++i << ": " << std::get<0>(game_and_coords)->get_score() << std::endl;
			});
#endif
		// find max score of all simulations
		if (maximizing_player)
		{
			int max_score = s_min_score;
			for (std::tuple<game*, std::pair<int, int>, std::pair<int, int>>& game_and_coords : list_of_games)
			{
				int current_score = std::get<0>(game_and_coords)->get_score();
				if (current_score > max_score)
				{
					max_score = current_score;
					best_moves.clear();
					best_moves.emplace_back(game_and_coords);
				}
				else if (current_score == max_score)
					best_moves.emplace_back(game_and_coords);

				// beta cutoff
				alpha = std::max(alpha, max_score);
				if (beta <= alpha)
				{
					//delete game_copy;
					break;
				}
			}
		}
		else // minimizing
		{
			int min_score = s_max_score;
			for (std::tuple<game*, std::pair<int, int>, std::pair<int, int>>& game_and_coords : list_of_games)
			{
				int current_score = std::get<0>(game_and_coords)->get_score();
				if (current_score < min_score)
				{
					min_score = current_score;
					best_moves.clear();
					best_moves.emplace_back(game_and_coords);
				}
				else if (current_score == min_score)
					best_moves.emplace_back(game_and_coords);

				// alpha cutoff
				beta = std::min(beta, min_score);
				if (beta <= alpha)
				{
					//delete game_copy;
					break;
				}
			}
		}
		
		return best_moves;
	}

	void bot::add_to_game_copy_list(std::list<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>>& list_of_games, game* game_copy, std::pair<int, int>* src_coords, std::pair<int, int>* dest_coords)
	{
		// make checks
		bool at_least_one_capture_available = game_copy->get_available_capture();
		if (at_least_one_capture_available)
		{
			for_each(game_copy->get_game_state()->get_current_player()->get_list()->begin(), game_copy->get_game_state()->get_current_player()->get_list()->end(), [](piece* p)
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
			for_each(game_copy->get_game_state()->get_current_player()->get_list()->begin(), game_copy->get_game_state()->get_current_player()->get_list()->end(), [&src_coords, &dest_coords, &game_copy, &list_of_games, this](piece* p)
				{
					for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&src_coords, &dest_coords, &game_copy, &p, &list_of_games, this](available_move* a)
						{
							assert(a);
							available_capture* found_capture = dynamic_cast<available_capture*>(a);
							assert(found_capture); // not working in release mode
							if (found_capture)
							{
								int old_x = p->get_x();
								int old_y = p->get_y();

								int new_x = found_capture->get_x();
								int new_y = found_capture->get_y();

								int deleted_x = found_capture->get_x_d();
								int deleted_y = found_capture->get_y_d();

								game* local_copy = new game(*game_copy);
								std::vector<std::vector<piece*>>* board = local_copy->get_board();
								piece* moving_piece = (*board)[old_x][old_y];
								local_copy->set_moving_piece(moving_piece);

								// save capture direction: 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left
								if (deleted_x > new_x && deleted_y < new_y)
									local_copy->set_last_capture_direction(3);
								else if (deleted_x < new_x && deleted_y < new_y)
									local_copy->set_last_capture_direction(2);
								else if (deleted_x > new_x && deleted_y > new_y)
									local_copy->set_last_capture_direction(1);
								else if (deleted_x < new_x && deleted_y > new_y)
									local_copy->set_last_capture_direction(0);
								else
									throw std::runtime_error("Capturing in wrong direction");

								piece* to_delete_piece = (*board)[found_capture->get_x_d()][found_capture->get_y_d()];

								std::list<piece*>* to_delete_list = local_copy->get_to_delete_list();
								local_copy->make_capture(board, moving_piece, to_delete_piece, new_x, new_y, to_delete_list);
								int dummy = 0;
								local_copy->set_available_capture(local_copy->evaluate(local_copy->get_game_state()->get_current_player()->get_list(), local_copy->get_board(), &dummy, dummy, local_copy->get_last_capture_direction(), local_copy->get_to_delete_list(), local_copy->get_moving_piece()));

								bool changed_argument = false;
								if (!src_coords)
								{
									changed_argument = true;
									src_coords = new std::pair<int, int>(std::make_pair(old_x, old_y));
									dest_coords = new std::pair<int, int>(std::make_pair(new_x, new_y));
								}

								// recursively go through all captures
								add_to_game_copy_list(list_of_games, local_copy, src_coords, dest_coords);

								if (changed_argument)
								{
									src_coords = nullptr;
									dest_coords = nullptr;
								}
							}
						});
				});

		}
		else
		{
			game_copy->get_game_state()->get_current_player()->set_combo(false);
			game_copy->clear_to_delete_list(game_copy->get_to_delete_list(), &game_copy->get_list_1());
			game_copy->clear_to_delete_list(game_copy->get_to_delete_list(), &game_copy->get_list_2());
			game_copy->set_moving_piece(nullptr);
			list_of_games.push_back(std::make_tuple(game_copy, *src_coords, *dest_coords));
		}
	}
}