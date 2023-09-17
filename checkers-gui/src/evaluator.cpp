#include <evaluator.h>
#include <king.h>

namespace checkers
{
	evaluator::evaluator(game_state* game_state, std::ostream& os) : m_game_state(game_state), m_os(os) {}

	evaluator::~evaluator() {}

	void evaluator::clear_list(std::list<piece*>* list) { assert(list); for_each(list->begin(), list->end(), [this](piece* p) { assert(p); p->get_av_list()->clear(); }); }

	bool evaluator::evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece) // add moving piece
	{
		bool av_capture = false;
		clear_list(list);
#ifdef _DEBUG
		if (counter)
			m_os << "Entered outer evaluation with counter: " << *counter << "; recursive integer: " << recursive << std::endl;
		else
			m_os << "Entered outer evaluation with empty counter and recursive integer: " << recursive << std::endl;
#endif
		// is there a multicapture
		if (moving_piece)
		{
#ifdef _DEBUG
			m_os << "Multicapture evaluation" << std::endl;
#endif
			if (dynamic_cast<king*>(moving_piece))
				av_capture = evaluate_piece(dynamic_cast<king*>(moving_piece), list, board, counter, recursive, last_capture_direction, dead_list);
			else
				av_capture = evaluate_piece(moving_piece, list, board, counter, recursive);
		}
		else
		{
			assert(dead_list);
#ifdef _DEBUG
			m_os << "Non-multicapture evaluation" << std::endl;
#endif
			if (list->empty())
			{
#ifdef _DEBUG
				m_os << "Evaluating empty list" << std::endl;
#else
				// in release mode, evaluating an empty list is end-of-game 
				get_game_state()->check_lists();
				m_os << "Setting the end of the game flags: no possible moves for current player evaluated" << std::endl;
#endif
			}
			else
			{
				// evaluate moves and captures for all of the pieces
				for_each(list->begin(), list->end(), [this, &board, &list, &counter, &recursive, &av_capture, &last_capture_direction, &dead_list](piece* p)
					{
						bool local_available = false;
						if (dynamic_cast<king*>(p))
						{
							local_available = evaluate_piece(dynamic_cast<king*>(p), list, board, counter, recursive, last_capture_direction, dead_list);
							if (local_available)
								av_capture = true;
						}
						else
						{
							local_available = evaluate_piece(p, list, board, counter, recursive);
							if (local_available)
								av_capture = true;
						}
					});

				// check, when there wasn't multicapture, but there are two or more pieces that can do captures
				int pieces_with_captures = 0;
				for (piece* p : *list)
				{
					for (available_move* a : *p->get_av_list())
					{
						// if at least one of available moves is a capture, increment and go to the next piece
						if (dynamic_cast<available_capture*>(a))
						{
							pieces_with_captures++;
							break;
						}
					}
					// stop checking, if there is at least two pieces containing separate captures
					if (pieces_with_captures >= 2)
						break;
				}

				// delete normal moves
				if (pieces_with_captures >= 1)
				{
					// delete possible captures with score lower than max
					for_each(list->begin(), list->end(), [this](piece* p)
						{
							std::list<available_move*> to_delete;
							for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &to_delete, &p](available_move* a)
								{
									if (!dynamic_cast<available_capture*>(a))
									{
										to_delete.push_back(a);
#ifdef _DEBUG
										m_os << "deleting move: x: " << a->get_x() << "; y: " << a->get_y() << std::endl;
#endif
									}
								});
							while (!to_delete.empty())
							{
								available_move* a = to_delete.front();
								p->get_av_list()->remove(a);
								to_delete.pop_front();
							}
						});
				}

				// delete captures, that are lower in multicapture count
				if (pieces_with_captures >= 2)
				{
					// find maximal capture counter over all captures
					int max_captures_overall = 0;
					for_each(list->begin(), list->end(), [&max_captures_overall](piece* p)
						{
							for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [&max_captures_overall](available_move* a)
								{
									if (dynamic_cast<available_capture*>(a))
									{
										int captures = dynamic_cast<available_capture*>(a)->get_max_score();
										if (captures > max_captures_overall)
											max_captures_overall = captures;
									}
								});
						});
#ifdef _DEBUG
					m_os << "Max captures overall: " << max_captures_overall << std::endl;
#endif
					// delete possible captures with score lower than max
					for_each(list->begin(), list->end(), [this, &max_captures_overall](piece* p)
						{
							std::list<available_move*> to_delete;
							for_each(p->get_av_list()->begin(), p->get_av_list()->end(), [this, &to_delete, &max_captures_overall, &p](available_move* a)
								{
									if (dynamic_cast<available_capture*>(a))
									{
										int captures = dynamic_cast<available_capture*>(a)->get_max_score();
										if (captures < max_captures_overall)
										{
											to_delete.push_back(a);
#ifdef _DEBUG
											m_os << "deleting capture: x: " << a->get_x() << "; y: " << a->get_y() << "; captures: " << captures << std::endl;
#endif
										}
									}
								});
							while (!to_delete.empty())
							{
								available_move* a = to_delete.front();
								p->get_av_list()->remove(a);
								to_delete.pop_front();
							}
						});
				}
			}
#ifdef _DEBUG
			m_os << "Evaluation returns: ";
			av_capture ? (m_os << "true") : (m_os << "false");
			m_os << std::endl;
