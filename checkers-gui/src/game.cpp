#include "include/game.h"
#include "include/king.h"

// todo:
// remake code into more functions, current player and opponent pointers
// add menu
// optimise drawing
// add animation
// add setting rows and check if set correctly
// switch to polymorphism completely (get rid of is_king flag)
// consider current moving piece to eliminate situation where two pieces have possible captures
// move some methods into private

namespace checkers
{
	game::game(int fps, std::istream& is, std::ostream& os) : m_console_game(false), m_is_finished(false), m_fps(fps),
		m_window(sf::VideoMode(s_square_size* s_size, s_square_size* s_size), "Checkers", sf::Style::Default, m_settings),
		m_selected_piece(NULL), m_moving_piece(NULL), m_available_capture(false), m_last_capture_direction(-1), m_is(is), m_os(os),
		m_tile(), m_clock(), m_event(), m_settings(), m_current_player(NULL)
	{
		assert(s_size % 2 == 0);
		
		//// create a font object
		//sf::Font font;
		//if (!font.loadFromFile("arial.ttf"))
		//	throw std::runtime_error("Font loading failure");
		//
		//// welcome text
		//sf::Text text;
		//text.setFont(font);
		//text.setCharacterSize(24);
		//text.setFillColor(sf::Color::White);
		//text.setPosition(50, 50);
		//text.setString("Checkers");

		//m_window.clear();
		//m_window.draw(text);
		//m_window.display();

		//// simplified: in-console choice
		//int choice = 0;
		//std::cout << "0 - player vs player" << std::endl;
		//std::cout << "any - player vs bot" << std::endl;
		//std::cout << "choose: ";
		//std::cin >> choice;

		//auto get_coords = std::bind(&game::get_click_coordinates, this);

		//if (choice == 0)
		//{
		//	m_player_1 = new player('W', "Player1", get_coords);
		//	m_player_2 = new player('B', "Player2", get_coords);
		//}
		//else
		//{
		//	m_player_1 = new player('W', "Player1", get_coords);
		//	m_player_2 = new bot('B', this);
		//}

		// simplified: set values
		if (!m_console_game)
		{
			auto get_coords = std::bind(&game::get_click_coordinates, this);
			m_player_1 = new player('W', "Player1", get_coords);
			m_player_2 = new player('B', "Player2", get_coords);
		}
		else
		{
			auto get_coords = std::bind(&game::get_coordinates_from_stream, this);
			m_player_1 = new player('W', "Player1", get_coords);
			m_player_2 = new player('B', "Player2", get_coords);
		}
		//m_player_2 = new bot('B', this);

		// set play order and evaluation direction
		m_player_1->set_first(true);
		m_player_2->set_first(false);
		m_player_1->set_next_player(m_player_2);
		m_player_2->set_next_player(m_player_1);
		m_current_player = m_player_1;
		
		// board init
		m_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));

		// fill the board with pieces
#ifdef _DEBUG
		//populate_board(3);
		populate_board_debug();
#else
		populate_board(3);
#endif

		// set pointers to piece lists
		m_player_1->set_list(&m_p_list_1);
		m_player_2->set_list(&m_p_list_2);

		// SFML setup, using original methods in camelCase style instead of snake_case
		m_settings.antialiasingLevel = 8;
		m_window.setFramerateLimit(fps);
		m_window.setVerticalSyncEnabled(true);
		m_tile.setSize(sf::Vector2f(s_square_size, s_square_size));

		// evaluate available moves for the first player
		int dummy = 0;
		m_available_capture = evaluate(&m_p_list_1, m_board, &dummy, m_player_1);
#ifdef _DEBUG
		std::cout << "Game is evaluated" << std::endl;

		std::cout << "List of pieces of first player" << std::endl;
		print_pieces(&m_p_list_1);

		std::cout << "List of pieces of second player" << std::endl;
		print_pieces(&m_p_list_2);
#endif
		m_window.clear();
	}

	game::game(const game& game) : m_fps(game.m_fps)
	{
		// copy player 1
		if (dynamic_cast<player*>(game.m_player_1))
			m_player_1 = new player(*dynamic_cast<player*>(game.m_player_1));
		else if (dynamic_cast<bot*>(game.m_player_1))
			m_player_1 = new bot(*dynamic_cast<bot*>(game.m_player_1));
		else
			m_player_1 = NULL;

		// copy player 2
		if (dynamic_cast<player*>(game.m_player_2))
			m_player_2 = new player(*dynamic_cast<player*>(game.m_player_2));
		else if (dynamic_cast<bot*>(game.m_player_2))
			m_player_2 = new bot(*dynamic_cast<bot*>(game.m_player_2));
		else
			m_player_2 = NULL;

		assert(m_player_1 != NULL);
		assert(m_player_2 != NULL);

		// establish current player
		m_current_player = game.m_current_player == game.m_player_1 ? m_player_1 : m_player_2;

		// set the next player fields
		m_player_1->set_next_player(m_player_2);
		m_player_2->set_next_player(m_player_1);

		// copy the board and recreate the lists
		m_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, NULL));
		std::vector<std::vector<piece*>>* original_board = game.m_board;
		
		for (int i = 0; i < s_size; ++i)
			for (int j = 0; j < s_size; ++j)
			{
				if ((*original_board)[i][j] != NULL)
				{
					piece* p = (*original_board)[i][j];
					if (dynamic_cast<king*>(p))
					{
						if (p->get_owner() == game.m_player_1)
							add_new_piece(&m_p_list_1, m_board, m_player_1, p);
						else if (p->get_owner() == game.m_player_2)
							add_new_piece(&m_p_list_2, m_board, m_player_2, p);
						else
							throw std::runtime_error("Copying the board: king piece: player signs not matching");
					}
					else
					{
						if (p->get_owner() == game.m_player_1)
							add_new_piece(&m_p_list_1, m_board, m_player_1, p);
						else if (p->get_owner() == game.m_player_2)
							add_new_piece(&m_p_list_2, m_board, m_player_2, p);
						else
							throw std::runtime_error("Copying the board: normal piece: player signs not matching");
					}
					
				}
			}
#ifdef _DEBUG
		std::cout << "Board after copying: " << std::endl;
		std::cout << m_board << std::endl;

		std::cout << "List of pieces of first player" << std::endl;
		print_pieces(&m_p_list_1);

		std::cout << "List of pieces of second player" << std::endl;
		print_pieces(&m_p_list_2);
#endif
		m_player_1->set_list(&m_p_list_1);
		m_player_2->set_list(&m_p_list_2);

		// recreate to delete list
		for_each(game.m_to_delete_list.begin(), game.m_to_delete_list.end(), [this](piece* p)
			{
				if (dynamic_cast<king*>(p))
					m_to_delete_list.push_back(new king(*dynamic_cast<king*>(p)));
				else
					m_to_delete_list.push_back(new piece(*p));
			});

		// evaluate available moves for the current player
		int dummy = 0;
		m_available_capture = evaluate(m_current_player->get_list(), m_board, &dummy, m_current_player);
#ifdef _DEBUG
		std::cout << "Game copy is evaluated" << std::endl;
