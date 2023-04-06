#include "Game.h"
// todo: (*done)
// * enable only current turn moves
// * enable only possible moves (captures)
// * enable hit series
// add check if king to evaluation
//		if the counter won't work: get value from argument and copy to local variable, then pass it to resursive function call
// remake code into more functions

namespace Checkers
{
	Game::Game(int s) : is_finished(false), window(sf::VideoMode(square_size * size, square_size * size), "Checkers", sf::Style::Default, settings), selected_piece(NULL), first_turn(true)
	{
		board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
		
		// todo: choice
		std::string name_1 = "Some player 1";
		std::string name_2 = "Some player 2";
		player_1 = new Player('W', name_1);
		player_2 = new Player('B', name_2);

		// todo: change to algorithm
		// rows of the second player (upper)
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < size; ++j)
			{
				if ((i + j) % 2 != 0)
				{
					(*board)[j][i] = new Piece(player_2->get_sign(), j, i);
					p_list_2.push_back((*board)[j][i]);
					player_2->add_piece();
				}
			}
		}
		// rows of the first player (lower)
		for (int i = size - 1; i >= size - 3; --i)
		{
			for (int j = 0; j < size; ++j)
			{
				if ((i + j) % 2 != 0)
				{
					(*board)[j][i] = new Piece(player_1->get_sign(), j, i);
					p_list_1.push_back((*board)[j][i]);
					player_1->add_piece();
				}
			}
		}

		// set the antialiasing
		settings.antialiasingLevel = 2.0;

		// evaluate available moves for the first player
		int dummy = 0;
		available_capture = evaluate(p_list_1, board, &dummy);

		std::cout << "List of pieces of first player" << std::endl;
		print_pieces(&p_list_1);

		std::cout << "List of pieces of second player" << std::endl;
		print_pieces(&p_list_2);
	}

	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<Piece*>>* board)
	{
		os << "\t  ";
		/*for (char a = 'a'; a < 'a' + Game::size; ++a) // colums as letters
			os << a << "   ";*/
		for (int i = 0; i < Game::size; ++i)
			os << i << "   ";
		os << std::endl << std::endl << std::endl;
		for (int i = 0; i < Game::size; ++i)
		{
			//os << Game::size - i << "\t| ";
			os << i << "\t| ";
			for (int j = 0; j < Game::size; ++j)
			{
				os << (*board)[j][i] << " | ";
			}
			os << std::endl << std::endl;
		}
		return os;
	}

	Game::~Game()
	{
		delete board;
		delete player_1;
		delete player_2;
	}

	void Game::switch_turn(void) { first_turn = !first_turn; }

	void Game::loop(void)
	{
		bool selected = false;
		//rotate_board();

		// main loop
		while (window.isOpen())
		{
			// 10 fps
			sf::sleep(sf::seconds(1.0f / 20));

			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();

				if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
				{
					if (!available_capture) // check for new kings made
					{

					}

					if (selected_piece != NULL) // choice after highlighting
					{
						int x = sf::Mouse::getPosition(window).x / square_size;
						int y = sf::Mouse::getPosition(window).y / square_size;
						
						// find corresponding piece
						bool is_found = false;
						AvailableMove* found_move = NULL;

						all_of(selected_piece->get_av_list()->begin(), selected_piece->get_av_list()->end(), [&x, &y, &is_found, &found_move](AvailableMove* a)
						{
							if (a->get_x() == x && a->get_y() == y)
							{
								std::cout << a->get_x() << " " << a->get_y() << std::endl;
								found_move = a;
								is_found = true;
								return false; // break
							}
							return true; // continue
						});
						if (!is_found)
						{
							selected = false;
							selected_piece = NULL;
						}
						else
						{
							// if available move is a capture
							//if (found_move->is_capture())
							//{
							//	// coords to delete
							//	int x_d = 0;
							//	int y_d = 0;
							//}
							
							if (found_move->is_capture())
							{
								// mark found capture
								AvailableCapture* found_capture = dynamic_cast<AvailableCapture*>(found_move);
								int x_d = found_capture->get_x_d();
								int y_d = found_capture->get_y_d();
								
								std::cout << "CONTROL" << std::endl;
								std::cout << "Coords to delete" << x_d << " " << y_d << std::endl;
								
								// push to the "to delete list" (delete later)
								to_delete_list.push_back(new Piece('x', x_d, y_d));
								(*board)[x_d][y_d]->set_captured();
								
								
								if (first_turn)
								{
									player_2->capture();
									
									player_1->set_combo(true);
									std::cout << "player's 1 combo" << std::endl;
								}
								else
								{
									player_1->capture();
									
									player_2->set_combo(true);
									std::cout << "player's 2 combo" << std::endl;
								}
							}
							//else // section to delete
							//{
							//	player_1->set_combo(false);
							//	player_2->set_combo(false);
							//	std::cout << "combo deleted" << std::endl;
							//}

							// move the piece (piece, which is moving -> both capture and normal move)
							(*board)[selected_piece->get_x()][selected_piece->get_y()] = NULL;
							selected_piece->set_x(x);
							selected_piece->set_y(y);
							(*board)[x][y] = selected_piece;
							selected_piece = NULL;
							selected = false;
							
							std::cout << "List of pieces of first player" << std::endl;
							print_pieces(&p_list_1);
							std::cout << "List of pieces of second player" << std::endl;
							print_pieces(&p_list_2);

							// add end of game *

							// switch turn, if no combo
							if (!player_1->get_combo() && !player_2->get_combo())
								switch_turn();
							else // section to test (fixes stuff)
							{
								clear_list(&p_list_1);
								clear_list(&p_list_2);
							}

							// add end of game *

							// evaluate opposite player
							int dummy = 0;
							if (first_turn)
							{
								// evaluate next move
								available_capture = evaluate(p_list_1, board, &dummy);

								// exit the combo, if no more captures
								if (player_1->get_combo() && !available_capture)
								{
									// delete opponent's pieces of multi capture, clear failed list of possible moves, cancel combo, evaluate again
									clear_to_delete_list(&to_delete_list, &p_list_2);

									clear_list(&p_list_1);
									player_1->set_combo(false);
									switch_turn();
									available_capture = evaluate_inv(p_list_2, board, &dummy);
								}
								else
									clear_list(&p_list_2);

								// check for empty evaluation?
							}
							else // second turn
							{
								// evaluate next move
								available_capture = evaluate_inv(p_list_2, board, &dummy);

								// exit the combo, if no more captures
								if (player_2->get_combo() && !available_capture)
								{
									// delete opponent's pieces of multi capture, clear failed list of possible moves, cancel combo, evaluate again
									clear_to_delete_list(&to_delete_list, &p_list_1);

									clear_list(&p_list_2);
									player_2->set_combo(false);
									switch_turn();
									available_capture = evaluate(p_list_1, board, &dummy);
								}
								else
									clear_list(&p_list_1);
							}
						}
					}
					else
						selected = !selected;
				}
			}

			window.clear();
			draw(window);

			// first choice, nothing is already highlighted
			if (selected)
			{
				selected_piece = NULL;

				int x = sf::Mouse::getPosition(window).x / square_size;
				int y = sf::Mouse::getPosition(window).y / square_size;

				// if the correspoding field contains a piece
				if ((*board)[x][y] != NULL)
				{
					std::cout << "x: " << x << "; y: " << y << "; piece: " << (*board)[x][y] << std::endl;

					// check if player owns this piece
					if (first_turn)
					{
						if ((*board)[x][y]->get_sign() == player_1->get_sign())
						{
							std::cout << ":)" << std::endl;
							bool found_capture = false;
							if (!(*board)[x][y]->get_av_list()->empty())
							{
								// find at least one move that is a capture
								all_of((*board)[x][y]->get_av_list()->begin(), (*board)[x][y]->get_av_list()->end(), [&found_capture](AvailableMove* a)
									{
										if (a->is_capture())
										{
											found_capture = true;
											return false;
										}
										return true;
									});
								for_each((*board)[x][y]->get_av_list()->begin(), (*board)[x][y]->get_av_list()->end(), [](AvailableMove* a) { std::cout << "available: x: " << a->get_x() << "; y: " << a->get_y() << std::endl; });
							}
							if ((found_capture && available_capture) || (!found_capture && !available_capture))
								selected_piece = (*board)[x][y];
						}
						else
							std::cout << ":(" << std::endl;
					}
					else
					{
						if ((*board)[x][y]->get_sign() == player_2->get_sign())
						{
							std::cout << ":)" << std::endl;
							bool found_capture = false;
							if (!(*board)[x][y]->get_av_list()->empty())
							{
								// find at least one move that is a capture
								all_of((*board)[x][y]->get_av_list()->begin(), (*board)[x][y]->get_av_list()->end(), [&found_capture](AvailableMove* a)
									{
										if (a->is_capture())
										{
											found_capture = true;
											return false;
										}
										return true;
									});
								for_each((*board)[x][y]->get_av_list()->begin(), (*board)[x][y]->get_av_list()->end(), [](AvailableMove* a) { std::cout << "available: x: " << a->get_x() << "; y: " << a->get_y() << std::endl; });
							}
							if ((found_capture && available_capture) || (!found_capture && !available_capture))
								selected_piece = (*board)[x][y];
						}
						else
							std::cout << ":(" << std::endl;
					}
				}
				else
				{
					std::cout << "x: " << x << "; y: " << y << std::endl;
					selected_piece = NULL;
				}
				selected = false;
			}

			//// cancel selection if the capture is possible and not selected - this block is replaced in evaluation
			//if (available_capture)
			//{
			//	bool found_capture = false;
			//	if (first_turn)
			//	{
			//		all_of(p_list_1.begin(), p_list_1.end(), [&found_capture](Piece* p)
			//			{
			//				bool tmp = true;
			//				all_of(p->get_av_list()->begin(), p->get_av_list()->end(), [&tmp, &found_capture](AvailableMove* a)
			//					{
			//						if (a->is_capture())
			//						{
			//							tmp = false;
			//							found_capture = true;
			//							return false;
			//						}
			//						return true;
			//					});
			//				return tmp;
			//			});
			//	}
			//	else
			//	{
			//		all_of(p_list_2.begin(), p_list_2.end(), [&found_capture](Piece* p)
			//			{
			//				bool tmp = true;
			//				all_of(p->get_av_list()->begin(), p->get_av_list()->end(), [&tmp, &found_capture](AvailableMove* a)
			//					{
			//						if (a->is_capture())
			//						{
			//							tmp = false;
			//							found_capture = true;
			//							return false;
			//						}
			//						return true;
			//					});
			//				return tmp;
			//			});
			//	}
			//	if (!found_capture)
			//		selected_piece = NULL;
			//}

			// highlight selected piece and its corresponding moves, when moves exist
			if (selected_piece != NULL)
			{
				if (!(selected_piece->get_av_list()->empty()))
				{
					highlight_selected(window, selected_piece->get_x(), selected_piece->get_y());
					for_each(selected_piece->get_av_list()->begin(), selected_piece->get_av_list()->end(), [this](AvailableMove* a) { highlight_available(window, a->get_x(), a->get_y()); });
				}
				else
					selected_piece = NULL;
			}

			// print alive pieces
			for (int i = 0; i < size; ++i)
				for (int j = 0; j < size; ++j)
					if ((*board)[i][j] != NULL)
						(*board)[i][j]->draw(window);

			// print pieces in multicapture
			for_each(to_delete_list.begin(), to_delete_list.end(), [this](Piece* p) { p->draw(window); });

			window.display();
		}


		//while (!is_finished)
		//{
		//	// move of the first player
		//	//system("cls");

		//	is_finished = player_1->move();
		//	std::cout << board << std::endl;
		//	
		//	// evaluate player 2
		//	if (is_finished) // break if finished
		//		break;

		//	// move of the second player
		//	//system("cls");
		//	is_finished = player_2->move();
		//	std::cout << board << std::endl;
		//	
		//	// evaluate player 1

		//	// tmp
		//	player_1->print_player();
		//	player_2->print_player();

		//	is_finished = true;
		//}
		////print_results();

		//std::cout << board << std::endl;

		//player_1->print_player();
		//player_2->print_player();

		//while (!p_list_1.empty())
		//{
		//	std::cout << "sign: " << p_list_1.front()->get_sign() << "; x: " << p_list_1.front()->get_x() << "; y: " << p_list_1.front()->get_y() << std::endl;
		//	while (!(p_list_1.front()->get_av_list()->empty()))
		//	{
		//		std::cout << "available move - x: " << p_list_1.front()->get_av_list()->front()->get_x() << "; y: " << p_list_1.front()->get_av_list()->front()->get_y() << std::endl;
		//		p_list_1.front()->get_av_list()->pop_front();
		//	}
		//	p_list_1.pop_front();
		//}

		//while (!p_list_2.empty())
		//{
		//	std::cout << "sign: " << p_list_2.front()->get_sign() << "; x: " << p_list_2.front()->get_x() << "; y: " << p_list_2.front()->get_y() << std::endl;
		//	p_list_2.pop_front();
		//}
	}

	std::vector<std::vector<Piece*>>* Game::get_board(void) { return board; }

	/*void Game::rotate_board(void)
	{
		is_rotated = !is_rotated;
		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < size - i; ++j)
			{
				Piece* tmp = (*board)[i][j];
				(*board)[i][j] = (*board)[size - 1 - i][size - 1 - j];
				(*board)[size - 1 - i][size - 1 - j] = tmp;

				if (i == j)
				{
					Piece* tmp = (*board)[i][size - 1 - i];
					(*board)[i][size - 1 - i] = (*board)[size - 1 - i][i];
					(*board)[size - 1 - i][i] = tmp;
				}
			}
		}
	}*/

	void Game::print_results(std::ostream& os)
	{
		if (!first_won && !second_won)
			std::cout << "Game wasn't finished" << std::endl;
		else if (first_won && second_won)
			std::cout << "Draw" << std::endl;
		else if (first_won)
			std::cout << "Player: \"" << player_1->get_name() << "\" won!" << std::endl;
		else
			std::cout << "Player: \"" << player_2->get_name() << "\" won!" << std::endl;
	}


	void Game::draw(sf::RenderWindow& window)
	{
		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(square_size, square_size));
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				tile.setPosition(sf::Vector2f(square_size * i, square_size * j));
				if ((i + j) % 2 == 0)
					tile.setFillColor(sf::Color(193, 173, 158, 255));
				else
					tile.setFillColor(sf::Color(133, 94, 66, 255));
				window.draw(tile);
			}
		}
	}

	void Game::highlight_selected(sf::RenderWindow& window, int x, int y)
	{
		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(square_size, square_size));
		tile.setFillColor(sf::Color(173, 134, 106, 255));
		tile.setPosition(sf::Vector2f(square_size * x, square_size * y));
		window.draw(tile);
	}

	void Game::highlight_available(sf::RenderWindow& window, int x, int y)
	{
		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(square_size, square_size));
		tile.setFillColor(sf::Color(103, 194, 106, 255));
		tile.setPosition(sf::Vector2f(square_size * x, square_size * y));
		window.draw(tile);
	}


	bool Game::evaluate(std::list<Piece*> list, std::vector<std::vector<Piece*>>* board_p, int* counter)
	{
		bool available_capture = false;
		for_each(list.begin(), list.end(), [this, &board_p, &list, &counter, &available_capture](Piece* p)
			{
				int x = p->get_x();
				int y = p->get_y();
				
				std::cout << "evaluating" << std::endl;
				std::cout << "x: " << x << "; y: " << y << std::endl;

				if ((*board_p)[x][y] != NULL)
					std::cout << (*board_p)[x][y] << std::endl;

				// captures
				bool possible_capture_top_left = false;
				bool possible_capture_top_right = false;
				bool possible_capture_bottow_left = false;
				bool possible_capture_bottom_right = false;

				// check possible captures in every direction
				// choose move with the most captures
				// add to possible moves

				// x ascending to the right  !
				// y ascendint to the bottom !

				// capture top right (0)
				if (x + 2 <= size - 1 && y - 2 >= 0 && (*board_p)[x + 1][y - 1] != NULL && (*board_p)[x + 1][y - 1]->get_sign() == player_2->get_sign() && !((*board_p)[x + 1][y - 1]->get_is_captured()) && (*board_p)[x + 2][y - 2] == NULL)
					possible_capture_top_right = true;

				// capture top left (1)
				if (x - 2 >= 0 && y - 2 >= 0 && (*board_p)[x - 1][y - 1] != NULL && (*board_p)[x - 1][y - 1]->get_sign() == player_2->get_sign() && !((*board_p)[x - 1][y - 1]->get_is_captured()) && (*board_p)[x - 2][y - 2] == NULL)
					possible_capture_top_left = true;

				// capture bottom right (2)
				if (x + 2 <= size - 1 && y + 2 <= size - 1 && (*board_p)[x + 1][y + 1] != NULL && (*board_p)[x + 1][y + 1]->get_sign() == player_2->get_sign() && !((*board_p)[x + 1][y + 1]->get_is_captured()) && (*board_p)[x + 2][y + 2] == NULL)
					possible_capture_bottom_right = true;

				// capture bottom left (3)
				if (x - 2 >= 0 && y + 2 <= size - 1 && (*board_p)[x - 1][y + 1] != NULL && (*board_p)[x - 1][y + 1]->get_sign() == player_2->get_sign() && !((*board_p)[x - 1][y + 1]->get_is_captured()) && (*board_p)[x - 2][y + 2] == NULL)
					possible_capture_bottow_left = true;


				if (possible_capture_top_left || possible_capture_top_right || possible_capture_bottow_left || possible_capture_bottom_right)
				{
					available_capture = true;

					// evaluate copy of the board recursively in every direction and find highest number of captures to add to base moves list
					int capture_counter[4] = { 0, 0, 0, 0 }; // 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left

					if (possible_capture_top_right)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y - 2, x + 1, y - 1));
						capture_counter[0] = 1; // change here to get from counter, then increment?

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());
							
						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x + 2);
						moving_piece->set_y(y - 2);
						(*copy_of_board)[x + 2][y - 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x + 1][y - 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;
						

						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[0] = moves;
							std::cout << "moves counter (top right): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					}

					if (possible_capture_top_left)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y - 2, x - 1, y - 1));
						capture_counter[1] = 1;

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x - 2);
						moving_piece->set_y(y - 2);
						(*copy_of_board)[x - 2][y - 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x - 1][y - 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[1] = moves;
							std::cout << "moves counter (top left): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					}

					if (possible_capture_bottom_right)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y + 2, x + 1, y + 1));
						capture_counter[2] = 1;

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x + 2);
						moving_piece->set_y(y + 2);
						(*copy_of_board)[x + 2][y + 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x + 1][y + 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[2] = moves;
							std::cout << "moves counter (bottom right): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					}

					if (possible_capture_bottow_left)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y + 2, x - 1, y + 1));
						capture_counter[3] = 1;

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x - 2);
						moving_piece->set_y(y + 2);
						(*copy_of_board)[x - 2][y + 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x - 1][y + 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[3] = moves;
							std::cout << "moves counter (bottom left): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					} // all recursive function made

					// tmp: write all counters

					//find max counter
					int max = capture_counter[0];
					for (int i = 1; i < 4; ++i)
						if (capture_counter[i] > max)
							max = capture_counter[i];
					std::cout << "found max counter: " << max << std::endl;

					// if counter == max push back available capture
					for (int i = 0; i < 4; ++i)
						if (capture_counter[i] == max)
						{
							switch (i)
							{
							case 0:
							{
								std::cout << "kierunek top right: " << capture_counter[0] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y - 2, x + 1, y - 1));
								break;
							}
							case 1:
							{
								std::cout << "kierunek top left: " << capture_counter[1] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y - 2, x - 1, y - 1));
								break;
							}
							case 2:
							{
								std::cout << "kierunek bottom right: " << capture_counter[2] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y + 2, x + 1, y + 1));
								break;
							}
							case 3:
							{
								std::cout << "kierunek bottom left: " << capture_counter[3] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y + 2, x - 1, y + 1));
								break;
							}
							default:
								throw std::exception("Eval. error: 199");
							}
						}
				}
				else
				{
					if (*counter != NULL)
						return;

					// moves to right
					if (x != size - 1 && y != 0)
					{
						if ((*board_p)[x + 1][y - 1] == NULL)
						{
							std::cout << "available move to the right!" << std::endl;
							(*p).get_av_list()->push_back(new AvailableMove(x + 1, y - 1));
						}
					}

					// moves to left
					if (x != 0 && y != 0)
					{
						if ((*board_p)[x - 1][y - 1] == NULL)
						{
							std::cout << "available move to the left!" << std::endl;
							(*p).get_av_list()->push_back(new AvailableMove(x - 1, y - 1));
						}
					}
				}

			});
			std::cout << "---available returning: ";
			available_capture ? (std::cout << "true") : (std::cout << "false");
			std::cout << std::endl;
			return available_capture;
	}

	bool Game::evaluate_inv(std::list<Piece*> list, std::vector<std::vector<Piece*>>* board_p, int* counter)
	{
		bool available_capture = false;
		for_each(list.begin(), list.end(), [this, &board_p, &list, &counter, &available_capture](Piece* p)
			{
				int x = p->get_x();
				int y = p->get_y();

				std::cout << "evaluating" << std::endl;
				std::cout << "x: " << x << "; y: " << y << std::endl;

				if ((*board_p)[x][y] != NULL)
					std::cout << (*board_p)[x][y] << std::endl;

				// captures
				bool possible_capture_top_left = false;
				bool possible_capture_top_right = false;
				bool possible_capture_bottow_left = false;
				bool possible_capture_bottom_right = false;

				// check possible captures in every direction
				// choose move with the most captures
				// add to possible moves

				// x ascending to the right  !
				// y ascendint to the bottom !

				// capture top right (0)
				if (x + 2 <= size - 1 && y - 2 >= 0 && (*board_p)[x + 1][y - 1] != NULL && (*board_p)[x + 1][y - 1]->get_sign() == player_1->get_sign() && !((*board_p)[x + 1][y - 1]->get_is_captured()) && (*board_p)[x + 2][y - 2] == NULL)
					possible_capture_top_right = true;

				// capture top left (1)
				if (x - 2 >= 0 && y - 2 >= 0 && (*board_p)[x - 1][y - 1] != NULL && (*board_p)[x - 1][y - 1]->get_sign() == player_1->get_sign() && !((*board_p)[x - 1][y - 1]->get_is_captured()) && (*board_p)[x - 2][y - 2] == NULL)
					possible_capture_top_left = true;

				// capture bottom right (2)
				if (x + 2 <= size - 1 && y + 2 <= size - 1 && (*board_p)[x + 1][y + 1] != NULL && (*board_p)[x + 1][y + 1]->get_sign() == player_1->get_sign() && !((*board_p)[x + 1][y + 1]->get_is_captured()) && (*board_p)[x + 2][y + 2] == NULL)
					possible_capture_bottom_right = true;

				// capture bottom left (3)
				if (x - 2 >= 0 && y + 2 <= size - 1 && (*board_p)[x - 1][y + 1] != NULL && (*board_p)[x - 1][y + 1]->get_sign() == player_1->get_sign() && !((*board_p)[x - 1][y + 1]->get_is_captured()) && (*board_p)[x - 2][y + 2] == NULL)
					possible_capture_bottow_left = true;


				if (possible_capture_top_left || possible_capture_top_right || possible_capture_bottow_left || possible_capture_bottom_right)
				{
					available_capture = true;

					// evaluate copy of the board recursively in every direction and find highest number of captures to add to base moves list
					int capture_counter[4] = { 0, 0, 0, 0 }; // 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left

					if (possible_capture_top_right)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y - 2, x + 1, y - 1));
						capture_counter[0] = 1; // change here to get from counter, then increment?

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x + 2);
						moving_piece->set_y(y - 2);
						(*copy_of_board)[x + 2][y - 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x + 1][y - 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[0] = moves;
							std::cout << "moves counter (top right): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					}

					if (possible_capture_top_left)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y - 2, x - 1, y - 1));
						capture_counter[1] = 1;

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x - 2);
						moving_piece->set_y(y - 2);
						(*copy_of_board)[x - 2][y - 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x - 1][y - 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[1] = moves;
							std::cout << "moves counter (top left): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					}

					if (possible_capture_bottom_right)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y + 2, x + 1, y + 1));
						capture_counter[2] = 1;

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x + 2);
						moving_piece->set_y(y + 2);
						(*copy_of_board)[x + 2][y + 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x + 1][y + 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[2] = moves;
							std::cout << "moves counter (bottom right): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					}

					if (possible_capture_bottow_left)
					{
						//(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y + 2, x - 1, y + 1));
						capture_counter[3] = 1;

						// copy the board and make empty list for moved piece
						std::vector<std::vector<Piece*>>* copy_of_board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
						std::list<Piece*> copy_of_list;
						if (*counter == NULL) // first call of evaluation
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y());

						}
						else // the function call is being recursive -*-*-*-------------------------*-*-*- NEW -*-*-*-----------------------------------*-*-*-
						{
							for (int i = 0; i < size; ++i)
								for (int j = 0; j < size; ++j)
									if ((*board_p)[i][j] != NULL)
										(*copy_of_board)[i][j] = new Piece((*board_p)[i][j]->get_sign(), (*board_p)[i][j]->get_x(), (*board_p)[i][j]->get_y());
						}

						// make planned move
						Piece* moving_piece = (*copy_of_board)[x][y];
						(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
						moving_piece->set_x(x - 2);
						moving_piece->set_y(y + 2);
						(*copy_of_board)[x - 2][y + 2] = moving_piece;
						copy_of_list.push_back(moving_piece);
						(*copy_of_board)[x - 1][y + 1] = NULL;
						moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
						std::cout << copy_of_board << std::endl;


						//evaluate recursively - separate in every direction - call tree
						if (*counter == NULL)
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter null" << std::endl;
							int moves = 1;
							evaluate(copy_of_list, copy_of_board, &moves);
							capture_counter[3] = moves;
							std::cout << "moves counter (bottom left): " << moves << std::endl;
						}
						else
						{
							std::cout << "---------------------------------------------------------------" << std::endl;
							std::cout << "counter not null" << std::endl;
							(*counter)++;
							evaluate(copy_of_list, copy_of_board, counter);
						}
					} // all recursive function made

					// tmp: write all counters

					//find max counter
					int max = capture_counter[0];
					for (int i = 1; i < 4; ++i)
						if (capture_counter[i] > max)
							max = capture_counter[i];
					std::cout << "found max counter: " << max << std::endl;

					// if counter == max push back available capture
					for (int i = 0; i < 4; ++i)
						if (capture_counter[i] == max)
						{
							switch (i)
							{
							case 0:
							{
								std::cout << "kierunek top right: " << capture_counter[0] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y - 2, x + 1, y - 1));
								break;
							}
							case 1:
							{
								std::cout << "kierunek top left: " << capture_counter[1] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y - 2, x - 1, y - 1));
								break;
							}
							case 2:
							{
								std::cout << "kierunek bottom right: " << capture_counter[2] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x + 2, y + 2, x + 1, y + 1));
								break;
							}
							case 3:
							{
								std::cout << "kierunek bottom left: " << capture_counter[3] << std::endl;
								(*p).get_av_list()->push_back(new AvailableCapture(x - 2, y + 2, x - 1, y + 1));
								break;
							}
							default:
								throw std::exception("Eval. error: 199");
							}
						}
				}
				else // normal move, not a capture
				{
					if (*counter != NULL)
						return;

					// moves to right
					if (x != size - 1 && y != size - 1)
					{
						if ((*board_p)[x + 1][y + 1] == NULL)
						{
							std::cout << "available move to the right!" << std::endl;
							(*p).get_av_list()->push_back(new AvailableMove(x + 1, y + 1));
						}
					}

					// moves to left
					if (x != 0 && y != size - 1)
					{
						if ((*board_p)[x - 1][y + 1] == NULL)
						{
							std::cout << "available move to the left!" << std::endl;
							(*p).get_av_list()->push_back(new AvailableMove(x - 1, y + 1));
						}
					}
				}

			});
		std::cout << "---available returning: ";
		available_capture ? (std::cout << "true") : (std::cout << "false");
		std::cout << std::endl;
		return available_capture;
	}

	//// its outdated
	//bool Game::evaluate_inv(std::list<Piece*> list, std::vector<std::vector<Piece*>>* board_p, int* counter)
	//{
	//	for_each(list.begin(), list.end(), [this, &board_p](Piece* p)
	//		{
	//			int x = p->get_x();
	//			int y = p->get_y();

	//			std::cout << "evaluating" << std::endl;
	//			std::cout << "x: " << x << "; y: " << y << std::endl;

	//			if ((*board_p)[x][y] != NULL)
	//				std::cout << (*board_p)[x][y] << std::endl;

	//			// captures
	//			bool possible_capture = false;

	//			// capture

	//			if (!possible_capture)
	//			{
	//				// if any piece is on last row, change to king

	//				// moves to right
	//				if (x != size - 1 && y != size - 1)
	//				{
	//					if ((*board_p)[x + 1][y + 1] == NULL)
	//					{
	//						std::cout << "available move to the right!" << std::endl;
	//						(*p).get_av_list()->push_back(new AvailableMove(x + 1, y + 1));
	//					}
	//				}

	//				// moves to left
	//				if (x != 0 && y != size - 1)
	//				{
	//					if ((*board_p)[x - 1][y + 1] == NULL)
	//					{
	//						std::cout << "available move to the left!" << std::endl;
	//						(*p).get_av_list()->push_back(new AvailableMove(x - 1, y + 1));
	//					}
	//				}
	//			}0,,,
	//		});
	//	return false;
	//}
	//void Game::clear_list(std::list<Piece*>* list) { for_each(list->begin(), list->end(), [this](Piece* p) { while (!(p->get_av_list()->empty())) { p->get_av_list()->pop_front(); } }); }

	void Game::clear_list(std::list<Piece*>* list) { for_each(list->begin(), list->end(), [this](Piece* p) { p->get_av_list()->clear(); }); }
	
	void Game::print_pieces(std::list<Piece*>* list, std::ostream& os) { std::for_each(list->begin(), list->end(), [i = 1, this, &os](Piece* p) mutable { os << i++ << "; sign: " << p << "; x: " << p->get_x() << "; y: " << p->get_y() << std::endl; }); }

	void Game::delete_from_list(std::list<Piece*>* list, Piece* piece_to_delete) { list->remove(piece_to_delete); }

	void Game::clear_to_delete_list(std::list<Piece*>* del_list, std::list<Piece*>* src_list)
	{
		while (!(del_list->empty()))
		{
			// temporary piece from "to delete list"
			Piece* tmp = del_list->front();

			int x_d = tmp->get_x();
			int y_d = tmp->get_y();
			Piece* piece_to_delete = (*board)[x_d][y_d];
			(*board)[x_d][y_d] = NULL;

			delete_from_list(src_list, piece_to_delete);

			del_list->pop_front();
			//delete tmp;
		}
	}
}



/*
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

class Piece
{
	int x;
public:
	int get_x(void) { return x; }
	int set_x(int a) { return x = a; }
	Piece() : x(5) {}
	Piece(int a) : x(a) {}
	friend ostream& operator<<(ostream& os, const Piece& p);
};
ostream& operator<<(ostream& os, const Piece& p) { return os << p.x; }

int main(int argc, char* argv[])
{
	vector<vector<Piece*>>* board;
	board = new vector<vector<Piece*>>(10, vector<Piece*>(10, NULL));


	(*board)[1][1] = new Piece(4);
	(*board)[2][2] = new Piece(6);
	list<Piece*> lista;
	lista.push_back((*board)[1][1]);
	lista.back()->set_x(7);


	for_each(board->begin(), board->end(), [](vector<Piece*> v) { for_each(v.begin(), v.end(), [](Piece* p)
		{
			if (p == NULL)
				std::cout << "0; ";
			else
				std::cout << *p << "; ";
		}); std::cout << endl; });

	return 0;
}
*/