#endif
		}
		return av_capture;
	}

	bool evaluator::evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive)
	{
		assert(dynamic_cast<piece*>(p));

		bool av_capture = false;

		// x coordinate of evaluated piece
		int x = p->get_x();
		// y coordinate of evaluated piece
		int y = p->get_y();

#ifdef _DEBUG
		m_os << "evaluating normal piece" << std::endl;
		m_os << "x: " << x << "; y: " << y << std::endl;

		if ((*board)[x][y])
			m_os << (*board)[x][y] << std::endl;
#endif
		// captures
		bool possible_capture_top_left = false;
		bool possible_capture_top_right = false;
		bool possible_capture_bottow_left = false;
		bool possible_capture_bottom_right = false;

		// players
		base_player* player = m_game_state->get_current_player();
		base_player* opponent = m_game_state->get_next_player();

		// capture top right (0)
		if (x + 2 <= s_size - 1 && y - 2 >= 0 && (*board)[x + 1][y - 1] && (*board)[x + 1][y - 1]->get_owner() == opponent && (*board)[x + 2][y - 2] == nullptr)
			possible_capture_top_right = true;

		// capture top left (1)
		if (x - 2 >= 0 && y - 2 >= 0 && (*board)[x - 1][y - 1] && (*board)[x - 1][y - 1]->get_owner() == opponent && (*board)[x - 2][y - 2] == nullptr)
			possible_capture_top_left = true;

		// capture bottom right (2)
		if (x + 2 <= s_size - 1 && y + 2 <= s_size - 1 && (*board)[x + 1][y + 1] && (*board)[x + 1][y + 1]->get_owner() == opponent && (*board)[x + 2][y + 2] == nullptr)
			possible_capture_bottom_right = true;

		// capture bottom left (3)
		if (x - 2 >= 0 && y + 2 <= s_size - 1 && (*board)[x - 1][y + 1] && (*board)[x - 1][y + 1]->get_owner() == opponent && (*board)[x - 2][y + 2] == nullptr)
			possible_capture_bottow_left = true;

		int x_sign = 0;
		int y_sign = 0;
		int index = -1;

		if (possible_capture_top_left || possible_capture_top_right || possible_capture_bottow_left || possible_capture_bottom_right)
		{
			// mark returned evaluation as true
			av_capture = true;

			// evaluate copy of the board recursively in every direction and find highest number of captures to add to base moves list
			int capture_counter[4] = { 0, 0, 0, 0 }; // 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left

			//
			int last_capture_direction;

			if (possible_capture_top_right)
			{
				last_capture_direction = 0;

				//(*p).get_av_list()->push_back(new available_capture(x + 2, y - 2, x + 1, y - 1));
				capture_counter[0] = 1; // change here to get from counter, then increment?

				// copy the board and make empty list for moved piece
				std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
				std::list<piece*> copy_of_list;
				copy_board(board, copy_of_board, player, opponent);

				// make planned move
				piece* moving_piece = (*copy_of_board)[x][y];
				move_piece(moving_piece, copy_of_board, x + 2, y - 2);
				copy_of_list.push_back(moving_piece);

				piece* deleted_piece = (*copy_of_board)[x + 1][y - 1];
				delete_piece(deleted_piece, copy_of_board, opponent);
#ifdef _DEBUG
				m_os << copy_of_board << std::endl;
#endif
				//evaluate recursively - separate in every direction - call tree
				if (!(*counter))
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: outer call" << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, &(capture_counter[0]), 1, last_capture_direction, nullptr, moving_piece);
#ifdef _DEBUG
					m_os << "moves counter (top right): " << capture_counter[0] << std::endl;
#endif
				}
				else
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, counter, recursive + 1, last_capture_direction, nullptr, moving_piece);
				}
			}

			if (possible_capture_top_left)
			{
				last_capture_direction = 1;

				capture_counter[1] = 1;

				// copy the board and make empty list for moved piece
				std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
				std::list<piece*> copy_of_list;
				copy_board(board, copy_of_board, player, opponent);

				// make planned move
				piece* moving_piece = (*copy_of_board)[x][y];
				(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = nullptr;
				moving_piece->set_x(x - 2);
				moving_piece->set_y(y - 2);
				(*copy_of_board)[x - 2][y - 2] = moving_piece;
				copy_of_list.push_back(moving_piece);
				(*copy_of_board)[x - 1][y - 1] = nullptr;
				moving_piece = (*copy_of_board)[x - 2][y - 2];

#ifdef _DEBUG
				m_os << copy_of_board << std::endl;
#endif

				//evaluate recursively - separate in every direction - call tree
				if (!(*counter))
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: outer call" << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, &(capture_counter[1]), 1, last_capture_direction, nullptr, moving_piece);
#ifdef _DEBUG
					m_os << "moves counter (top left): " << capture_counter[1] << std::endl;
#endif
				}
				else
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, counter, recursive + 1, last_capture_direction, nullptr, moving_piece);
				}
			}

			if (possible_capture_bottom_right)
			{
				last_capture_direction = 2;

				capture_counter[2] = 1;

				// copy the board and make empty list for moved piece
				std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
				std::list<piece*> copy_of_list;
				copy_board(board, copy_of_board, player, opponent);

				// make planned move
				piece* moving_piece = (*copy_of_board)[x][y];
				(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = nullptr;
				moving_piece->set_x(x + 2);
				moving_piece->set_y(y + 2);
				(*copy_of_board)[x + 2][y + 2] = moving_piece;
				copy_of_list.push_back(moving_piece);
				(*copy_of_board)[x + 1][y + 1] = nullptr;
				moving_piece = (*copy_of_board)[x + 2][y + 2];

#ifdef _DEBUG
				m_os << copy_of_board << std::endl;
#endif

				//evaluate recursively - separate in every direction - call tree
				if (!(*counter))
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: outer call" << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, &(capture_counter[2]), 1, last_capture_direction, nullptr, moving_piece);
#ifdef _DEBUG	
					m_os << "moves counter (bottom right): " << capture_counter[2] << std::endl;
#endif
				}
				else
				{
#ifdef _DEBUG
					m_os << "counter not nullptr" << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, counter, recursive + 1, last_capture_direction, nullptr, moving_piece);
				}
			}

			if (possible_capture_bottow_left)
			{
				last_capture_direction = 3;

				capture_counter[3] = 1;

				// copy the board and make empty list for moved piece
				std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
				std::list<piece*> copy_of_list;
				copy_board(board, copy_of_board, player, opponent);

				// make planned move
				piece* moving_piece = (*copy_of_board)[x][y];
				(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = nullptr;
				moving_piece->set_x(x - 2);
				moving_piece->set_y(y + 2);
				(*copy_of_board)[x - 2][y + 2] = moving_piece;
				copy_of_list.push_back(moving_piece);
				(*copy_of_board)[x - 1][y + 1] = nullptr;
				moving_piece = (*copy_of_board)[x - 2][y + 2];

#ifdef _DEBUG
				m_os << copy_of_board << std::endl;
#endif	

				//evaluate recursively - separate in every direction - call tree
				if (!(*counter))
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: outer call" << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, &(capture_counter[3]), 1, last_capture_direction, nullptr, moving_piece);
#ifdef _DEBUG
					m_os << "moves counter (bottom left): " << capture_counter[3] << std::endl;
#endif
				}
				else
				{
#ifdef _DEBUG
					m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif
					evaluate(&copy_of_list, copy_of_board, counter, recursive + 1, last_capture_direction, nullptr, moving_piece);
				}
			} // all recursive function madei


			//find max counter
			int max = capture_counter[0];
			for (int i = 1; i < 4; ++i)
				if (capture_counter[i] > max)
					max = capture_counter[i];