#endif
		// copy the rest
		m_selected_piece = NULL;
		m_moving_piece = NULL;
		m_console_game = game.m_console_game;
		m_is_finished = game.m_is_finished;
		m_first_won = game.m_first_won;
		m_second_won = game.m_second_won;
		m_last_capture_direction = game.m_last_capture_direction;
		m_frame_duration = game.m_frame_duration;
		/*m_is = game.m_is;
		m_os = game.m_os;
		m_tile = game.m_tile;
		m_clock = game.m_clock;
		m_settings = game.m_settings;
		m_window = game.m_window;
		m_event = game.m_event;*/
	}


	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board)
	{
		os << "\t  ";
		for (char a = 'a'; a < 'a' + checkers::s_size; ++a) // colums as letters
			os << a << "   ";
		/*for (int i = 0; i < checkers::size; ++i)
			os << i << "   ";*/
		os << std::endl << std::endl << std::endl;
		for (int i = 0; i < checkers::s_size; ++i)
		{
			//os << game::size - i << "\t| ";
			os << i << "\t| ";
			for (int j = 0; j < s_size; ++j)
				os << (*board)[j][i] << " | ";
			os << std::endl << std::endl;
		}
		return os;
	}

	game::~game()
	{
		delete m_board;
		delete m_player_1;
		delete m_player_2;
	}

	void game::switch_turn(void)
	{
		m_current_player = m_current_player->get_next_player();
#ifdef _DEBUG
		std::cout << "Current player: " << m_current_player->get_name() << std::endl;
#endif 
	}

	void game::populate_board(int rows)
	{
		assert(m_board->size() == s_size);
		assert(rows <= s_size / 2);

		// todo: change to algorithm
		// rows of the second player (upper)
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < s_size; ++j)
				if ((i + j) % 2 != 0)
					add_new_piece(&m_p_list_2, m_board, m_player_2, j, i);

		// rows of the first player (lower)
		for (int i = s_size - 1; i >= s_size - 3; --i)
			for (int j = 0; j < s_size; ++j)
				if ((i + j) % 2 != 0)
					add_new_piece(&m_p_list_1, m_board, m_player_1, j, i);
	}

	void game::populate_board_debug(void)
	{
		assert(m_board->size() == s_size);

		// rows of the second player (upper)
		add_new_piece(&m_p_list_2, m_board, m_player_2, 1, 8);
		add_new_piece(&m_p_list_2, m_board, m_player_2, 2, 7);

		// rows of the first player (lower)
		int j = 9;
		for (int i = s_size - 1; i >= s_size - 8; --i)
		{
			if ((i + j) % 2 != 0)
				add_new_piece(&m_p_list_1, m_board, m_player_1, j, i);
			--j;
			if (j >= 6)
				j = 9;
		}

		//add_new_piece(&m_p_list_1, m_board, m_player_1, 7, 8);
		//add_new_piece(&m_p_list_1, m_board, m_player_1, 8, 7);
		//add_new_piece(&m_p_list_1, m_board, m_player_1, 6, 9);
		add_new_piece(&m_p_list_1, m_board, m_player_1, 1, 2);

		add_new_piece(&m_p_list_1, m_board, m_player_1, 5, 6);
		add_new_piece(&m_p_list_1, m_board, m_player_1, 5, 4);
		add_new_piece(&m_p_list_1, m_board, m_player_1, 5, 2);
		add_new_piece(&m_p_list_1, m_board, m_player_1, 7, 2);
		add_new_piece(&m_p_list_1, m_board, m_player_1, 7, 4);
	}

	void game::add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y)
	{
		assert((*board)[x][y] == NULL);
		(*board)[x][y] = new piece(player->get_sign(), x, y, player);
		list->push_back((*board)[x][y]);
		player->add_piece();
	}

	void game::add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, piece* based_on)
	{
		int x = based_on->get_x();
		int y = based_on->get_y();
		assert((*board)[x][y] == NULL);
		if (dynamic_cast<king*>(based_on))
			(*board)[x][y] = new king(player->get_sign(), x, y, player);
		else
			(*board)[x][y] = new piece(player->get_sign(), x, y, player);
		list->push_back((*board)[x][y]);
		player->add_piece();
	}


	std::vector<std::vector<piece*>>* game::get_board(void) { assert(m_board != NULL); return m_board; }

	std::tuple<int, int> game::get_coordinates(void) { return m_current_player->get_coordinates(); }

	std::tuple<int, int> game::get_click_coordinates(void)
	{
		int x = sf::Mouse::getPosition(m_window).x / (m_window.getSize().x / s_size);
		int y = sf::Mouse::getPosition(m_window).y / (m_window.getSize().y / s_size);

		return std::make_tuple(x, y);
	}

	std::tuple<int, int> game::get_coordinates_from_stream(void)
	{
		// change to string, add checks if it is a number etc.
		int x = 0;
		int y = 0;

		while (x < 1 || x > 10)
		{
			m_os << "Give the x coordinate: ";
			m_is >> x;
		}
		while (y < 1 || y > 10)
		{
			m_os << "Give the y coordinate: ";
			m_is >> y;
		}
		--x;
		--y;

		return std::make_tuple(x, y);
	}

	void game::print_results(std::ostream& os)
	{
		if (!m_first_won && !m_second_won)
			std::cout << "game wasn't finished" << std::endl;
		else if (m_first_won && m_second_won)
			std::cout << "Draw" << std::endl;
		else if (m_first_won)
			std::cout << "player: \"" << m_player_1->get_name() << "\" won!" << std::endl;
		else
			std::cout << "player: \"" << m_player_2->get_name() << "\" won!" << std::endl;
		std::cout << "player " << m_player_1->get_name() << "'s score: " << m_player_2->get_captured_pieces() << "; player " << m_player_2->get_name() << "'s score: " << m_player_1->get_captured_pieces() << std::endl;
	}

	void game::draw(sf::RenderWindow& window)
	{
		for (int i = 0; i < s_size; ++i)
		{
			for (int j = 0; j < s_size; ++j)
			{
				m_tile.setPosition(sf::Vector2f(s_square_size * i, s_square_size * j));
				if ((i + j) % 2 == 0)
					m_tile.setFillColor(sf::Color(193, 173, 158, 255));
				else
					m_tile.setFillColor(sf::Color(133, 94, 66, 255));
				window.draw(m_tile);
			}
		}
	}

	void game::highlight_selected(sf::RenderWindow& window, int x, int y)
	{
		sf::RectangleShape selected_tile;
		selected_tile.setSize(sf::Vector2f(s_square_size, s_square_size));
		selected_tile.setFillColor(sf::Color(173, 134, 106, 255));
		selected_tile.setPosition(sf::Vector2f(s_square_size * x, s_square_size * y));
		window.draw(selected_tile);
	}

	void game::highlight_available(sf::RenderWindow& window, int x, int y)
	{
		sf::RectangleShape available_tile;
		available_tile.setSize(sf::Vector2f(s_square_size, s_square_size));
		available_tile.setFillColor(sf::Color(103, 194, 106, 255));
		available_tile.setPosition(sf::Vector2f(s_square_size * x, s_square_size * y));
		window.draw(available_tile);
	}

	void game::handle_events(void)
	{

	}
	
	void game::draw_board(void)
	{

	}
	
	void game::select_piece(void)
	{

	}
	
	void game::make_move(void)
	{

	}

	void game::check_game_completion(void)
	{

	}
		
	void game::loop(void)
	{
		bool selected = false;
		m_clock.restart();

		// main loop
		while (m_window.isOpen())
		{
			while (m_window.pollEvent(m_event))
			{
				if (m_event.type == sf::Event::Closed)
					m_window.close();

				if (m_event.type == sf::Event::MouseButtonPressed && m_event.mouseButton.button == sf::Mouse::Left || dynamic_cast<bot*>(m_current_player) || m_console_game)
				{
					if (m_selected_piece != NULL) // choice after highlighting
					{
						// getting coords of the click after highlighting selected piece, ignore clicks outside
						std::tuple<int, int> coordinates = get_coordinates();
						int x = std::get<0>(coordinates);
						int y = std::get<1>(coordinates);

						if (x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1)
							break;

						// find corresponding piece
						bool is_found = false;
						available_move* found_move = NULL;

						all_of(m_selected_piece->get_av_list()->begin(), m_selected_piece->get_av_list()->end(), [&x, &y, &is_found, &found_move](available_move* a)
							{
								// check if selected coords match any of possible moves
								if (a->get_x() == x && a->get_y() == y)
								{
#ifdef _DEBUG
									std::cout << a->get_x() << " " << a->get_y() << std::endl;
#endif		
									found_move = a;
									is_found = true;
									return false;
								}
								return true;
							});
						if (!is_found) // deselection when wrong coords given
						{
							selected = false;
							m_selected_piece = NULL;
						}
						else // making a move
						{
							if (dynamic_cast<available_capture*>(found_move))
							{
								// mark found capture
								available_capture* found_capture = dynamic_cast<available_capture*>(found_move);
								int x_d = found_capture->get_x_d();
								int y_d = found_capture->get_y_d();

								// save capture direction: 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left
								if (x_d > x && y_d < y)
									m_last_capture_direction = 3;
								else if (x_d < x && y_d < y)
									m_last_capture_direction = 2;
								else if (x_d > x && y_d > y)
									m_last_capture_direction = 1;
								else if (x_d < x && y_d > y)
									m_last_capture_direction = 0;
								else
									throw std::runtime_error("Capturing in wrong direction");
#ifdef _DEBUG
								m_os << "/ saved last capture direction: ";
								switch (m_last_capture_direction)
								{
								case 0:
									m_os << "top right";
									break;
								case 1:
									m_os << "top left";
									break;
								case 2:
									m_os << "bottom right";
									break;
								case 3:
									m_os << "bottom left";
									break;
								}
								m_os << std::endl;

								std::cout << "CONTROL" << std::endl;
								std::cout << "Coords to delete" << x_d << " " << y_d << std::endl;
#endif
								// temporary: delete to debug
								piece* piece_to_delete = (*m_board)[x_d][y_d];
								(*m_board)[x_d][y_d] = NULL;

								m_current_player->get_next_player()->make_capture();
								delete_from_list(m_current_player->get_next_player()->get_list(), piece_to_delete);

								if (m_current_player->get_next_player()->get_list()->empty())
								{
									if (m_current_player->is_first())
										m_first_won = true;
									else
										m_second_won = true;
								}

								// create new piece which represents dead piece during multicapture, it is indifferent whether it was normal piece or king
								m_to_delete_list.push_back(new piece(std::tolower(m_current_player->get_next_player()->get_sign()), x_d, y_d, m_current_player->get_next_player()));
								m_current_player->set_combo(true);
#ifdef _DEBUG
								std::cout << m_current_player->get_name() << " combo" << std::endl;
#endif
							}

							// move the piece (piece, which is moving -> both capture and normal move), keep selected_piece pointer for the possible king
							(*m_board)[m_selected_piece->get_x()][m_selected_piece->get_y()] = NULL;
							m_selected_piece->set_x(x);
							m_selected_piece->set_y(y);
							(*m_board)[x][y] = m_selected_piece;
							selected = false;

#ifdef _DEBUG
							std::cout << "List of pieces of first player" << std::endl;
							print_pieces(&m_p_list_1);
							std::cout << "List of pieces of second player" << std::endl;
							print_pieces(&m_p_list_2);
#endif		

							// tmp flag indicating, that the king check was made this round
							bool made_king_check = false;

							// switch turn, if no combo
							if (!m_player_1->get_combo() && !m_player_2->get_combo())
							{
								// king function
								if (!made_king_check)
									m_current_player->change_to_king(m_selected_piece, m_board);
								made_king_check = true;
								switch_turn();
							}
							else // section to test (fixes stuff)
							{
								clear_list(&m_p_list_1);
								clear_list(&m_p_list_2);
							}

							// evaluate current player and check if there is more captures, if not, check for new kings
							int dummy = 0;
							m_available_capture = evaluate(m_current_player->get_list(), m_board, &dummy, m_current_player);
							std::cout << "dummy first: " << dummy << std::endl;

							// exit the combo, if no more captures
							if (m_current_player->get_combo() && !m_available_capture)
							{
								// delete opponent's pieces of multi capture, clear failed list of possible moves, cancel combo, evaluate again
								clear_to_delete_list(&m_to_delete_list, &m_p_list_1);
								clear_to_delete_list(&m_to_delete_list, &m_p_list_2);

								clear_list(&m_p_list_1);
								clear_list(&m_p_list_2);
								m_current_player->set_combo(false);
								m_current_player->get_next_player()->set_combo(false);
#ifdef _DEBUG
								std::cout << "Combo cancelled" << std::endl;
#endif
								// king function
								if (!made_king_check)
									m_current_player->change_to_king(m_selected_piece, m_board);
								made_king_check = true;
								switch_turn();
								m_available_capture = evaluate(m_current_player->get_list(), m_board, &dummy, m_current_player);
								std::cout << "dummy second: " << dummy << std::endl;
							}
							else // continue the combo
								clear_list(m_current_player->get_next_player()->get_list());

							m_selected_piece = NULL;

							// check for empty evaluation?
						}
					}
					else
						selected = !selected;
				}
			}

			// end of the game
			if (m_first_won || m_second_won)
			{
				std::cout << "Game is finished" << std::endl;
				break;
			}

			//m_window.clear();
			draw(m_window);

			// first choice, nothing is already highlighted
			if (selected)
			{
				m_selected_piece = NULL;

				// getting coords of the click after highlighting selected piece, ignore clicks outside
				std::tuple<int, int> coordinates = get_coordinates();
				int x = std::get<0>(coordinates);
				int y = std::get<1>(coordinates);

				if (x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1)
					break;

				// check if the correspoding field contains a piece
				if ((*m_board)[x][y] != NULL)
				{
#ifdef _DEBUG
					std::cout << "x: " << x << "; y: " << y << "; piece: " << (*m_board)[x][y] << std::endl;
#endif
					// check if player owns this piece
					if ((*m_board)[x][y]->get_sign() == m_current_player->get_sign())
					{
#ifdef _DEBUG
						std::cout << "That piece belongs to you" << std::endl;
#endif
						bool found_capture = false;
						if (!(*m_board)[x][y]->get_av_list()->empty())
						{
							// find at least one move that is a capture
							all_of((*m_board)[x][y]->get_av_list()->begin(), (*m_board)[x][y]->get_av_list()->end(), [&found_capture](available_move* a)
								{
									if (dynamic_cast<available_capture*>(a))
									{
										found_capture = true;
										return false;
									}
									return true;
								});
#ifdef _DEBUG
							for_each((*m_board)[x][y]->get_av_list()->begin(), (*m_board)[x][y]->get_av_list()->end(), [](available_move* a) { std::cout << "available: x: " << a->get_x() << "; y: " << a->get_y() << std::endl; });
#endif
						}
						if ((found_capture && m_available_capture) || (!found_capture && !m_available_capture)) // this lets making only capture moves, comment out to enable testing - replace to xnor
							m_selected_piece = (*m_board)[x][y];
					}
					else
					{
#ifdef _DEBUG
						std::cout << "That piece does not belong to you" << std::endl;
#endif
					}
				}
				else
				{
#ifdef _DEBUG
					std::cout << "x: " << x << "; y: " << y << std::endl;
#endif

					m_selected_piece = NULL;
				}
				selected = false;
			}

			// highlight selected piece and its corresponding moves, when moves exist
			if (m_selected_piece != NULL)
			{
				if (!(m_selected_piece->get_av_list()->empty()))
				{
					highlight_selected(m_window, m_selected_piece->get_x(), m_selected_piece->get_y());
					for_each(m_selected_piece->get_av_list()->begin(), m_selected_piece->get_av_list()->end(), [this](available_move* a) { highlight_available(m_window, a->get_x(), a->get_y()); });
				}
				else
					m_selected_piece = NULL;
			}

			// print alive pieces
			for (int i = 0; i < s_size; ++i)
				for (int j = 0; j < s_size; ++j)
					if ((*m_board)[i][j] != NULL)
						(*m_board)[i][j]->draw(m_window);

			// print pieces in multicapture
			for_each(m_to_delete_list.begin(), m_to_delete_list.end(), [this](piece* p) { p->draw(m_window); });

			// sleep time complementary to the frame time
			sf::Time elapsed_time = m_clock.restart();
			if (elapsed_time.asSeconds() < m_frame_duration)
				sf::sleep(sf::seconds(m_frame_duration - elapsed_time.asSeconds()));
			m_window.display();
		}
		print_results(m_os);
	}

	bool game::evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, base_player* player)
	{
		bool av_capture = false;

		// todo: list with each piece with a possible capture and corresponding score, after for_each add capture for only the higest score
		for_each(list->begin(), list->end(), [this, &board, &list, &counter, &av_capture, &player](piece* p)
			{
				// x coordinate of evaluated piece
				int x = p->get_x();
				// y coordinate of evaluated piece
				int y = p->get_y();

				// todo: cancel possible moves or captures if another type (piece or king) has higher score
				// or change to one common moves enabler
				if (!(dynamic_cast<king*>(p)))
				{
#ifdef _DEBUG
					std::cout << "evaluating normal piece" << std::endl;
					std::cout << "x: " << x << "; y: " << y << std::endl;

					if ((*board)[x][y] != NULL)
						std::cout << (*board)[x][y] << std::endl;
#endif
					// captures
					bool possible_capture_top_left = false;
					bool possible_capture_top_right = false;
					bool possible_capture_bottow_left = false;
					bool possible_capture_bottom_right = false;

					// capture top right (0)
					if (x + 2 <= s_size - 1 && y - 2 >= 0 && (*board)[x + 1][y - 1] != NULL && (*board)[x + 1][y - 1]->get_sign() == player->get_next_player()->get_sign() && (*board)[x + 2][y - 2] == NULL)
						possible_capture_top_right = true;

					// capture top left (1)
					if (x - 2 >= 0 && y - 2 >= 0 && (*board)[x - 1][y - 1] != NULL && (*board)[x - 1][y - 1]->get_sign() == player->get_next_player()->get_sign() && (*board)[x - 2][y - 2] == NULL)
						possible_capture_top_left = true;

					// capture bottom right (2)
					if (x + 2 <= s_size - 1 && y + 2 <= s_size - 1 && (*board)[x + 1][y + 1] != NULL && (*board)[x + 1][y + 1]->get_sign() == player->get_next_player()->get_sign() && (*board)[x + 2][y + 2] == NULL)
						possible_capture_bottom_right = true;

					// capture bottom left (3)
					if (x - 2 >= 0 && y + 2 <= s_size - 1 && (*board)[x - 1][y + 1] != NULL && (*board)[x - 1][y + 1]->get_sign() == player->get_next_player()->get_sign() && (*board)[x - 2][y + 2] == NULL)
						possible_capture_bottow_left = true;


					if (possible_capture_top_left || possible_capture_top_right || possible_capture_bottow_left || possible_capture_bottom_right)
					{
						av_capture = true;

						// evaluate copy of the board recursively in every direction and find highest number of captures to add to base moves list
						int capture_counter[4] = { 0, 0, 0, 0 }; // 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left

						if (possible_capture_top_right)
						{
							//(*p).get_av_list()->push_back(new available_capture(x + 2, y - 2, x + 1, y - 1));
							capture_counter[0] = 1; // change here to get from counter, then increment?

							// copy the board and make empty list for moved piece
							std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
							std::list<piece*> copy_of_list;
							
							for (int i = 0; i < s_size; ++i)
								for (int j = 0; j < s_size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);

							// make planned move
							piece* moving_piece = (*copy_of_board)[x][y];
							(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
							moving_piece->set_x(x + 2);
							moving_piece->set_y(y - 2);
							(*copy_of_board)[x + 2][y - 2] = moving_piece;
							copy_of_list.push_back(moving_piece);
							(*copy_of_board)[x + 1][y - 1] = NULL;
							moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
							std::cout << copy_of_board << std::endl;
#endif
							//evaluate recursively - separate in every direction - call tree
							if (*counter == NULL)
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter null" << std::endl;
#endif
								int moves = 1;
								evaluate(&copy_of_list, copy_of_board, &moves, player);
								capture_counter[0] = moves;
#ifdef _DEBUG
								std::cout << "moves counter (top right): " << moves << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter not null" << std::endl;
#endif
								(*counter)++;
								evaluate(&copy_of_list, copy_of_board, counter, player);
							}
						}

						if (possible_capture_top_left)
						{
							//(*p).get_av_list()->push_back(new available_capture(x - 2, y - 2, x - 1, y - 1));
							capture_counter[1] = 1;

							// copy the board and make empty list for moved piece
							std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
							std::list<piece*> copy_of_list;
							
							for (int i = 0; i < s_size; ++i)
								for (int j = 0; j < s_size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);

							// make planned move
							piece* moving_piece = (*copy_of_board)[x][y];
							(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
							moving_piece->set_x(x - 2);
							moving_piece->set_y(y - 2);
							(*copy_of_board)[x - 2][y - 2] = moving_piece;
							copy_of_list.push_back(moving_piece);
							(*copy_of_board)[x - 1][y - 1] = NULL;
							moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
							std::cout << copy_of_board << std::endl;
#endif

							//evaluate recursively - separate in every direction - call tree
							if (*counter == NULL)
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter null" << std::endl;
#endif

								int moves = 1;
								evaluate(&copy_of_list, copy_of_board, &moves, player);
								capture_counter[1] = moves;

#ifdef _DEBUG
								std::cout << "moves counter (top left): " << moves << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter not null" << std::endl;
#endif

								(*counter)++;
								evaluate(&copy_of_list, copy_of_board, counter, player);
							}
						}

						if (possible_capture_bottom_right)
						{
							//(*p).get_av_list()->push_back(new available_capture(x + 2, y + 2, x + 1, y + 1));
							capture_counter[2] = 1;

							// copy the board and make empty list for moved piece
							std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
							std::list<piece*> copy_of_list;
							
							for (int i = 0; i < s_size; ++i)
								for (int j = 0; j < s_size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);
							
							// make planned move
							piece* moving_piece = (*copy_of_board)[x][y];
							(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
							moving_piece->set_x(x + 2);
							moving_piece->set_y(y + 2);
							(*copy_of_board)[x + 2][y + 2] = moving_piece;
							copy_of_list.push_back(moving_piece);
							(*copy_of_board)[x + 1][y + 1] = NULL;
							moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
							std::cout << copy_of_board << std::endl;
#endif

							//evaluate recursively - separate in every direction - call tree
							if (*counter == NULL)
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter null" << std::endl;
#endif

								int moves = 1;
								evaluate(&copy_of_list, copy_of_board, &moves, player);
								capture_counter[2] = moves;

#ifdef _DEBUG	
								std::cout << "moves counter (bottom right): " << moves << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter not null" << std::endl;
#endif

								(*counter)++;
								evaluate(&copy_of_list, copy_of_board, counter, player);
							}
						}

						if (possible_capture_bottow_left)
						{
							//(*p).get_av_list()->push_back(new available_capture(x - 2, y + 2, x - 1, y + 1));
							capture_counter[3] = 1;

							// copy the board and make empty list for moved piece
							std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
							std::list<piece*> copy_of_list;
							
							for (int i = 0; i < s_size; ++i)
								for (int j = 0; j < s_size; ++j)
									if ((*board)[i][j] != NULL)
										(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);
							
							// make planned move
							piece* moving_piece = (*copy_of_board)[x][y];
							(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
							moving_piece->set_x(x - 2);
							moving_piece->set_y(y + 2);
							(*copy_of_board)[x - 2][y + 2] = moving_piece;
							copy_of_list.push_back(moving_piece);
							(*copy_of_board)[x - 1][y + 1] = NULL;
							moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
							std::cout << copy_of_board << std::endl;
#endif	

							//evaluate recursively - separate in every direction - call tree
							if (*counter == NULL)
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter null" << std::endl;
#endif

								int moves = 1;
								evaluate(&copy_of_list, copy_of_board, &moves, player);
								capture_counter[3] = moves;

#ifdef _DEBUG
								std::cout << "moves counter (bottom left): " << moves << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "---------------------------------------------------------------" << std::endl;
								std::cout << "counter not null" << std::endl;
#endif

								(*counter)++;
								evaluate(&copy_of_list, copy_of_board, counter, player);
							}
						} // all recursive function made

						// tmp: write all counters

						//find max counter
						int max = capture_counter[0];
						for (int i = 1; i < 4; ++i)
							if (capture_counter[i] > max)
								max = capture_counter[i];
#ifdef _DEBUG
						std::cout << "found max counter: " << max << std::endl;
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
									std::cout << "top right direction: " << capture_counter[0] << std::endl;
#endif
									(*p).get_av_list()->push_back(new available_capture(x + 2, y - 2, x + 1, y - 1));
									break;
								}
								case 1:
								{
#ifdef _DEBUG
									std::cout << "top left direction: " << capture_counter[1] << std::endl;
#endif
									(*p).get_av_list()->push_back(new available_capture(x - 2, y - 2, x - 1, y - 1));
									break;
								}
								case 2:
								{
#ifdef _DEBUG
									std::cout << "bottom right direction: " << capture_counter[2] << std::endl;
#endif
									(*p).get_av_list()->push_back(new available_capture(x + 2, y + 2, x + 1, y + 1));
									break;
								}
								case 3:
								{
#ifdef _DEBUG
									std::cout << "bottom left direction: " << capture_counter[3] << std::endl;
#endif
									(*p).get_av_list()->push_back(new available_capture(x - 2, y + 2, x - 1, y + 1));
									break;
								}
								default:
									throw std::exception("Evaluation error on piece " + p->get_sign());
								}
							}
					}
					else
					{
						if (*counter != NULL)
							return;

						// different direction of ordinary move
						if (player->is_first())
						{
							// moves to right
							if (x != s_size - 1 && y != 0)
							{
								if ((*board)[x + 1][y - 1] == NULL)
								{
#ifdef _DEBUG
									std::cout << "available move to the right!" << std::endl;
#endif
									(*p).get_av_list()->push_back(new available_move(x + 1, y - 1));
								}
							}

							// moves to left
							if (x != 0 && y != 0)
							{
								if ((*board)[x - 1][y - 1] == NULL)
								{
#ifdef _DEBUG
									std::cout << "available move to the left!" << std::endl;
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
								if ((*board)[x + 1][y + 1] == NULL)
								{
#ifdef _DEBUG
									std::cout << "available move to the right!" << std::endl;
#endif		
									(*p).get_av_list()->push_back(new available_move(x + 1, y + 1));
								}
							}

							// moves to left
							if (x != 0 && y != s_size - 1)
							{
								if ((*board)[x - 1][y + 1] == NULL)
								{
#ifdef _DEBUG
									std::cout << "available move to the left!" << std::endl;
#endif
									(*p).get_av_list()->push_back(new available_move(x - 1, y + 1));
								}
							}
						}
					}
				}
				else // king piece
				{
#ifdef _DEBUG
					std::cout << "evaluating the king" << std::endl;
					std::cout << "x: " << x << "; y: " << y << std::endl;

					if ((*board)[x][y] != NULL)
						std::cout << (*board)[x][y] << std::endl;
#endif
					// flags blocking opposite captures in multicapture
					bool possible_top_right = true;
					bool possible_top_left = true;
					bool possible_bottom_right = true;
					bool possible_bottom_left = true;

					bool multicapture = !m_to_delete_list.empty();

					if (multicapture) // cannot capture in one direction and then in opposite
					{
						switch (m_last_capture_direction)
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
							throw std::runtime_error("Wrong direction");
						}
					}
#ifdef _DEBUG
					if (!possible_top_right)
						m_os << "/ top right capture won't be available" << std::endl;
					if (!possible_top_left)
						m_os << "/ top left capture won't be available" << std::endl;
					if (!possible_bottom_right)
						m_os << "/ bottom right capture won't be available" << std::endl;
					if (!possible_bottom_left)
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

					// make temporary board with dead pieces
					//std::vector<std::vector<piece*>>* dead_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
					std::vector<std::vector<piece*>> dead_board(s_size, std::vector<piece*>(s_size, NULL));
					for_each(m_to_delete_list.begin(), m_to_delete_list.end(), [&dead_board, this, player](piece* p)
						{
							int x = p->get_x();
							int y = p->get_y();
							if (dead_board[x][y] == NULL)
								dead_board[x][y] = new piece(p->get_sign(), x, y, player->get_next_player());
							else
								throw std::runtime_error("Copying to delete list: wrong piece data");
						});
#ifdef _DEBUG
					m_os << "Dead board:" << std::endl;
					m_os << &dead_board << std::endl;
#endif

					// capture top right (0) + -
#ifdef _DEBUG
					std::cout << "top right checking" << std::endl;
#endif
					int i = 1;
					while (x + i + 1 <= s_size - 1 && y - i - 1 >= 0)
					{
#ifdef _DEBUG
						std::cout << "checking: x: " << x + i << ", y: " << y - i << " and place to go: x: " << x + i + 1 << ", y: " << y - i - 1 << std::endl;
#endif
						// searching for own piece (cannot jump across them)
						if ((*board)[x + i][y - i] != NULL && (*board)[x + i][y - i]->get_sign() == player->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found own piece and breaking" << std::endl;
#endif
							possible_capture_top_right.push_back(false);
							break;
						}
						// searching for dead piece (cannot jump across them)
						else if (dead_board[x + i][y - i] != NULL && dead_board[x + i][y - i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found dead piece and breaking" << std::endl;
#endif
							possible_capture_top_right.push_back(false);
							break;
						}
						else if ((*board)[x + i][y - i] != NULL && (*board)[x + i][y - i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found opponent's piece and checking for next fields" << std::endl;
#endif
							if ((*board)[x + i + 1][y - i - 1] == NULL && dead_board[x + i + 1][y - i - 1] == NULL)
							{
#ifdef _DEBUG
								std::cout << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
								possible_capture_top_right.push_back(true);
								at_least_one_capture_top_right = true;

								// adding all possible capture options
								int j = 0;
								while (x + i + 1 + j <= s_size - 1 && y - i - 1 - j >= 0 && (*board)[x + i + 1 + j][y - i - 1 - j] == NULL && dead_board[x + i + 1 + j][y - i - 1 - j] == NULL)
								{
#ifdef _DEBUG
									std::cout << "\%piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x + i + 1 + j << ", y: " << y - i - 1 - j << std::endl;
#endif
									local_captures_top_right.push_back(available_capture(x + i + 1 + j, y - i - 1 - j, x + i, y - i));
									++j;
								}
#ifdef _DEBUG
								std::cout << "found and added all possible capture options, continuing" << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*next piece is not empty and braking" << std::endl;
#endif
								possible_capture_top_right.push_back(false);
								break;
							}
						}
						else // empty field
						{
							if ((*board)[x + i + 1][y - i - 1] != NULL)
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
								if (at_least_one_capture_top_right)
								{
#ifdef _DEBUG
									std::cout << "*no more places to check, it is a singular check" << std::endl;
#endif
									break;
								}
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty" << std::endl;
#endif
							}
							local_moves_top_right.push_back(available_move(x + i, y - i));
							possible_capture_top_right.push_back(false);
						}
						
						++i;
					}
					if (x + i <= s_size - 1 && y - i >= 0 && (*board)[x + i][y - i] == NULL && !at_least_one_capture_top_right) // last, not checked field (checking looks for next which is outside the boundaries)
					{
#ifdef _DEBUG
						std::cout << "last but not least" << std::endl;
#endif
						local_moves_top_right.push_back(available_move(x + i, y - i));
					}

					// capture top left (1) - -
#ifdef _DEBUG
					std::cout << "top left checking" << std::endl;
#endif
					i = 1;
					while (x - i - 1 >= 0 && y - i - 1 >= 0)
					{
#ifdef _DEBUG
						std::cout << "checking: x: " << x - i << ", y: " << y - i << " and place to go: x: " << x - i - 1 << ", y: " << y - i - 1 << std::endl;
#endif
						// searching for own piece (cannot jump across them)
						if ((*board)[x - i][y - i] != NULL && (*board)[x - i][y - i]->get_sign() == player->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found own piece and breaking" << std::endl;
#endif
							possible_capture_top_left.push_back(false);
							break;
						}
						// searching for dead piece (cannot jump across them)
						else if (dead_board[x - i][y - i] != NULL && dead_board[x - i][y - i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found dead piece and breaking" << std::endl;
#endif
							possible_capture_top_right.push_back(false);
							break;
						}
						else if ((*board)[x - i][y - i] != NULL && (*board)[x - i][y - i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found opponent's piece and checking for next fields" << std::endl;
#endif
							if ((*board)[x - i - 1][y - i - 1] == NULL && dead_board[x - i - 1][y - i - 1] == NULL)
							{
#ifdef _DEBUG
								std::cout << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
								possible_capture_top_left.push_back(true);
								at_least_one_capture_top_left = true;

								// adding all possible capture options
								int j = 0;
								while (x - i - 1 - j >= 0 && y - i - 1 - j >= 0 && (*board)[x - i - 1 - j][y - i - 1 - j] == NULL && dead_board[x - i - 1 - j][y - i - 1 - j] == NULL)
								{
#ifdef _DEBUG
									std::cout << "\%piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x - i - 1 - j << ", y: " << y - i - 1 - j << std::endl;
#endif
									local_captures_top_left.push_back(available_capture(x - i - 1 - j, y - i - 1 - j, x - i, y - i));
									++j;
								}
#ifdef _DEBUG
								std::cout << "found and added all possible capture options, continuing" << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*next piece is not empty and braking" << std::endl;
#endif
								possible_capture_top_left.push_back(false);
								break;
							}
						}
						else // empty field
						{
							if ((*board)[x - i - 1][y - i - 1] != NULL)
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
								if (at_least_one_capture_top_right)
								{
#ifdef _DEBUG
									std::cout << "*no more places to check, it is a singular check" << std::endl;
#endif
									break;
								}
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty" << std::endl;
#endif
							}
							local_moves_top_left.push_back(available_move(x - i, y - i));
							possible_capture_top_left.push_back(false);
						}
						++i;
					}
					if (x - i >= 0 && y - i >= 0 && (*board)[x - i][y - i] == NULL && !at_least_one_capture_top_left) // last, not checked field (checking looks for next which is outside the boundaries)
					{
#ifdef _DEBUG
						std::cout << "last but not least" << std::endl;
#endif
						local_moves_top_left.push_back(available_move(x - i, y - i));
					}


					// capture bottom right (2) + +
#ifdef _DEBUG
					std::cout << "bottom right checking" << std::endl;
#endif
					i = 1;
					while (x + i + 1 <= s_size - 1 && y + i + 1 <= s_size - 1)
					{
#ifdef _DEBUG
						std::cout << "checking: x: " << x + i << ", y: " << y + i << " and place to go: x: " << x + i + 1 << ", y: " << y + i + 1 << std::endl;
#endif
						// searching for own piece (cannot jump across them)
						if ((*board)[x + i][y + i] != NULL && (*board)[x + i][y + i]->get_sign() == player->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found own piece and breaking" << std::endl;
#endif
							possible_capture_bottom_right.push_back(false);
							break;
						}
						// searching for dead piece (cannot jump across them)
						else if (dead_board[x + i][y + i] != NULL && dead_board[x + i][y + i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found dead piece and breaking" << std::endl;
#endif
							possible_capture_top_right.push_back(false);
							break;
						}
						else if ((*board)[x + i][y + i] != NULL && (*board)[x + i][y + i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found opponent's piece and checking for next fields" << std::endl;
#endif
							if ((*board)[x + i + 1][y + i + 1] == NULL && dead_board[x + i + 1][y + i + 1] == NULL)
							{
#ifdef _DEBUG
								std::cout << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
								possible_capture_bottom_right.push_back(true);
								at_least_one_capture_bottom_right = true;

								// adding all possible capture options
								int j = 0;
								while (x + i + 1 + j <= s_size - 1 && y + i + 1 + j <= s_size - 1 && (*board)[x + i + 1 + j][y + i + 1 + j] == NULL && dead_board[x + i + 1 + j][y + i + 1 + j] == NULL)
								{
#ifdef _DEBUG
									std::cout << "\%piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x + i + 1 + j << ", y: " << y + i + 1 + j << std::endl;
#endif
									local_captures_bottom_right.push_back(available_capture(x + i + 1 + j, y + i + 1 + j, x + i, y + i));
									++j;
								}
#ifdef _DEBUG
								std::cout << "found and added all possible capture options, continuing" << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*next piece is not empty and braking" << std::endl;
#endif
								possible_capture_bottom_right.push_back(false);
								break;
							}
						}
						else // empty field
						{
							if ((*board)[x + i + 1][y + i + 1] != NULL)
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
								if (at_least_one_capture_top_right)
								{
#ifdef _DEBUG
									std::cout << "*no more places to check, it is a singular check" << std::endl;
#endif
									break;
								}
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty" << std::endl;
#endif
							}
							local_moves_bottom_right.push_back(available_move(x + i, y + i));
							possible_capture_bottom_right.push_back(false);
						}

						++i;
					}
					if (x + i <= s_size - 1 && y + i <= s_size - 1 && (*board)[x + i][y + i] == NULL && !at_least_one_capture_bottom_right) // last, not checked field (checking looks for next which is outside the boundaries)
					{
#ifdef _DEBUG
						std::cout << "last but not least" << std::endl;
#endif
						local_moves_bottom_right.push_back(available_move(x + i, y + i));
					}

					// capture bottom left (3) - +
#ifdef _DEBUG
					std::cout << "bottom left checking" << std::endl;
#endif
					i = 1;
					while (x - i - 1 >= 0 && y + i + 1 <= s_size - 1)
					{
#ifdef _DEBUG
						std::cout << "checking: x: " << x - i << ", y: " << y + i << " and place to go: x: " << x - i - 1 << ", y: " << y + i + 1 << std::endl;
#endif
						// searching for own piece (cannot jump across them)
						if ((*board)[x - i][y + i] != NULL && (*board)[x - i][y + i]->get_sign() == player->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found own piece and breaking" << std::endl;
#endif
							possible_capture_bottom_left.push_back(false);
							break;
						}
						// searching for dead piece (cannot jump across them)
						else if (dead_board[x - i][y + i] != NULL && dead_board[x - i][y + i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found dead piece and breaking" << std::endl;
#endif
							possible_capture_top_right.push_back(false);
							break;
						}
						else if ((*board)[x - i][y + i] != NULL && (*board)[x - i][y + i]->get_sign() == player->get_next_player()->get_sign())
						{
#ifdef _DEBUG
							std::cout << "*found opponent's piece and checking for next fields" << std::endl;
#endif
							if ((*board)[x - i - 1][y + i + 1] == NULL && dead_board[x - i - 1][y + i + 1] == NULL)
							{
#ifdef _DEBUG
								std::cout << "*found opponent's piece that can be captured, searching for next" << std::endl;
#endif
								possible_capture_bottom_left.push_back(true);
								at_least_one_capture_bottom_left = true;

								// adding all possible capture options
								int j = 0;
								while (x - i - 1 - j >= 0 && y + i + 1 + j <= s_size - 1 && (*board)[x - i - 1 - j][y + i + 1 + j] == NULL && dead_board[x - i - 1 - j][y + i + 1 + j] == NULL)
								{
#ifdef _DEBUG
									std::cout << "\%piece at x: " << x << ", y: " << y << " can capture on coords: x: " << x - i - 1 - j << ", y: " << y + i + 1 + j << std::endl;
#endif
									local_captures_bottom_left.push_back(available_capture(x - i - 1 - j, y + i + 1 + j, x - i, y + i));
									++j;
								}
#ifdef _DEBUG
								std::cout << "found and added all possible capture options, continuing" << std::endl;
#endif
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*next piece is not empty and braking" << std::endl;
#endif
								possible_capture_bottom_left.push_back(false);
								break;
							}
						}
						else // empty field
						{
							if ((*board)[x - i - 1][y + i + 1] != NULL)
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty and next field is not empty" << std::endl;
#endif
								if (at_least_one_capture_top_right)
								{
#ifdef _DEBUG
									std::cout << "*no more places to check, it is a singular check" << std::endl;
#endif
									break;
								}
							}
							else
							{
#ifdef _DEBUG
								std::cout << "*there is no capture, checked field is empty" << std::endl;
#endif
							}
							local_moves_bottom_left.push_back(available_move(x - i, y + i));
							possible_capture_bottom_left.push_back(false);
						}

						++i;
					}
					if (x - i >= 0 && y + i <= s_size - 1 && (*board)[x - i][y + i] == NULL && !at_least_one_capture_bottom_left) // last, not checked field (checking looks for next which is outside the boundaries)
					{
#ifdef _DEBUG
						std::cout << "Added last possible field" << std::endl;
#endif
						local_moves_bottom_left.push_back(available_move(x - i, y + i));
					}
#ifdef _DEBUG
					// list top right moves and captures
					std::cout << "king top right moves" << std::endl;
					for_each(possible_capture_top_right.begin(), possible_capture_top_right.end(), [i = 1](bool b) mutable { std::cout << i++ << ": " << b << std::endl; });

					std::cout << "local moves" << std::endl;
					for_each(local_moves_top_right.begin(), local_moves_top_right.end(), [](available_move& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

					std::cout << "local captures" << std::endl;
					for_each(local_captures_top_right.begin(), local_captures_top_right.end(), [](available_capture& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });
					
					// list top left moves and captures
					std::cout << "king top left moves" << std::endl;
					for_each(possible_capture_top_left.begin(), possible_capture_top_left.end(), [i = 1](bool b) mutable { std::cout << i++ << ": " << b << std::endl; });

					std::cout << "local moves" << std::endl;
					for_each(local_moves_top_left.begin(), local_moves_top_left.end(), [](available_move& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

					std::cout << "local captures" << std::endl;
					for_each(local_captures_top_left.begin(), local_captures_top_left.end(), [](available_capture& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });

					// list bottom right moves and captures
					std::cout << "king bottom right moves" << std::endl;
					for_each(possible_capture_bottom_right.begin(), possible_capture_bottom_right.end(), [i = 1](bool b) mutable { std::cout << i++ << ": " << b << std::endl; });

					std::cout << "local moves" << std::endl;
					for_each(local_moves_bottom_right.begin(), local_moves_bottom_right.end(), [](available_move& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

					std::cout << "local captures" << std::endl;
					for_each(local_captures_bottom_right.begin(), local_captures_bottom_right.end(), [](available_capture& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });

					// list bottom left moves and captures
					std::cout << "king bottom left moves" << std::endl;
					for_each(possible_capture_bottom_left.begin(), possible_capture_bottom_left.end(), [i = 1](bool b) mutable { std::cout << i++ << ": " << b << std::endl; });

					std::cout << "local moves" << std::endl;
					for_each(local_moves_bottom_left.begin(), local_moves_bottom_left.end(), [](available_move& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << std::endl; });

					std::cout << "local captures" << std::endl;
					for_each(local_captures_bottom_left.begin(), local_captures_bottom_left.end(), [](available_capture& a) { std::cout << "-x: " << a.get_x() << "; y: " << a.get_y() << "; x_d: " << a.get_x_d() << "; y_d: " << a.get_y_d() << std::endl; });
#endif
					if (at_least_one_capture_top_right || at_least_one_capture_top_left || at_least_one_capture_bottom_right || at_least_one_capture_bottom_left)
					{
						av_capture = true;

						int top_right = local_captures_top_right.size();
						int top_left = local_captures_top_left.size();
						int bottom_right = local_captures_bottom_right.size();
						int bottom_left = local_captures_bottom_left.size();
						
						// for storing recursively evaluated capture counters
						std::vector<int> capture_counter[4] = { std::vector<int>(top_right), std::vector<int>(top_left), std::vector<int>(bottom_right), std::vector<int>(bottom_left) }; // 0 - top right, 1 - top left, 2 - bottom right, 3 - bottom left

						// top right recursive evaluation
						if (at_least_one_capture_top_right)
						{
							for_each(local_captures_top_right.begin(), local_captures_top_right.end(), [i = 0, this, &capture_counter, &counter, &board, &player, &x, &y](available_capture& a) mutable
								{
									capture_counter[0][i] = 1; // change here to get from counter, then increment?

									// copy coords
									int x_to_go = a.get_x();
									int y_to_go = a.get_y();
									int x_to_delete = a.get_x_d();
									int y_to_delete = a.get_y_d();

									// copy the board and make empty list for moved piece
									std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
									std::list<piece*> copy_of_list;
									
									for (int i = 0; i < s_size; ++i)
										for (int j = 0; j < s_size; ++j)
											if ((*board)[i][j] != NULL)
												(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);

									// make planned move
									piece* moving_piece = (*copy_of_board)[x][y];
									(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
									moving_piece->set_x(x_to_go);
									moving_piece->set_y(y_to_go);
									(*copy_of_board)[x_to_go][y_to_go] = moving_piece;
									copy_of_list.push_back(moving_piece);
									(*copy_of_board)[x_to_delete][y_to_delete] = NULL;
									moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
									std::cout << copy_of_board << std::endl;
#endif
									//evaluate recursively - separate in every direction - call tree
									if (*counter == NULL)
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter null" << std::endl;
#endif
										int moves = 1;
										evaluate(&copy_of_list, copy_of_board, &moves, player);
										capture_counter[0][i] = moves;
#ifdef _DEBUG
										std::cout << "moves counter (top right): " << moves << std::endl;
#endif
									}
									else
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter not null" << std::endl;
#endif
										(*counter)++;
										evaluate(&copy_of_list, copy_of_board, counter, player);
									}
									++i;
								});
#ifdef _DEBUG
							// print
							std::cout << "for every capture option, these are multi capture counters" << std::endl;
							for_each(capture_counter[0].begin(), capture_counter[0].end(), [i = 1](int c) mutable { std::cout << i++ << ": " << c << std::endl; });
#endif
						}

						// top left recursive evaluation
						if (at_least_one_capture_top_left)
						{
							for_each(local_captures_top_left.begin(), local_captures_top_left.end(), [i = 0, this, &capture_counter, &counter, &board, &player, &x, &y](available_capture& a) mutable
								{
									capture_counter[1][i] = 1; // change here to get from counter, then increment?

									// copy coords
									int x_to_go = a.get_x();
									int y_to_go = a.get_y();
									int x_to_delete = a.get_x_d();
									int y_to_delete = a.get_y_d();

									// copy the board and make empty list for moved piece
									std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
									std::list<piece*> copy_of_list;
									
									for (int i = 0; i < s_size; ++i)
										for (int j = 0; j < s_size; ++j)
											if ((*board)[i][j] != NULL)
												(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);
									
									// make planned move
									piece* moving_piece = (*copy_of_board)[x][y];
									(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
									moving_piece->set_x(x_to_go);
									moving_piece->set_y(y_to_go);
									(*copy_of_board)[x_to_go][y_to_go] = moving_piece;
									copy_of_list.push_back(moving_piece);
									(*copy_of_board)[x_to_delete][y_to_delete] = NULL;
									moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
									std::cout << copy_of_board << std::endl;
#endif
									//evaluate recursively - separate in every direction - call tree
									if (*counter == NULL)
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter null" << std::endl;
#endif
										int moves = 1;
										evaluate(&copy_of_list, copy_of_board, &moves, player);
										capture_counter[1][i] = moves;
#ifdef _DEBUG
										std::cout << "moves counter (top right): " << moves << std::endl;
#endif
									}
									else
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter not null" << std::endl;
#endif
										(*counter)++;
										evaluate(&copy_of_list, copy_of_board, counter, player);
									}
									++i;
								});

#ifdef _DEBUG
							// print
							std::cout << "for every capture option, these are multi capture counters" << std::endl;
							for_each(capture_counter[1].begin(), capture_counter[1].end(), [i = 1](int c) mutable { std::cout << i++ << ": " << c << std::endl; });
#endif
						}

						// bottom right recursive evaluation
						if (at_least_one_capture_bottom_right)
						{
							for_each(local_captures_bottom_right.begin(), local_captures_bottom_right.end(), [i = 0, this, &capture_counter, &counter, &board, &player, &x, &y](available_capture& a) mutable
								{
									//(*p).get_av_list()->push_back(new available_capture(x + 2, y - 2, x + 1, y - 1));
									capture_counter[2][i] = 1; // change here to get from counter, then increment?

									// copy coords
									int x_to_go = a.get_x();
									int y_to_go = a.get_y();
									int x_to_delete = a.get_x_d();
									int y_to_delete = a.get_y_d();

									// copy the board and make empty list for moved piece
									std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
									std::list<piece*> copy_of_list;
									
									for (int i = 0; i < s_size; ++i)
										for (int j = 0; j < s_size; ++j)
											if ((*board)[i][j] != NULL)
												(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);
									
									// make planned move
									piece* moving_piece = (*copy_of_board)[x][y];
									(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
									moving_piece->set_x(x_to_go);
									moving_piece->set_y(y_to_go);
									(*copy_of_board)[x_to_go][y_to_go] = moving_piece;
									copy_of_list.push_back(moving_piece);
									(*copy_of_board)[x_to_delete][y_to_delete] = NULL;
									moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
									std::cout << copy_of_board << std::endl;
#endif
									//evaluate recursively - separate in every direction - call tree
									if (*counter == NULL)
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter null" << std::endl;
#endif
										int moves = 1;
										evaluate(&copy_of_list, copy_of_board, &moves, player);
										capture_counter[2][i] = moves;
#ifdef _DEBUG
										std::cout << "moves counter (top right): " << moves << std::endl;
#endif
									}
									else
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter not null" << std::endl;
#endif
										(*counter)++;
										evaluate(&copy_of_list, copy_of_board, counter, player);
									}
									++i;
								});
#ifdef _DEBUG
							// print
							std::cout << "for every capture option, these are multi capture counters" << std::endl;
							for_each(capture_counter[2].begin(), capture_counter[2].end(), [i = 1](int c) mutable { std::cout << i++ << ": " << c << std::endl; });
#endif
						}

						// bottom left recursive evaluation
						if (at_least_one_capture_bottom_left)
						{
							for_each(local_captures_bottom_left.begin(), local_captures_bottom_left.end(), [i = 0, this, &capture_counter, &counter, &board, &player, &x, &y](available_capture& a) mutable
								{
									capture_counter[3][i] = 1; // change here to get from counter, then increment?

									// copy coords
									int x_to_go = a.get_x();
									int y_to_go = a.get_y();
									int x_to_delete = a.get_x_d();
									int y_to_delete = a.get_y_d();

									// copy the board and make empty list for moved piece
									std::vector<std::vector<piece*>>* copy_of_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, 0));
									std::list<piece*> copy_of_list;
									
									for (int i = 0; i < s_size; ++i)
										for (int j = 0; j < s_size; ++j)
											if ((*board)[i][j] != NULL)
												(*copy_of_board)[i][j] = new piece((*board)[i][j]->get_sign(), (*board)[i][j]->get_x(), (*board)[i][j]->get_y(), player);
									
									// make planned move
									piece* moving_piece = (*copy_of_board)[x][y];
									(*copy_of_board)[moving_piece->get_x()][moving_piece->get_y()] = NULL;
									moving_piece->set_x(x_to_go);
									moving_piece->set_y(y_to_go);
									(*copy_of_board)[x_to_go][y_to_go] = moving_piece;
									copy_of_list.push_back(moving_piece);
									(*copy_of_board)[x_to_delete][y_to_delete] = NULL;
									moving_piece = NULL; // now, copy of board contains board with moved piece and the list contains only moved piece
#ifdef _DEBUG
									std::cout << copy_of_board << std::endl;
#endif
									//evaluate recursively - separate in every direction - call tree
									if (*counter == NULL)
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter null" << std::endl;
#endif
										int moves = 1;
										evaluate(&copy_of_list, copy_of_board, &moves, player);
										capture_counter[3][i] = moves;
#ifdef _DEBUG
										std::cout << "moves counter (top right): " << moves << std::endl;
#endif
									}
									else
									{
#ifdef _DEBUG
										std::cout << "---------------------------------------------------------------" << std::endl;
										std::cout << "counter not null" << std::endl;
#endif
										(*counter)++;
										evaluate(&copy_of_list, copy_of_board, counter, player);
									}
									++i;
								});
#ifdef _DEBUG
							// print
							std::cout << "for every capture option, these are multi capture counters" << std::endl;
							for_each(capture_counter[3].begin(), capture_counter[3].end(), [i = 1](int c) mutable { std::cout << i++ << ": " << c << std::endl; });
#endif
						}

						// find maximal capture counter
						int max_captures = 0;
						for (int i = 0; i < 4; ++i)
							for_each(capture_counter[i].begin(), capture_counter[i].end(), [&max_captures](int c) { if (c > max_captures) max_captures = c; });
#ifdef _DEBUG
						std::cout << "found max captures: " << max_captures << std::endl;
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
												p->get_av_list()->push_back(new available_capture(local_captures_top_right[j].get_x(), local_captures_top_right[j].get_y(), local_captures_top_right[j].get_x_d(), local_captures_top_right[j].get_y_d()));
											break;
										case 1:
											if (multicapture && possible_top_left || !multicapture)
												p->get_av_list()->push_back(new available_capture(local_captures_top_left[j].get_x(), local_captures_top_left[j].get_y(), local_captures_top_left[j].get_x_d(), local_captures_top_left[j].get_y_d()));
											break;
										case 2:
											if (multicapture && possible_bottom_right || !multicapture)
												p->get_av_list()->push_back(new available_capture(local_captures_bottom_right[j].get_x(), local_captures_bottom_right[j].get_y(), local_captures_bottom_right[j].get_x_d(), local_captures_bottom_right[j].get_y_d()));
											break;
										case 3:
											if (multicapture && possible_bottom_left || !multicapture)
												p->get_av_list()->push_back(new available_capture(local_captures_bottom_left[j].get_x(), local_captures_bottom_left[j].get_y(), local_captures_bottom_left[j].get_x_d(), local_captures_bottom_left[j].get_y_d()));
											break;
										}
									}
									++j;
								});
					}
					else // only moves
					{
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
				}
			});