#ifdef _DEBUG
			m_os << "found max counter: " << max << std::endl;
#endif

			// if counter == max push back available capture
			for (int i = 0; i < 4; ++i)
				if (capture_counter[i] == max)
				{
					switch (i)
					{
					case 0:
					{
#ifdef _DEBUG
						m_os << "top right direction: " << capture_counter[0] << std::endl;
#endif
						(*p).get_av_list()->push_back(new available_capture(x + 2, y - 2, x + 1, y - 1, max));
						break;
					}
					case 1:
					{
#ifdef _DEBUG
						m_os << "top left direction: " << capture_counter[1] << std::endl;
#endif
						(*p).get_av_list()->push_back(new available_capture(x - 2, y - 2, x - 1, y - 1, max));
						break;
					}
					case 2:
					{
#ifdef _DEBUG
						m_os << "bottom right direction: " << capture_counter[2] << std::endl;
#endif
						(*p).get_av_list()->push_back(new available_capture(x + 2, y + 2, x + 1, y + 1, max));
						break;
					}
					case 3:
					{
#ifdef _DEBUG
						m_os << "bottom left direction: " << capture_counter[3] << std::endl;
#endif
						(*p).get_av_list()->push_back(new available_capture(x - 2, y + 2, x - 1, y + 1, max));
						break;
					}
					default:
						throw std::exception("Evaluation error on piece " + p->get_owner()->get_sign());
					}
				}
		}
		else
		{
			if (recursive != 0 && recursive > *counter)
				*counter = recursive;

			if (!(*counter))
			{
				// different direction of ordinary move
				if (player->is_first())
				{
					// moves to right
					if (x != s_size - 1 && y != 0)
					{
						if ((*board)[x + 1][y - 1] == nullptr)
						{
#ifdef _DEBUG
							m_os << "available move to the right!" << std::endl;
#endif
							(*p).get_av_list()->push_back(new available_move(x + 1, y - 1));
						}
					}

					// moves to left
					if (x != 0 && y != 0)
					{
						if ((*board)[x - 1][y - 1] == nullptr)
						{
#ifdef _DEBUG
							m_os << "available move to the left!" << std::endl;
#endif
							(*p).get_av_list()->push_back(new available_move(x - 1, y - 1));
						}
					}
				}
				else // next player is primary player (first, lower on board): checked by assertions
				{
					// moves to right
					if (x != s_size - 1 && y != s_size - 1)
					{
						if ((*board)[x + 1][y + 1] == nullptr)
						{
#ifdef _DEBUG
							m_os << "available move to the right!" << std::endl;
#endif		
							(*p).get_av_list()->push_back(new available_move(x + 1, y + 1));
						}
					}

					// moves to left
					if (x != 0 && y != s_size - 1)
					{
						if ((*board)[x - 1][y + 1] == nullptr)
						{
#ifdef _DEBUG
							m_os << "available move to the left!" << std::endl;
#endif
							(*p).get_av_list()->push_back(new available_move(x - 1, y + 1));
						}
					}
				}
			}
		}
#ifdef _DEBUG
		m_os << "piece evaluation returns: ";
		av_capture ? (m_os << "true") : (m_os << "false");
		m_os << std::endl;
#endif
		return av_capture;
	}

	bool evaluator::evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list)
	{
		assert(dynamic_cast<king*>(p));

		bool av_capture = false;

		// x coordinate of evaluated piece
		int x = p->get_x();
		// y coordinate of evaluated piece
		int y = p->get_y();

#ifdef _DEBUG
		m_os << "evaluating the king" << std::endl;
		m_os << "x: " << x << "; y: " << y << std::endl;

		if ((*board)[x][y] != nullptr)
			m_os << (*board)[x][y] << std::endl;
#endif  
		// flags blocking opposite captures in multicapture
		bool possible_top_right = true;
		bool possible_top_left = true;
		bool possible_bottom_right = true;
		bool possible_bottom_left = true;

		// players
		base_player* player = m_game_state->get_current_player();
		base_player* opponent = m_game_state->get_next_player();

		bool multicapture = !(*dead_list).empty();

		if (multicapture) // cannot capture in one direction and then in opposite
		{
			switch (last_capture_direction)
			{
			case 0: // top right
				possible_bottom_left = false;
				break;
			case 1: // top left
				possible_bottom_right = false;
				break;
			case 2: // bottom right
				possible_top_left = false;
				break;
			case 3: // bottom left
				possible_top_right = false;
				break;
			default:
				throw std::runtime_error("King evaluation: wrong member: last capture direction");
			}
		}
#ifdef _DEBUG
		if (!possible_top_right)
			m_os << "/ top right capture won't be available" << std::endl;
		else if (!possible_top_left)
			m_os << "/ top left capture won't be available" << std::endl;
		else if (!possible_bottom_right)
			m_os << "/ bottom right capture won't be available" << std::endl;
		else if (!possible_bottom_left)
			m_os << "/ bottom left capture won't be available" << std::endl;
#endif

		// vector, where true indicates a piece to be captured on specific place, it can be captured going into a few separate locations
		std::vector<bool> possible_capture_top_right;
		std::vector<bool> possible_capture_top_left;
		std::vector<bool> possible_capture_bottom_right;
		std::vector<bool> possible_capture_bottom_left;

		// condensed form of the lists above
		bool at_least_one_capture_top_right = false;
		bool at_least_one_capture_top_left = false;
		bool at_least_one_capture_bottom_right = false;
		bool at_least_one_capture_bottom_left = false;

		// lists (vectors) of moves, that can be made when no capture is available
		std::vector<available_move> local_moves_top_right;
		std::vector<available_move> local_moves_top_left;
		std::vector<available_move> local_moves_bottom_right;
		std::vector<available_move> local_moves_bottom_left;

		// lists (vectors) of captures: one capture in each direction, every in possible location
		std::vector<available_capture> local_captures_top_right;
		std::vector<available_capture> local_captures_top_left;
		std::vector<available_capture> local_captures_bottom_right;
		std::vector<available_capture> local_captures_bottom_left;

		// temporary board made from dead list
		std::vector<std::vector<piece*>> dead_board(s_size, std::vector<piece*>(s_size, nullptr));
		for_each(dead_list->begin(), dead_list->end(), [&dead_board, this](piece* p)
			{
				int x = p->get_x();
				int y = p->get_y();
				if (dead_board[x][y] == nullptr)
				{
					if (dynamic_cast<king*>(p))
						dead_board[x][y] = new king(x, y, p->is_alive(), p->get_owner());
					else
						dead_board[x][y] = new piece(x, y, p->is_alive(), p->get_owner());
				}
				else
					throw std::runtime_error("Copying to delete list: wrong piece data: overwriting another piece");
			});
#ifdef _DEBUG
		m_os << "Dead board:" << std::endl;
		m_os << &dead_board << std::endl;
#endif
		int i;
		if (possible_top_right)
		{
			// capture top right (0) + -
#ifdef _DEBUG
			m_os << "top right checking" << std::endl;
#endif
			i = 1;
			while (x + i + 1 <= s_size - 1 && y - i - 1 >= 0)
			{
#ifdef _DEBUG
				m_os << "checking: x: " << x + i << ", y: " << y - i << " and place to go: x: " << x + i + 1 << ", y: " << y - i - 1 << std::endl;
#endif
				// searching for own piece (cannot jump across them)
				if ((*board)[x + i][y - i] != nullptr && (*board)[x + i][y - i]->get_owner() == player)
				{
#ifdef _DEBUG
					m_os << "*found own piece and breaking" << std::endl;
#endif
					possible_capture_top_right.push_back(false);
					break;
				}
				// searching for dead piece (cannot jump across them)
				else if (dead_board[x + i][y - i] != nullptr && dead_board[x + i][y - i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found dead piece and breaking" << std::endl;
#endif
					possible_capture_top_right.push_back(false);
					break;
				}
				else if ((*board)[x + i][y - i] != nullptr && (*board)[x + i][y - i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found opponent's piece and checking for next fields" << std::endl;
#endif
					if ((*board)[x + i + 1][y - i - 1] == nullptr && dead_board[x + i + 1][y - i - 1] == nullptr)
					{
#ifdef _DEBUG
						m_os << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
						possible_capture_top_right.push_back(true);
						at_least_one_capture_top_right = true;

						// adding all possible capture options
						int j = 0;
						while (x + i + 1 + j <= s_size - 1 && y - i - 1 - j >= 0 && (*board)[x + i + 1 + j][y - i - 1 - j] == nullptr && dead_board[x + i + 1 + j][y - i - 1 - j] == nullptr)
						{
#ifdef _DEBUG
							m_os << "=piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x + i + 1 + j << ", y: " << y - i - 1 - j << std::endl;
#endif
							local_captures_top_right.push_back(available_capture(x + i + 1 + j, y - i - 1 - j, x + i, y - i, 1));
							++j;
						}
#ifdef _DEBUG
						m_os << "found and added all possible capture options, continuing" << std::endl;
#endif
					}
					else
					{
#ifdef _DEBUG
						m_os << "*next piece is not empty and braking" << std::endl;
#endif
						possible_capture_top_right.push_back(false);
						break;
					}
				}
				else // empty field
				{
					if ((*board)[x + i + 1][y - i - 1] != nullptr)
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
						if (at_least_one_capture_top_right)
						{
#ifdef _DEBUG
							m_os << "*no more places to check, it is a singular check" << std::endl;
#endif
							break;
						}
					}
					else
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty" << std::endl;
#endif
					}
					local_moves_top_right.push_back(available_move(x + i, y - i));
					possible_capture_top_right.push_back(false);
				}

				++i;
			}
			if (x + i <= s_size - 1 && y - i >= 0 && (*board)[x + i][y - i] == nullptr && !at_least_one_capture_top_right) // last, not checked field (checking looks for next which is outside the boundaries)
			{
#ifdef _DEBUG
				m_os << "last but not least" << std::endl;
#endif
				local_moves_top_right.push_back(available_move(x + i, y - i));
			}
		}

		if (possible_top_left)
		{
			// capture top left (1) - -
#ifdef _DEBUG
			m_os << "top left checking" << std::endl;
#endif
			i = 1;
			while (x - i - 1 >= 0 && y - i - 1 >= 0)
			{
#ifdef _DEBUG
				m_os << "checking: x: " << x - i << ", y: " << y - i << " and place to go: x: " << x - i - 1 << ", y: " << y - i - 1 << std::endl;
#endif
				// searching for own piece (cannot jump across them)
				if ((*board)[x - i][y - i] != nullptr && (*board)[x - i][y - i]->get_owner() == player)
				{
#ifdef _DEBUG
					m_os << "*found own piece and breaking" << std::endl;
#endif
					possible_capture_top_left.push_back(false);
					break;
				}
				// searching for dead piece (cannot jump across them)
				else if (dead_board[x - i][y - i] != nullptr && dead_board[x - i][y - i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found dead piece and breaking" << std::endl;
#endif
					possible_capture_top_right.push_back(false);
					break;
				}
				else if ((*board)[x - i][y - i] != nullptr && (*board)[x - i][y - i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found opponent's piece and checking for next fields" << std::endl;
#endif
					if ((*board)[x - i - 1][y - i - 1] == nullptr && dead_board[x - i - 1][y - i - 1] == nullptr)
					{
#ifdef _DEBUG
						m_os << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
						possible_capture_top_left.push_back(true);
						at_least_one_capture_top_left = true;

						// adding all possible capture options
						int j = 0;
						while (x - i - 1 - j >= 0 && y - i - 1 - j >= 0 && (*board)[x - i - 1 - j][y - i - 1 - j] == nullptr && dead_board[x - i - 1 - j][y - i - 1 - j] == nullptr)
						{
#ifdef _DEBUG
							m_os << "=piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x - i - 1 - j << ", y: " << y - i - 1 - j << std::endl;
#endif
							local_captures_top_left.push_back(available_capture(x - i - 1 - j, y - i - 1 - j, x - i, y - i, 1));
							++j;
						}
#ifdef _DEBUG
						m_os << "found and added all possible capture options, continuing" << std::endl;
#endif
					}
					else
					{
#ifdef _DEBUG
						m_os << "*next piece is not empty and braking" << std::endl;
#endif
						possible_capture_top_left.push_back(false);
						break;
					}
				}
				else // empty field
				{
					if ((*board)[x - i - 1][y - i - 1] != nullptr)
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
						if (at_least_one_capture_top_left)
						{
#ifdef _DEBUG
							m_os << "*no more places to check, it is a singular check" << std::endl;
#endif
							break;
						}
					}
					else
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty" << std::endl;
#endif
					}
					local_moves_top_left.push_back(available_move(x - i, y - i));
					possible_capture_top_left.push_back(false);
				}
				++i;
			}
			if (x - i >= 0 && y - i >= 0 && (*board)[x - i][y - i] == nullptr && !at_least_one_capture_top_left) // last, not checked field (checking looks for next which is outside the boundaries)
			{
#ifdef _DEBUG
				m_os << "last but not least" << std::endl;
#endif
				local_moves_top_left.push_back(available_move(x - i, y - i));
			}
		}


		if (possible_bottom_right)
		{
			// capture bottom right (2) + +
#ifdef _DEBUG
			m_os << "bottom right checking" << std::endl;
#endif
			i = 1;
			while (x + i + 1 <= s_size - 1 && y + i + 1 <= s_size - 1)
			{
#ifdef _DEBUG
				m_os << "checking: x: " << x + i << ", y: " << y + i << " and place to go: x: " << x + i + 1 << ", y: " << y + i + 1 << std::endl;
#endif
				// searching for own piece (cannot jump across them)
				if ((*board)[x + i][y + i] != nullptr && (*board)[x + i][y + i]->get_owner() == player)
				{
#ifdef _DEBUG
					m_os << "*found own piece and breaking" << std::endl;
#endif
					possible_capture_bottom_right.push_back(false);
					break;
				}
				// searching for dead piece (cannot jump across them)
				else if (dead_board[x + i][y + i] != nullptr && dead_board[x + i][y + i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found dead piece and breaking" << std::endl;
#endif
					possible_capture_top_right.push_back(false);
					break;
				}
				else if ((*board)[x + i][y + i] != nullptr && (*board)[x + i][y + i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found opponent's piece and checking for next fields" << std::endl;
#endif
					if ((*board)[x + i + 1][y + i + 1] == nullptr && dead_board[x + i + 1][y + i + 1] == nullptr)
					{
#ifdef _DEBUG
						m_os << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
						possible_capture_bottom_right.push_back(true);
						at_least_one_capture_bottom_right = true;

						// adding all possible capture options
						int j = 0;
						while (x + i + 1 + j <= s_size - 1 && y + i + 1 + j <= s_size - 1 && (*board)[x + i + 1 + j][y + i + 1 + j] == nullptr && dead_board[x + i + 1 + j][y + i + 1 + j] == nullptr)
						{
#ifdef _DEBUG
							m_os << "=piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x + i + 1 + j << ", y: " << y + i + 1 + j << std::endl;
#endif
							local_captures_bottom_right.push_back(available_capture(x + i + 1 + j, y + i + 1 + j, x + i, y + i, 1));
							++j;
						}
#ifdef _DEBUG
						m_os << "found and added all possible capture options, continuing" << std::endl;
#endif
					}
					else
					{
#ifdef _DEBUG
						m_os << "*next piece is not empty and braking" << std::endl;
#endif
						possible_capture_bottom_right.push_back(false);
						break;
					}
				}
				else // empty field
				{
					if ((*board)[x + i + 1][y + i + 1] != nullptr)
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
						if (at_least_one_capture_bottom_right)
						{
#ifdef _DEBUG
							m_os << "*no more places to check, it is a singular check" << std::endl;
#endif
							break;
						}
					}
					else
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty" << std::endl;
#endif
					}
					local_moves_bottom_right.push_back(available_move(x + i, y + i));
					possible_capture_bottom_right.push_back(false);
				}

				++i;
			}
			if (x + i <= s_size - 1 && y + i <= s_size - 1 && (*board)[x + i][y + i] == nullptr && !at_least_one_capture_bottom_right) // last, not checked field (checking looks for next which is outside the boundaries)
			{
#ifdef _DEBUG
				m_os << "last but not least" << std::endl;
#endif
				local_moves_bottom_right.push_back(available_move(x + i, y + i));
			}
		}

		if (possible_bottom_left)
		{
			// capture bottom left (3) - +
#ifdef _DEBUG
			m_os << "bottom left checking" << std::endl;
#endif
			i = 1;
			while (x - i - 1 >= 0 && y + i + 1 <= s_size - 1)
			{
#ifdef _DEBUG
				m_os << "checking: x: " << x - i << ", y: " << y + i << " and place to go: x: " << x - i - 1 << ", y: " << y + i + 1 << std::endl;
#endif
				// searching for own piece (cannot jump across them)
				if ((*board)[x - i][y + i] != nullptr && (*board)[x - i][y + i]->get_owner() == player)
				{
#ifdef _DEBUG
					m_os << "*found own piece and breaking" << std::endl;
#endif
					possible_capture_bottom_left.push_back(false);
					break;
				}
				// searching for dead piece (cannot jump across them)
				else if (dead_board[x - i][y + i] != nullptr && dead_board[x - i][y + i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found dead piece and breaking" << std::endl;
#endif
					possible_capture_top_right.push_back(false);
					break;
				}
				else if ((*board)[x - i][y + i] != nullptr && (*board)[x - i][y + i]->get_owner() == opponent)
				{
#ifdef _DEBUG
					m_os << "*found opponent's piece and checking for next fields" << std::endl;
#endif
					if ((*board)[x - i - 1][y + i + 1] == nullptr && dead_board[x - i - 1][y + i + 1] == nullptr)
					{
#ifdef _DEBUG
						m_os << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
						possible_capture_bottom_left.push_back(true);
						at_least_one_capture_bottom_left = true;

						// adding all possible capture options
						int j = 0;
						while (x - i - 1 - j >= 0 && y + i + 1 + j <= s_size - 1 && (*board)[x - i - 1 - j][y + i + 1 + j] == nullptr && dead_board[x - i - 1 - j][y + i + 1 + j] == nullptr)
						{
#ifdef _DEBUG
							m_os << "=piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x - i - 1 - j << ", y: " << y + i + 1 + j << std::endl;
#endif
							local_captures_bottom_left.push_back(available_capture(x - i - 1 - j, y + i + 1 + j, x - i, y + i, 1));
							++j;
						}
#ifdef _DEBUG
						m_os << "found and added all possible capture options, continuing" << std::endl;
#endif
					}
					else
					{
#ifdef _DEBUG
						m_os << "*next piece is not empty and braking" << std::endl;
#endif
						possible_capture_bottom_left.push_back(false);
						break;
					}
				}
				else // empty field
				{
					if ((*board)[x - i - 1][y + i + 1] != nullptr)
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
						if (at_least_one_capture_bottom_left)
						{
#ifdef _DEBUG
							m_os << "*no more places to check, it is a singular check" << std::endl;
#endif
							break;
						}
					}
					else
					{
#ifdef _DEBUG
						m_os << "*there is no capture, checked field is empty" << std::endl;
#endif
					}
					local_moves_bottom_left.push_back(available_move(x - i, y + i));
					possible_capture_bottom_left.push_back(false);
				}

				++i;
			}
			if (x - i >= 0 && y + i <= s_size - 1 && (*board)[x - i][y + i] == nullptr && !at_least_one_capture_bottom_left) // last, not checked field (checking looks for next which is outside the boundaries)
			{
#ifdef _DEBUG
				m_os << "Added last possible field" << std::endl;
#endif
				local_moves_bottom_left.push_back(available_move(x - i, y + i));
			}
		}

#ifdef _DEBUG
		// list top right moves and captures
		m_os << "king top right moves" << std::endl;
		for_each(possible_capture_top_right.begin(), possible_capture_top_right.end(), [this, i = 1](bool b) mutable { m_os << i++ << ": " << b << std::endl; });

		m_os << "local moves" << std::endl;
		for_each(local_moves_top_right.begin(), local_moves_top_right.end(), [this](available_move& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

		m_os << "local captures" << std::endl;
		for_each(local_captures_top_right.begin(), local_captures_top_right.end(), [this](available_capture& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });

		// list top left moves and captures
		m_os << "king top left moves" << std::endl;
		for_each(possible_capture_top_left.begin(), possible_capture_top_left.end(), [this, i = 1](bool b) mutable { m_os << i++ << ": " << b << std::endl; });

		m_os << "local moves" << std::endl;
		for_each(local_moves_top_left.begin(), local_moves_top_left.end(), [this](available_move& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

		m_os << "local captures" << std::endl;
		for_each(local_captures_top_left.begin(), local_captures_top_left.end(), [this](available_capture& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });

		// list bottom right moves and captures
		m_os << "king bottom right moves" << std::endl;
		for_each(possible_capture_bottom_right.begin(), possible_capture_bottom_right.end(), [this, i = 1](bool b) mutable { m_os << i++ << ": " << b << std::endl; });

		m_os << "local moves" << std::endl;
		for_each(local_moves_bottom_right.begin(), local_moves_bottom_right.end(), [this](available_move& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

		m_os << "local captures" << std::endl;
		for_each(local_captures_bottom_right.begin(), local_captures_bottom_right.end(), [this](available_capture& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });

		// list bottom left moves and captures
		m_os << "king bottom left moves" << std::endl;
		for_each(possible_capture_bottom_left.begin(), possible_capture_bottom_left.end(), [this, i = 1](bool b) mutable { m_os << i++ << ": " << b << std::endl; });

		m_os << "local moves" << std::endl;
		for_each(local_moves_bottom_left.begin(), local_moves_bottom_left.end(), [this](available_move& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

		m_os << "local captures" << std::endl;
		for_each(local_captures_bottom_left.begin(), local_captures_bottom_left.end(), [this](available_capture& a) { m_os << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });
#endif

		if (at_least_one_capture_top_right || at_least_one_capture_top_left || at_least_one_capture_bottom_right || at_least_one_capture_bottom_left)
		{
			// mark returned evaluation as true
			av_capture = true;

			int top_right = (int)local_captures_top_right.size();
			int top_left = (int)local_captures_top_left.size();
			int bottom_right = (int)local_captures_bottom_right.size();
			int bottom_left = (int)local_captures_bottom_left.size();

			// for storing recursively evaluated capture counters
			std::vector<int> capture_counter[4] = { std::vector<int>(top_right), std::vector<int>(top_left), std::vector<int>(bottom_right), std::vector<int>(bottom_left) }; // 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left

			//
			int new_last_capture_direction;

			// top right recursive evaluation
			if (at_least_one_capture_top_right)
			{
				new_last_capture_direction = 0;
				for_each(local_captures_top_right.begin(), local_captures_top_right.end(), [i = 0, this, &capture_counter, &counter, &recursive, &board, &player, &opponent, &new_last_capture_direction, &x, &y, &dead_list](available_capture& a) mutable
					{
						capture_counter[0][i] = 1; // change here to get from counter, then increment?

						// copy coords
						int x_to_go = a.get_x();
						int y_to_go = a.get_y();
						int x_to_delete = a.get_x_d();
						int y_to_delete = a.get_y_d();

						// copy the board and make empty list for moved piece
						std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
						std::list<piece*>* copy_of_list = new std::list<piece*>;
						std::list<piece*>* copy_of_dead = new std::list<piece*>;
						copy_board(board, copy_of_board, player, opponent);
						for_each(dead_list->begin(), dead_list->end(), [&copy_of_dead](piece* p) { copy_of_dead->push_back(new piece(p->get_x(), p->get_y(), false, p->get_owner())); });

						// make planned move
						piece* moving_piece = (*copy_of_board)[x][y];
						piece* deleted_piece = (*copy_of_board)[x_to_delete][y_to_delete];
						make_capture(copy_of_board, moving_piece, deleted_piece, x_to_go, y_to_go, copy_of_dead);
						copy_of_list->push_back(moving_piece);
#ifdef _DEBUG
						m_os << copy_of_board << std::endl;
#endif
						//evaluate recursively - separate in every direction - call tree
						if (!(*counter))
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: outer call" << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, &(capture_counter[0][i]), 1, new_last_capture_direction, copy_of_dead, moving_piece);
#ifdef _DEBUG
							m_os << "moves counter (top right): " << capture_counter[0][i] << std::endl;

#endif
						}
						else
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif

							evaluate(copy_of_list, copy_of_board, counter, recursive + 1, new_last_capture_direction, copy_of_dead, moving_piece);
						}
						++i;
					});

#ifdef _DEBUG
				// print
				m_os << "for every capture option, these are multi capture counters" << std::endl;
				for_each(capture_counter[0].begin(), capture_counter[0].end(), [this, i = 1](int c) mutable { m_os << i++ << ": " << c << std::endl; });
#endif
			}

			// top left recursive evaluation
			if (at_least_one_capture_top_left)
			{
				new_last_capture_direction = 1;
				for_each(local_captures_top_left.begin(), local_captures_top_left.end(), [i = 0, this, &capture_counter, &counter, &recursive, &board, &player, &opponent, &new_last_capture_direction, &x, &y, &dead_list](available_capture& a) mutable
					{
						capture_counter[1][i] = 1; // change here to get from counter, then increment?

						// copy coords
						int x_to_go = a.get_x();
						int y_to_go = a.get_y();
						int x_to_delete = a.get_x_d();
						int y_to_delete = a.get_y_d();

						// copy the board and make empty list for moved piece
						std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
						std::list<piece*>* copy_of_list = new std::list<piece*>;
						std::list<piece*>* copy_of_dead = new std::list<piece*>;
						copy_board(board, copy_of_board, player, opponent);
						for_each(dead_list->begin(), dead_list->end(), [&copy_of_dead](piece* p) { copy_of_dead->push_back(new piece(p->get_x(), p->get_y(), false, p->get_owner())); });

						// make planned move
						piece* moving_piece = (*copy_of_board)[x][y];
						piece* deleted_piece = (*copy_of_board)[x_to_delete][y_to_delete];
						make_capture(copy_of_board, moving_piece, deleted_piece, x_to_go, y_to_go, copy_of_dead);
						copy_of_list->push_back(moving_piece);
#ifdef _DEBUG
						m_os << copy_of_board << std::endl;
#endif
						//evaluate recursively - separate in every direction - call tree
						if (!(*counter))
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: outer call" << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, &(capture_counter[1][i]), 1, new_last_capture_direction, copy_of_dead, moving_piece);
#ifdef _DEBUG
							m_os << "moves counter (top left): " << capture_counter[1][i] << std::endl;
#endif
						}
						else
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, counter, recursive + 1, new_last_capture_direction, copy_of_dead, moving_piece);
						}
						++i;
					});

#ifdef _DEBUG
				// print
				m_os << "for every capture option, these are multi capture counters" << std::endl;
				for_each(capture_counter[1].begin(), capture_counter[1].end(), [this, i = 1](int c) mutable { m_os << i++ << ": " << c << std::endl; });
#endif
			}

			// bottom right recursive evaluation
			if (at_least_one_capture_bottom_right)
			{
				new_last_capture_direction = 2;
				for_each(local_captures_bottom_right.begin(), local_captures_bottom_right.end(), [i = 0, this, &capture_counter, &counter, &recursive, &board, &player, &opponent, &new_last_capture_direction, &x, &y, &dead_list](available_capture& a) mutable
					{
						capture_counter[2][i] = 1; // change here to get from counter, then increment?

						// copy coords
						int x_to_go = a.get_x();
						int y_to_go = a.get_y();
						int x_to_delete = a.get_x_d();
						int y_to_delete = a.get_y_d();

						// copy the board and make empty list for moved piece
						std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
						std::list<piece*>* copy_of_list = new std::list<piece*>;
						std::list<piece*>* copy_of_dead = new std::list<piece*>;
						copy_board(board, copy_of_board, player, opponent);
						for_each(dead_list->begin(), dead_list->end(), [&copy_of_dead](piece* p) { copy_of_dead->push_back(new piece(p->get_x(), p->get_y(), false, p->get_owner())); });

						// make planned move
						piece* moving_piece = (*copy_of_board)[x][y];
						piece* deleted_piece = (*copy_of_board)[x_to_delete][y_to_delete];
						make_capture(copy_of_board, moving_piece, deleted_piece, x_to_go, y_to_go, copy_of_dead);
						copy_of_list->push_back(moving_piece);
#ifdef _DEBUG
						m_os << copy_of_board << std::endl;
#endif
						//evaluate recursively - separate in every direction - call tree
						if (!(*counter))
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: outer call" << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, &(capture_counter[2][i]), 1, new_last_capture_direction, copy_of_dead, moving_piece);
#ifdef _DEBUG
							m_os << "moves counter (bottom right): " << capture_counter[2][i] << std::endl;
#endif
						}
						else
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, counter, recursive + 1, new_last_capture_direction, copy_of_dead, moving_piece);
						}
						++i;
					});
#ifdef _DEBUG
				// print
				m_os << "for every capture option, these are multi capture counters" << std::endl;
				for_each(capture_counter[2].begin(), capture_counter[2].end(), [this, i = 1](int c) mutable { m_os << i++ << ": " << c << std::endl; });
#endif
			}

			// bottom left recursive evaluation
			if (at_least_one_capture_bottom_left)
			{
				new_last_capture_direction = 3;
				for_each(local_captures_bottom_left.begin(), local_captures_bottom_left.end(), [i = 0, this, &capture_counter, &counter, &recursive, &board, &player, &opponent, &new_last_capture_direction, &x, &y, &dead_list](available_capture& a) mutable
					{
						capture_counter[3][i] = 1; // change here to get from counter, then increment?

						// copy coords
						int x_to_go = a.get_x();
						int y_to_go = a.get_y();
						int x_to_delete = a.get_x_d();
						int y_to_delete = a.get_y_d();

						// copy the board and make empty list for moved piece
						std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
						std::list<piece*>* copy_of_list = new std::list<piece*>;
						std::list<piece*>* copy_of_dead = new std::list<piece*>;
						copy_board(board, copy_of_board, player, opponent);
						for_each(dead_list->begin(), dead_list->end(), [&copy_of_dead](piece* p) { copy_of_dead->push_back(new piece(p->get_x(), p->get_y(), false, p->get_owner())); });

						// make planned move
						piece* moving_piece = (*copy_of_board)[x][y];
						piece* deleted_piece = (*copy_of_board)[x_to_delete][y_to_delete];
						make_capture(copy_of_board, moving_piece, deleted_piece, x_to_go, y_to_go, copy_of_dead);
						copy_of_list->push_back(moving_piece);
#ifdef _DEBUG
						m_os << copy_of_board << std::endl;
#endif
						//evaluate recursively - separate in every direction - call tree
						if (!(*counter))
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: outer call" << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, &(capture_counter[3][i]), 1, new_last_capture_direction, copy_of_dead, moving_piece);
#ifdef _DEBUG
							m_os << "moves counter (bottom left): " << capture_counter[3][i] << std::endl;
#endif
						}
						else
						{
#ifdef _DEBUG
							m_os << "recursive evaluation: inner call; counter: " << (*counter) << std::endl;
#endif
							evaluate(copy_of_list, copy_of_board, counter, recursive + 1, new_last_capture_direction, copy_of_dead, moving_piece);
						}
						++i;
					});
#ifdef _DEBUG
				// print
				m_os << "for every capture option, these are multi capture counters" << std::endl;
				for_each(capture_counter[3].begin(), capture_counter[3].end(), [this, i = 1](int c) mutable { m_os << i++ << ": " << c << std::endl; });
#endif
			}

			// max captures
			int max_captures = 0;

			// save how many places have max captures
			int elements_with_max = 0;

			for (int i = 0; i < 4; ++i)
				for_each(capture_counter[i].begin(), capture_counter[i].end(), [&max_captures, &elements_with_max](int c)
					{
						if (c == max_captures)
							++elements_with_max;
						else if (c > max_captures)
						{
							max_captures = c;
							elements_with_max = 1;
						}
					});
#ifdef _DEBUG
			m_os << "found max captures: " << max_captures << std::endl;
			m_os << "elements with max captures: " << elements_with_max << std::endl;
#endif
			// actual adding captures to the piece
			for (int i = 0; i < 4; ++i)
				for_each(capture_counter[i].begin(), capture_counter[i].end(), [&max_captures, &i, &p, j = 0, &multicapture, &possible_top_right, &possible_top_left, &possible_bottom_right, &possible_bottom_left, &local_captures_top_right, &local_captures_top_left, &local_captures_bottom_right, &local_captures_bottom_left](int c) mutable
					{
						if (c == max_captures)
						{
							switch (i)
							{
							case 0:
								if (multicapture && possible_top_right || !multicapture)
									p->get_av_list()->push_back(new available_capture(local_captures_top_right[j].get_x(), local_captures_top_right[j].get_y(), local_captures_top_right[j].get_x_d(), local_captures_top_right[j].get_y_d(), max_captures));
								break;
							case 1:
								if (multicapture && possible_top_left || !multicapture)
									p->get_av_list()->push_back(new available_capture(local_captures_top_left[j].get_x(), local_captures_top_left[j].get_y(), local_captures_top_left[j].get_x_d(), local_captures_top_left[j].get_y_d(), max_captures));
								break;
							case 2:
								if (multicapture && possible_bottom_right || !multicapture)
									p->get_av_list()->push_back(new available_capture(local_captures_bottom_right[j].get_x(), local_captures_bottom_right[j].get_y(), local_captures_bottom_right[j].get_x_d(), local_captures_bottom_right[j].get_y_d(), max_captures));
								break;
							case 3:
								if (multicapture && possible_bottom_left || !multicapture)
									p->get_av_list()->push_back(new available_capture(local_captures_bottom_left[j].get_x(), local_captures_bottom_left[j].get_y(), local_captures_bottom_left[j].get_x_d(), local_captures_bottom_left[j].get_y_d(), max_captures));
								break;
							}
						}
						++j;
					});
		}
		else // only moves
		{
			if (recursive != 0 && recursive > *counter)
				*counter = recursive;

			if (multicapture && possible_top_right || !multicapture)
				for_each(local_moves_top_right.begin(), local_moves_top_right.end(), [&p](available_move& a)
					{
						p->get_av_list()->push_back(new available_move(a.get_x(), a.get_y()));
					});

			if (multicapture && possible_top_left || !multicapture)
				for_each(local_moves_top_left.begin(), local_moves_top_left.end(), [&p](available_move& a)
					{
						p->get_av_list()->push_back(new available_move(a.get_x(), a.get_y()));
					});

			if (multicapture && possible_bottom_right || !multicapture)
				for_each(local_moves_bottom_right.begin(), local_moves_bottom_right.end(), [&p](available_move& a)
					{
						p->get_av_list()->push_back(new available_move(a.get_x(), a.get_y()));
					});

			if (multicapture && possible_bottom_left || !multicapture)
				for_each(local_moves_bottom_left.begin(), local_moves_bottom_left.end(), [&p](available_move& a)
					{
						p->get_av_list()->push_back(new available_move(a.get_x(), a.get_y()));
					});
		}
#ifdef _DEBUG
		m_os << "king evaluation returns: ";
		av_capture ? (m_os << "true") : (m_os << "false");
		m_os << std::endl;
#endif
		return av_capture;
	}
}