#ifdef _DEBUG
		std::cout << "Evaluation returns: ";
		av_capture ? (std::cout << "true") : (std::cout << "false");
		std::cout << std::endl;
#endif
		return av_capture;
	}

	void game::clear_list(std::list<piece*>* list) { for_each(list->begin(), list->end(), [this](piece* p) { p->get_av_list()->clear(); }); }

	void game::print_pieces(std::list<piece*>* list)
	{
		std::for_each(list->begin(), list->end(), [i = 1, this](piece* p) mutable
			{
				m_os << i++ << "; sign: " << p << "; x: " << p->get_x() << "; y: " << p->get_y();
				if (dynamic_cast<king*>(p))
					m_os << "; king";
				m_os << std::endl;
			});
	}

	void game::delete_from_list(std::list<piece*>* list, piece* piece_to_delete) { list->remove(piece_to_delete); }

	void game::clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list)
	{
		while (!(del_list->empty()))
		{
			// temporary piece from "to delete list"
			piece* tmp = del_list->front();

			int x_d = tmp->get_x();
			int y_d = tmp->get_y();
			piece* piece_to_delete = (*m_board)[x_d][y_d];
			(*m_board)[x_d][y_d] = NULL;

			delete_from_list(src_list, piece_to_delete);

			del_list->pop_front();
			//delete tmp;
		}
	}
}