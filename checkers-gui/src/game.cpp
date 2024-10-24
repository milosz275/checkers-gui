#include "game.h"
#include "king.h"

// TODO: add menu
// TODO: add animation
// TODO: scale to window resolution

namespace checkers
{
	game::game(int fps, std::istream& is, std::ostream& os) : m_event_handler(new event_handler(this)), m_game_state(new game_state(os)), m_console_game(false),
		m_is_finished(false), m_fps(fps),
		m_selected(false), m_selected_piece(nullptr), m_moving_piece(nullptr), m_available_capture(false), m_last_capture_direction(-1), m_is(is), m_os(os),
		m_log_file("log.txt", std::ios::app)
	{
		assert(s_size % 2 == 0);
		assert(m_fps > 0);

		if (m_log_file.is_open())
		{
			m_log.rdbuf(m_log_file.rdbuf());
			m_log << "Log file: Checkers game (" << s_size << "x" << s_size << ")" << std::endl;
			std::time_t current_time;
			std::time(&current_time);
			char buffer[26];
			ctime_r(&current_time, buffer);
			m_log << buffer;
		}
		else
		{
#ifdef _DEBUG
			m_os << "Log file not opened" << std::endl;
#endif
		}

		// choose how to play
		int menu_choice = 0;
		bool against_bot = false;

		m_os << "Do you want to exit (0), play against another player (1) or versus bot (2)?" << std::endl << "Enter: ";
		m_is >> menu_choice;
		if (menu_choice == 0)
		{
			m_is_finished = true;
			return;
		}
		while (menu_choice == 2)
		{
			m_os << "Chosen player vs bot" << std::endl;
			against_bot = true;
			break;
			/*m_os << "How many moves ahead should bot consider?" << std::endl << "Enter: ";
			m_is >> bot_search_depth;
			if (bot_search_depth <= 0 || bot_search_depth >= 10)
				m_os << "Bot's intelligence greater than 0 and smaller than 10" << std::endl;
			else
				break;*/
		}
		if (menu_choice == 1)
			m_os << "Chosen player vs player" << std::endl;

		// setup user interface
		m_gui = new gui(fps);
		std::string player_name_1 = "Player_1";
		std::string player_name_2 = "Player_2";
		char player_sign_1 = 'W';
		char player_sign_2 = 'B';

		// bind user input
		if (!m_console_game)
		{
			auto get_coords = std::bind(&game::get_click_coordinates, this);
			m_player_1 = new player(player_sign_1, player_name_1, get_coords);
			if (!against_bot)
				m_player_2 = new player(player_sign_2, player_name_2, get_coords);
		}
		else
		{
			auto get_coords = std::bind(&game::get_coordinates_from_stream, this);
			m_player_1 = new player(player_sign_1, player_name_1, get_coords);
			if (!against_bot)
				m_player_2 = new player(player_sign_2, player_name_2, get_coords);
		}
		int bot_search_depth = 1;
		if (against_bot)
			m_player_2 = new bot('B', bot_search_depth, m_os);

		// set play order and evaluation direction
		m_player_1->set_first(true);
		m_player_2->set_first(false);
		m_game_state->set_current_player(m_player_1);
		m_game_state->set_next_player(m_player_2);

		// board init
		m_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));

		// set pointers to piece lists
		m_player_1->set_list(&m_p_list_1);
		m_player_2->set_list(&m_p_list_2);

		// fill the board with pieces
#ifdef _DEBUG
		//populate_board(4);
		//populate_board_debug();
		load_pieces_from_file("pieces.txt");
#else
		// load pieces from file, if it fails fill with default board
		bool loaded_pieces = load_pieces_from_file("pieces.txt");
		if (!loaded_pieces)
			populate_board(s_size / 2 - 1);
#endif
		// evaluate available moves for the first player
		int dummy = 0;
		m_available_capture = evaluate(m_game_state->get_current_player()->get_list(), m_board, &dummy, dummy, m_last_capture_direction, &m_to_delete_list, nullptr);
#ifdef _DEBUG
		m_os << "Game is evaluated" << std::endl;

		m_os << "List of pieces of first player" << std::endl;
		print_pieces(m_player_1->get_list());

		m_os << "List of pieces of second player" << std::endl;
		print_pieces(m_player_2->get_list());
#endif
		m_gui->draw_board(*m_player_1->get_list(), *m_player_2->get_list(), m_to_delete_list, m_selected_piece);
	}

	game::game(const game& game) : m_fps(game.m_fps)
	{
		s_game_copied++;
#ifdef _DEBUG
		m_os << "Game copy construction overall: " << s_game_copied << std::endl;
#endif
		// copy player 1
		assert(game.m_player_1);
		if (dynamic_cast<player*>(game.m_player_1))
			m_player_1 = new player(*dynamic_cast<player*>(game.m_player_1));
		else
			m_player_1 = new bot(*dynamic_cast<bot*>(game.m_player_1));
		assert(m_player_1);

		// copy player 2
		assert(game.m_player_2);
		if (dynamic_cast<player*>(game.m_player_2))
			m_player_2 = new player(*dynamic_cast<player*>(game.m_player_2));
		else
			m_player_2 = new bot(*dynamic_cast<bot*>(game.m_player_2));
		assert(m_player_2);

		// recreate game state
		assert(game.m_game_state);
		assert(game.m_game_state->get_current_player());
		assert(game.m_game_state->get_current_player() != game.m_game_state->get_next_player());
		m_game_state = new game_state(*game.m_game_state);
		
		if (game.m_game_state->get_current_player() == game.m_player_1)
		{
			m_game_state->set_current_player(m_player_1);
			m_game_state->set_next_player(m_player_2);
		}
		else
		{
			m_game_state->set_current_player(m_player_2);
			m_game_state->set_next_player(m_player_1);
		}

		// copy the board and recreate the lists
		assert(game.m_board);
		m_board = new std::vector<std::vector<piece*>>(s_size, std::vector<piece*>(s_size, nullptr));
		std::vector<std::vector<piece*>>* original_board = game.m_board;

		m_player_1->set_list(&m_p_list_1);
		m_player_2->set_list(&m_p_list_2);

		for (int i = 0; i < s_size; ++i)
			for (int j = 0; j < s_size; ++j)
			{
				piece* p = (*original_board)[i][j];
				if (p)
				{
					if (dynamic_cast<king*>(p))
					{
						if (p->get_owner() == game.m_player_1)
							add_new_piece(m_player_1->get_list(), m_board, m_player_1, i, j, true, true);
						else if (p->get_owner() == game.m_player_2)
							add_new_piece(m_player_2->get_list(), m_board, m_player_2, i, j, true, true);
						else
							throw std::runtime_error("Copying the board: king piece: piece ownership not matching");
					}
					else
					{
						if (p->get_owner() == game.m_player_1)
							add_new_piece(m_player_1->get_list(), m_board, m_player_1, i, j, true);
						else if (p->get_owner() == game.m_player_2)
							add_new_piece(m_player_2->get_list(), m_board, m_player_2, i, j, true);
						else
							throw std::runtime_error("Copying the board: normal piece: piece ownership not matching");
					}
				}
			}
		
#ifdef _DEBUG
		m_os << "Board after copying: " << std::endl;
		m_os << m_board << std::endl;
#endif
		for_each(m_p_list_1.begin(), m_p_list_1.end(), [this](piece* p) { assert(p->get_owner() == m_player_1); });
		for_each(m_p_list_2.begin(), m_p_list_2.end(), [this](piece* p) { assert(p->get_owner() == m_player_2); });
#ifdef _DEBUG
		m_os << "List of pieces of first player" << std::endl;
		print_pieces(&m_p_list_1);

		m_os << "List of pieces of second player" << std::endl;
		print_pieces(&m_p_list_2);
#endif
		
		if (!game.m_to_delete_list.empty())
		{
			// recreate to delete list
			for_each(game.m_to_delete_list.begin(), game.m_to_delete_list.end(), [this, &game](piece* p) // check if to delete list contains opponent's pieces
				{
					assert(p);
					assert(p->get_owner() == game.m_game_state->get_next_player());
				});
			for_each(game.m_to_delete_list.begin(), game.m_to_delete_list.end(), [this](piece* p)
				{
					if (dynamic_cast<king*>(p))
					{
						if (p->get_owner()->is_first())
							m_to_delete_list.push_back(new king(p->get_x(), p->get_y(), false, m_player_1));
						else
							m_to_delete_list.push_back(new king(p->get_x(), p->get_y(), false, m_player_2));
					}
					else if (p)
					{
						if (p->get_owner()->is_first())
							m_to_delete_list.push_back(new piece(p->get_x(), p->get_y(), false, m_player_1));
						else
							m_to_delete_list.push_back(new piece(p->get_x(), p->get_y(), false, m_player_2));
					}
					else
						throw std::runtime_error("To delete list contains nullptr instead of pieces");
				});
			for_each(m_to_delete_list.begin(), m_to_delete_list.end(), [this](piece* p) // check if to delete list contains opponent's pieces
				{
					assert(p);
					assert(p->get_owner() == m_game_state->get_next_player());
				});
		}
		else
			assert(m_to_delete_list.empty());
		
		// recreate selected piece
		if (!(game.m_selected_piece))
			m_selected_piece = nullptr;
		else
		{
			assert(game.m_game_state->get_current_player() == game.m_selected_piece->get_owner());
			m_selected_piece = (*m_board)[game.m_selected_piece->get_x()][game.m_selected_piece->get_y()];
		}
		
		// recreate moving piece
		if (!(game.m_moving_piece))
			m_moving_piece = nullptr;
		else
		{
			assert(game.m_game_state->get_current_player() == game.m_moving_piece->get_owner());
			m_moving_piece = (*m_board)[game.m_moving_piece->get_x()][game.m_moving_piece->get_y()];
		}

		// check if the source game was normal
		assert((m_to_delete_list.empty() && !m_moving_piece) || (!m_to_delete_list.empty() && m_moving_piece));
		assert((game.m_to_delete_list.empty() && !game.m_moving_piece) || (!game.m_to_delete_list.empty() && game.m_moving_piece));

		// other fields
		m_selected = game.m_selected;
		m_last_capture_direction = game.m_last_capture_direction;
		m_console_game = game.m_console_game;
		m_is_finished = game.m_is_finished;
		m_gui = nullptr;
		m_event_handler = nullptr;

		// evaluate available moves for the current player
#ifdef _DEBUG
		m_os << "Starting game copy evaluation" << std::endl;
#endif
		int counter = 0;
		m_available_capture = evaluate(m_game_state->get_current_player()->get_list(), m_board, &counter, counter, m_last_capture_direction, &m_to_delete_list, m_moving_piece);
#ifdef _DEBUG
		m_os << "Game copy evaluated" << std::endl;
#endif
	}

	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<piece*>>* board)
	{
		os << "\t  ";
		for (char a = 'a'; a < 'a' + checkers::s_size; ++a) // columns as letters
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
		if (m_gui)
			delete m_gui;
		if (m_event_handler)
			delete m_event_handler;
		if (m_game_state)
			delete m_game_state;
		m_p_list_1.clear();
		m_p_list_2.clear();
		for_each(m_to_delete_list.begin(), m_to_delete_list.end(), [](piece* p) { assert(p); delete p; });
		m_to_delete_list.clear();
		m_selected_piece = nullptr;
		m_moving_piece = nullptr;
		for (auto& row : *m_board)
			for (piece* p : row) delete p;
		delete m_board;

		if (m_player_1)
			delete m_player_1;
		if (m_player_2)
			delete m_player_2;
#ifdef _DEBUG
		m_os << "Deleted game" << std::endl;
#endif
	}

	bool game::get_is_finished(void) { return m_is_finished; }

	gui* game::get_gui(void) { return m_gui; }
	
	event_handler* game::get_event_handler(void) { return m_event_handler; }
	
	game_state* game::get_game_state(void) { return m_game_state; }

	int game::get_score(void) { return (int)m_p_list_1.size() - (int)m_p_list_2.size(); }

	int game::get_last_capture_direction(void) { return m_last_capture_direction; }

	int game::set_last_capture_direction(int direction) { assert(direction >= 0 && direction < 4); return m_last_capture_direction = direction; }

	base_player* game::get_player_1(void) { return m_player_1; }

	base_player* game::get_player_2(void) { return m_player_2; }

	base_player* game::set_player_1(base_player* player) { return m_player_1 = player; }

	base_player* game::set_player_2(base_player* player) { return m_player_2 = player; }

	std::list<piece*>& game::get_list_1(void) { return m_p_list_1; }

	std::list<piece*>& game::get_list_2(void) { return m_p_list_2; }

	bool game::get_selected(void) { return m_selected; }

	bool game::set_selected(bool flag) { return m_selected = flag; }

	piece* game::get_selected_piece(void) { return m_selected_piece; }

	piece* game::set_selected_piece(piece* p) { return m_selected_piece = p; }

	piece* game::get_moving_piece(void) { return m_moving_piece; }

	piece* game::set_moving_piece(piece* p) { return m_moving_piece = p; }

	bool game::get_available_capture(void) { return m_available_capture; }

	bool game::set_available_capture(bool flag) { return m_available_capture = flag; }

	std::list<piece*>* game::get_pieces(void) { return m_game_state->get_current_player()->get_list(); }

	std::ostream& game::get_os(void) { return m_os; }

	std::ostream& game::get_log(void) { return m_log; }

	std::ofstream& game::get_log_file(void) { return m_log_file; }

	void game::populate_board(int rows)
	{
		assert(m_board->size() == s_size);
		assert(rows <= s_size / 2);

		// rows of the second player (upper)
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < s_size; ++j)
				if ((i + j) % 2 != 0)
				{
					add_new_piece(&m_p_list_2, m_board, m_player_2, j, i, true, m_gui);
					s_pieces_initialized++;
				}

		// rows of the first player (lower)
		for (int i = s_size - 1; i >= s_size - rows; --i)
			for (int j = 0; j < s_size; ++j)
				if ((i + j) % 2 != 0)
				{
					add_new_piece(&m_p_list_1, m_board, m_player_1, j, i, true, m_gui);
					s_pieces_initialized++;
				}

		m_log << "Populated board: row count: " << rows << std::endl;
	}

	void game::populate_board_debug(void)
	{
		assert(m_board->size() == s_size);
		//...
	}

	bool game::load_pieces_from_file(std::string file_name)
	{
		assert(m_player_1 && m_player_2);
		assert(m_player_1->get_list() && m_player_2->get_list());
		assert(m_board);

		std::ifstream file(file_name);
		if (!file.is_open())
			return false;

		// data for moving piece set at the end
		int moving_piece_x = -1;
		int moving_piece_y = -1;

		std::string line;
		int line_index = 0;
		std::queue<piece*> loaded_pieces;
		if (std::getline(file, line))
		{
			if (line.empty() || line == "")
				return false;
			++line_index;
			std::stringstream ss(line);

			// player turn
			int turn;
			if (!(ss >> turn))
			{
#ifdef _DEBUG
				m_os << "Incorrect data at line: " << line_index << std::endl;
#endif 
				if (turn != 1 && turn != 2)
				{
#ifdef _DEBUG
					m_os << "Wrong input data: player turn" << std::endl;
#endif
				}
				else
				{
#ifdef _DEBUG
					m_os << "Wrong dataset length: player turn" << std::endl;
#endif
				}
				return false;
			}
			if (turn == 2)
			{
				m_game_state->set_current_player(m_player_2);
				m_game_state->set_next_player(m_player_1);
			}
			else if (turn != 1)
				return false;
		}
		if (std::getline(file, line))
		{
			if (line.empty() || line == "")
				return false;
			++line_index;
			std::stringstream ss(line);

			// data of moving piece
			int x = -1, y = -1;
			if (!(ss >> x >> y))
			{
#ifdef _DEBUG
				m_os << "Incorrect data at line: " << line_index << std::endl;
#endif 
				if (x < -1 || y < -1 || x > s_size - 1 || y > s_size - 1)
				{
#ifdef _DEBUG
					m_os << "Wrong input data: moving piece" << std::endl;
#endif
				}
				else
				{
#ifdef _DEBUG
					m_os << "Wrong dataset length: moving piece" << std::endl;
#endif
				}
				return false;
			}
			if ((x == -1 || y == -1) || (x >= 0 && x < s_size && y >= 0 && y < s_size))
				moving_piece_x = x, moving_piece_y = y;
			else
				return false;
		}
		while (std::getline(file, line))
		{
			++line_index;
			std::stringstream ss(line);

			// data of each piece
			int x = -1, y = -1;
			int owner = -1;
			int is_king = -1;
			int is_alive = -1;
			if (!(ss >> x >> y >> owner >> is_king >> is_alive))
			{
#ifdef _DEBUG
				m_os << "Incorrect data at line: " << line_index << std::endl;
#endif 
				if (x < 0 || y < 0 || x > s_size - 1 || y > s_size - 1 || (*m_board)[x][y] || owner != 1 || owner != 2 || is_king != 0 || is_king != 1 || is_alive != 0 || is_alive != 1)
				{
#ifdef _DEBUG
					m_os << "Wrong input data" << std::endl;
#endif 
				}
				else
				{
#ifdef _DEBUG
					m_os << "Wrong dataset length"  << std::endl;
#endif 
				}
				while (!loaded_pieces.empty())
					loaded_pieces.pop();
				return false;
			}
			if (is_king && owner == 1)
				loaded_pieces.push(new king(x, y, is_alive, m_player_1));
			else if (is_king && owner == 2)
				loaded_pieces.push(new king(x, y, is_alive, m_player_2));
			else if (!is_king && owner == 1)
				loaded_pieces.push(new piece(x, y, is_alive, m_player_1));
			else if (!is_king && owner == 2)
				loaded_pieces.push(new piece(x, y, is_alive, m_player_2));
			else
			{
				while (!loaded_pieces.empty())
					loaded_pieces.pop();
				return false;
			}
		}
		if (line_index < 3)
			return false;
		while (!loaded_pieces.empty())
		{
			piece* p = loaded_pieces.front();
			assert(!(*m_board)[p->get_x()][p->get_y()]);
			if (p->is_alive())
			{
				if (dynamic_cast<king*>(p))
				{
					add_new_piece(p->get_owner()->get_list(), m_board, p->get_owner(), p->get_x(), p->get_y(), true, true, m_gui);
					s_pieces_initialized++;
				}
				else
				{
					add_new_piece(p->get_owner()->get_list(), m_board, p->get_owner(), p->get_x(), p->get_y(), true, m_gui);
					s_pieces_initialized++;
				}
			}
			else
			{
				m_to_delete_list.push_back(new piece(p->get_x(), p->get_y(), false, p->get_owner(), m_gui));
			}
			loaded_pieces.pop();
		}

		// setup the moving piece
		if (moving_piece_x == -1 || moving_piece_y == -1)
			m_moving_piece = nullptr;
		else
		{
			assert((*m_board)[moving_piece_x][moving_piece_y]);
			m_moving_piece = (*m_board)[moving_piece_x][moving_piece_y];
		}
#ifndef _DEBUG
		const int pieces_of_each_player = (s_size / 2 - 1) * (s_size / 2);
		m_player_1->set_captured_pieces(pieces_of_each_player - (int)m_player_1->get_list()->size());
		m_player_2->set_captured_pieces(pieces_of_each_player - (int)m_player_2->get_list()->size());
#endif
		file.close();
		return true;
	}

	void game::save_to_file(std::string file_name)
	{
		if (m_player_1->get_list()->empty() || m_player_2->get_list()->empty())
			return;
#ifdef _DEBUG
		m_os << "Saving game state..." << std::endl;
#endif 
		std::ofstream file(file_name, std::ios::out);
		if (file.is_open())
		{
			if (m_game_state->get_current_player() == m_player_1)
				file << "1" << std::endl;
			else
				file << "2" << std::endl;
			if (m_moving_piece)
				file << m_moving_piece->get_x() << " " << m_moving_piece->get_y() << std::endl;
			else
				file << "-1 -1" << std::endl;
			for_each(m_board->begin(), m_board->end(), [this, &file](std::vector<piece*>& row)
				{
					for_each(row.begin(), row.end(), [this, &file](piece* p)
						{
							if (p)
							{
								assert(p->get_gui());
								bool is_king = dynamic_cast<king*>(p);
								if (p->get_owner() == m_player_1)
									file << p->get_x() << " " << p->get_y() << " 1 " << is_king << " 1" << std::endl;
								else if (p->get_owner() == m_player_2)
									file << p->get_x() << " " << p->get_y() << " 2 " << is_king << " 1" << std::endl;
							}
						});
				});
			for_each(m_to_delete_list.begin(), m_to_delete_list.end(), [this, &file](piece* p)
				{
					assert(p);
					if (p->get_owner() == m_player_1)
						file << p->get_x() << " " << p->get_y() << " 1 0 0" << std::endl;
					else if (p->get_owner() == m_player_2)
						file << p->get_x() << " " << p->get_y() << " 2 0 0" << std::endl;
				});
#ifdef _DEBUG
			m_os << "Saving game state complete" << std::endl;
			m_log << "Game state saved to the log file" << std::endl;
#endif
		}
		else
		{
#ifdef _DEBUG
			m_os << "Failed to save game state" << std::endl;
#endif
		}
	}

	void game::add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y, bool is_alive, gui* ui)
	{
		if ((*board)[x][y])
			return;
		piece* p = new piece(x, y, is_alive, player, ui);
		(*board)[x][y] = p;
		if (list)
			list->push_back(p);
		bool changed_to_king = player->change_to_king(p, board);
#ifdef _DEBUG
		if (changed_to_king)
			m_os << "Added new king!" << std::endl;
		else
			m_os << "Added new piece!" << std::endl;
#endif
	}

	void game::add_new_piece(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, base_player* player, int x, int y, bool is_alive, bool force_king, gui* ui)
	{
		if ((*board)[x][y])
			return;
		if (force_king)
		{
			king* p = new king(x, y, is_alive, player, ui);
			(*board)[x][y] = p;
			if (list)
				list->push_back(p);
		}
		else
		{
			piece* p = new piece(x, y, is_alive, player, ui);
			(*board)[x][y] = p;
			if (list)
				list->push_back(p);
		}
#ifdef _DEBUG
		if (force_king)
			m_os << "Added new king!" << std::endl;
		else
			m_os << "Added new piece!" << std::endl;
#endif
	}

	std::vector<std::vector<piece*>>* game::get_board(void) { assert(m_board); return m_board; }

	std::list<piece*>* game::get_to_delete_list(void) { return &m_to_delete_list; }

	std::pair<int, int> game::get_coordinates(void)
	{	
		m_game_state->set_game_freeze(true);
		bot* player_bot = dynamic_cast<bot*>(m_game_state->get_current_player());
		if (player_bot)
		{
			game* previous = player_bot->get_game();
			assert(this != previous);
#ifdef _DEBUG
			m_os << "Game: Setting new game copy for bot" << std::endl;
#endif
			player_bot->set_game(new game(*this));

			if (previous)
			{
#ifdef _DEBUG
				m_os << "Game: Deleting previous game copy for bot" << std::endl;
#endif
				delete previous;
			}
		}
		std::pair<int, int> coords = m_game_state->get_current_player()->get_coordinates();
		assert(coords.first >= 0 && coords.first < s_size && coords.second >= 0 && coords.second < s_size);
		m_game_state->set_game_freeze(false);
		return coords;
	}

	std::pair<int, int> game::get_click_coordinates(void) { return m_gui->get_click_coordinates(); }

	int get_int_from_stream(std::istream& is)
	{
		int integer = -1;
		is >> integer;
		return integer;
	}

	std::pair<int, int> game::get_coordinates_from_stream(void)
	{
		// change to string, add checks if it is a number etc.
		int x = 0;
		int y = 0;

		while (x < 1 || x > 10)
		{
			m_os << "Give the x coordinate: ";
			x = get_int_from_stream(m_is);
		}
		while (y < 1 || y > 10)
		{
			m_os << "Give the y coordinate: ";
			y = get_int_from_stream(m_is);
		}
		--x;
		--y;

		return std::make_pair(x, y);
	}

	void game::print_results(std::ostream& os)
	{
		os << "Results:" << std::endl;
		if (!m_game_state->get_first_won() && !m_game_state->get_second_won())
			os << "Game wasn't finished" << std::endl;
		else if (m_game_state->get_first_won() && m_game_state->get_second_won())
			os << "Draw" << std::endl;
		else if (m_game_state->get_first_won())
			os << "Player: \"" << m_player_1->get_name() << "\" won!" << std::endl;
		else
			os << "Player: \"" << m_player_2->get_name() << "\" won!" << std::endl;

		os << "Player \"" << m_player_1->get_name() << "\" captured " << m_player_2->get_captured_pieces() << " pieces" << std::endl;
		os << "Player \"" << m_player_2->get_name() << "\" captured " << m_player_1->get_captured_pieces() << " pieces" << std::endl;
	}

	void game::select_piece(void)
	{
		// dereference old selected piece
		m_selected = false;
		m_selected_piece = nullptr;

		// getting coords of the click after highlighting selected piece, ignore clicks outside
		std::pair<int, int> coordinates = get_coordinates();
		int x = std::get<0>(coordinates);
		int y = std::get<1>(coordinates);

		if (x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1)
			return;

		//
		if (!m_to_delete_list.empty() && (*m_board)[x][y] != m_moving_piece)
		{
#ifdef _DEBUG
			if ((*m_board)[x][y])
				m_os << "Multicapture: this isn't the moving piece." << std::endl;
			else
				m_os << "Multicapture: this is an empty space." << std::endl;
#endif
			return;
		}

		// check if the correspoding field contains a piece
		if ((*m_board)[x][y])
		{
#ifdef _DEBUG
			m_os << "x: " << x << "; y: " << y << "; piece: " << (*m_board)[x][y] << std::endl;
#endif
			// check if player owns this piece
			if ((*m_board)[x][y]->get_owner() == m_game_state->get_current_player())
			{
#ifdef _DEBUG
				m_os << "That piece belongs to you" << std::endl;
#endif
				bool at_least_one_move = false;
				bool found_capture = false;
				if (!(*m_board)[x][y]->get_av_list()->empty())
				{
					// find at least one move
					for (available_move* a : *(*m_board)[x][y]->get_av_list())
					{
						if (a)
						{
							at_least_one_move = true;
							break;
						}
					}

					if (at_least_one_move)
					{
						// find at least one move that is a capture
						for (available_move* a : *(*m_board)[x][y]->get_av_list())
						{
							if (dynamic_cast<available_capture*>(a))
							{
								found_capture = true;
								break;
							}
						}
#ifdef _DEBUG
						for_each((*m_board)[x][y]->get_av_list()->begin(), (*m_board)[x][y]->get_av_list()->end(), [this](available_move* a)
							{
								m_os << "available: x: " << a->get_x() << "; y: " << a->get_y();
								if (dynamic_cast<available_capture*>(a))
									m_os << "; max captures: " << dynamic_cast<available_capture*>(a)->get_max_score();
								m_os << std::endl;
							});
#endif
					}
				}
				if (!at_least_one_move)
				{
#ifdef _DEBUG
					m_os << "No moves for this piece - not selecting" << std::endl;
#endif
				}
				else if ((found_capture && m_available_capture) || (!found_capture && !m_available_capture)) // this lets making only capture moves, comment out to enable testing - replace to xnor
				{
#ifdef _DEBUG
					m_os << "Selecting" << std::endl;
#endif
					m_selected_piece = (*m_board)[x][y];
					m_selected = true;
					m_game_state->set_any_changes(true);
					return;
				}
			}
			else
			{
#ifdef _DEBUG
				m_os << "That piece does not belong to you" << std::endl;
#endif
			}
		}
		else
		{
#ifdef _DEBUG
			m_os << "x: " << x << "; y: " << y << std::endl;
#endif
		}
	}

	void game::move_selected_piece(void)
	{
		assert(m_selected_piece); // choice after highlighting

		if (m_selected_piece->get_av_list()->empty())
		{
#ifdef _DEBUG
			m_os << "Empty available list: deselecting" << std::endl;
#endif
			m_selected_piece = nullptr;
			m_selected = false;
			m_game_state->set_any_changes(true);
			return;
		}

		// getting coords of the click after highlighting selected piece, ignore clicks outside
		std::pair<int, int> coordinates = get_coordinates();
		int x = std::get<0>(coordinates);
		int y = std::get<1>(coordinates);

		if (x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1)
		{
			m_selected = false;
			m_selected_piece = nullptr;
			m_game_state->set_any_changes(true);
			return;
		}

		// find corresponding piece
		bool is_found = false;
		available_move* found_move = nullptr;

		for (available_move* a : *m_selected_piece->get_av_list())
		{
			// check if selected coords match any of possible moves
			if (a->get_x() == x && a->get_y() == y)
			{
#ifdef _DEBUG
				m_os << a->get_x() << " " << a->get_y() << std::endl;
#endif		
				found_move = a;
				is_found = true;
				break;
			}
		}

		if (!is_found) // deselection when wrong coords given
		{
			m_selected = false;
			m_selected_piece = nullptr;
			m_game_state->set_any_changes(true);
		}
		else // making a move
		{
			m_game_state->set_any_changes(true);
			available_capture* found_capture = dynamic_cast<available_capture*>(found_move);
			if (found_capture)
			{
				// mark found capture
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
				m_os << "Coords to delete: x: " << x_d << "; y: " << y_d << std::endl;
#endif
				// delete captured piece
				piece* piece_to_delete = (*m_board)[x_d][y_d];
				delete_piece(piece_to_delete, m_board, m_game_state->get_next_player());
				m_game_state->get_next_player()->add_capture();

				// check game completion
				if (m_game_state->get_next_player()->get_list()->empty())
				{
					if (m_game_state->get_current_player()->is_first())
						m_game_state->set_first_won(true);
					else
						m_game_state->set_second_won(true);
#ifdef _DEBUG
					m_os << "Setting the end of the game flags: current player has no pieces" << std::endl;
#endif
				}

				// if there is no multicapture, set new moving piece
				if (m_to_delete_list.empty())
					m_moving_piece = m_selected_piece;

				// create new piece which represents dead piece during multicapture, it is indifferent whether it was normal piece or king
				m_to_delete_list.push_back(new piece(x_d, y_d, false, m_game_state->get_next_player(), m_gui));
				m_game_state->get_current_player()->set_combo(true);
#ifdef _DEBUG
				m_os << m_game_state->get_current_player()->get_name() << " combo" << std::endl;
#endif
			}

			// move the piece (piece, which is moving -> both capture and normal move), keep selected_piece pointer for the possible king
			move_piece(m_selected_piece, m_board, x, y);
			(*m_board)[x][y] = m_selected_piece;
			m_selected = false;
#ifdef _DEBUG
			m_os << "List of pieces of first player" << std::endl;
			print_pieces(&m_p_list_1);
			m_os << "List of pieces of second player" << std::endl;
			print_pieces(&m_p_list_2);
#endif		
			// tmp flag indicating, that the king check was made this round
			bool made_king_check = false;
			bool changed_to_king = false;

			// switch turn, if no combo
			if (!m_player_1->get_combo() && !m_player_2->get_combo())
			{
				// king function
				if (!made_king_check)
					changed_to_king = m_game_state->get_current_player()->change_to_king(m_selected_piece, m_board);
				made_king_check = true;
#ifdef _DEBUG
				if (changed_to_king)
					m_os << m_game_state->get_current_player()->get_name() << " changed his piece to king!" << std::endl;
#endif
				m_game_state->switch_turn();
				// TODO: move to game state
				m_selected = false;
				m_selected_piece = nullptr;
				m_moving_piece = nullptr;
			}
			else // section to test (fixes stuff)
			{
				clear_list(&m_p_list_1);
				clear_list(&m_p_list_2);
			}

			// evaluate current player and check if there is more captures, if not, check for new kings
			int counter = 0;
			m_available_capture = evaluate(m_game_state->get_current_player()->get_list(), m_board, &counter, counter, m_last_capture_direction, &m_to_delete_list, m_moving_piece);
#ifdef _DEBUG
			m_os << "main game loop: first evaluation counter: " << counter << std::endl;
#endif

			// exit the combo, if no more captures
			if (m_game_state->get_current_player()->get_combo() && !m_available_capture)
			{
				m_moving_piece = nullptr;

				// delete opponent's pieces of multi capture, clear failed list of possible moves, cancel combo, evaluate again
				if (m_game_state->get_current_player()->is_first())
				{
					clear_to_delete_list(&m_to_delete_list, &m_p_list_2);
					clear_list(&m_p_list_1);
				}
				else
				{
					clear_to_delete_list(&m_to_delete_list, &m_p_list_1);
					clear_list(&m_p_list_2);
				}
				m_game_state->get_current_player()->set_combo(false);
				//m_game_state->get_next_player()->set_combo(false);
#ifdef _DEBUG
				m_os << "Combo cancelled" << std::endl;
#endif
				// king function
				if (!made_king_check)
					changed_to_king = m_game_state->get_current_player()->change_to_king(m_selected_piece, m_board);
				made_king_check = true;
#ifdef _DEBUG
				if (changed_to_king)
					m_os << m_game_state->get_current_player()->get_name() << " changed his piece to king!" << std::endl;
#endif
				m_game_state->switch_turn();
				// TODO: move to game state
				m_selected = false;
				m_selected_piece = nullptr;
				m_moving_piece = nullptr;
				m_available_capture = evaluate(m_game_state->get_current_player()->get_list(), m_board, &counter, counter, m_last_capture_direction, &m_to_delete_list, m_moving_piece);
#ifdef _DEBUG
				m_os << "main game loop: second evaluation counter: " << counter << std::endl;
#endif
				m_game_state->check_completion(); // TODO: CHECK GAME STATE FOR NO POSSIBLE MOVES
			}
			else // continue the combo
			{
				clear_list(m_game_state->get_next_player()->get_list());
			}

			m_selected_piece = nullptr;
		}
	}

	void game::make_capture(std::vector<std::vector<piece*>>* board, piece* moving_piece, piece* deleted_piece, int new_x, int new_y, std::list<piece*>* dead_list)
	{
		assert(board && moving_piece);
		assert(deleted_piece);

		int x_to_delete = deleted_piece->get_x();
		int y_to_delete = deleted_piece->get_y();

		move_piece(moving_piece, board, new_x, new_y);
		
		dead_list->push_back(deleted_piece);
		delete_piece(deleted_piece, board, deleted_piece->get_owner());
	}

	void game::move_piece(piece* piece_to_move, std::vector<std::vector<piece*>>* board, int new_x, int new_y)
	{
		assert(board && piece_to_move);
		assert(!((*board)[new_x][new_y]));

		int old_x = piece_to_move->get_x();
		int old_y = piece_to_move->get_y();

		assert((*board)[old_x][old_y] == piece_to_move);


		(*board)[old_x][old_y] = nullptr;
		piece_to_move->set_x(new_x);
		piece_to_move->set_y(new_y);
		(*board)[new_x][new_y] = piece_to_move;
		
		m_log << "Moved piece: origin: x:" << old_x << "; y: " << old_y << "; destination: x: " << new_x << "; y: " << new_y << std::endl;
	}

	void game::delete_piece(piece* piece_to_delete, std::vector<std::vector<piece*>>* board, base_player* owner)
	{
		int x = piece_to_delete->get_x();
		int y = piece_to_delete->get_y();

		assert((*board)[x][y] == piece_to_delete);
		assert(piece_to_delete->get_owner() == owner);

		(*board)[x][y] = nullptr;

		delete_from_list(owner->get_list(), piece_to_delete);
		m_log << "Deleted piece: x: " << x << "; y: " << y << std::endl;
	}

	void game::copy_board(std::vector<std::vector<piece*>>* source_board, std::vector<std::vector<piece*>>* copy_of_board, base_player* owner_1, base_player* owner_2) // TODO: first: == owner; second: == is_first_player
	{
		assert(source_board);
		assert(copy_of_board);
		assert(owner_1);
		assert(owner_2);
		
		base_player* player_1 = nullptr;
		base_player* player_2 = nullptr;
		if (owner_1->is_first())
		{
			player_1 = owner_1;
			player_2 = owner_2;
		}
		else if (owner_2->is_first())
		{
			player_1 = owner_2;
			player_2 = owner_1;
		}
		else
			throw std::runtime_error("Copying board: none of the players is a first player");
		assert(player_1 != player_2);
		assert((player_1->is_first() && !player_2->is_first()) || (!player_1->is_first() && player_2->is_first()));

		for (int i = 0; i < s_size; ++i)
			for (int j = 0; j < s_size; ++j)
			{
				piece* p = (*source_board)[i][j];
				if (p)
				{
					if (dynamic_cast<king*>(p))
					{
						if (p->get_owner()->is_first())
							add_new_piece(nullptr, copy_of_board, player_1, i, j, true, true);
						else
							add_new_piece(nullptr, copy_of_board, player_2, i, j, true, true);
					}
					else
					{
						if (p->get_owner()->is_first())
							add_new_piece(nullptr, copy_of_board, player_1, i, j, true);
						else 
							add_new_piece(nullptr, copy_of_board, player_2, i, j, true);
					}
				}
			}
	}

	void game::loop(void)
	{
		// start frame clock
		m_gui->get_clock().restart();

		// inform about game start
		m_os << "Game loop started!" << std::endl;

		// main loop
		while (m_gui->get_window().isOpen())
		{
			m_event_handler->handle_events();
			if (m_game_state->get_any_changes())
			{
				m_game_state->set_any_changes(false);
				m_gui->draw_board(*m_player_1->get_list(), *m_player_2->get_list(), m_to_delete_list, m_selected_piece);
			}
		}
		if (!m_game_state->get_first_won() && !m_game_state->get_second_won()) // save the game state, when game wasn't finished
			save_to_file("pieces.txt");
		else
			std::ofstream file("pieces.txt", std::ios::out); // clear the pieces file to start from the beginning next time
		print_results(m_os);

		if (m_log_file.is_open())
			print_results(m_log);
	}

	bool game::evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece) // add moving piece
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

	bool game::evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive)
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
						throw std::runtime_error(std::string("Evaluation error on piece ") + p->get_owner()->get_sign());
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

	bool game::evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list)
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

		if (player == nullptr || opponent == nullptr) {
            throw std::runtime_error("Player or opponent is null");
        }

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
							m_os << "recursive evaluation: outer call"  << std::endl;
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

	void game::clear_list(std::list<piece*>* list) { assert(list); for_each(list->begin(), list->end(), [this](piece* p) { assert(p); p->get_av_list()->clear(); }); }

	void game::print_pieces(std::list<piece*>* list)
	{
		assert(list);
		std::for_each(list->begin(), list->end(), [i = 1, this](piece* p) mutable
			{
				m_os << i++ << "; sign: " << p << "; x: " << p->get_x() << "; y: " << p->get_y();
				if (dynamic_cast<king*>(p))
					m_os << "; king";
				m_os << std::endl;
			});
	}

	void game::delete_from_list(std::list<piece*>* list, piece* piece_to_delete)
	{
		assert(list);
		//assert(piece_to_delete);
		list->remove(piece_to_delete);
	}

	void game::clear_to_delete_list(std::list<piece*>* del_list, std::list<piece*>* src_list)
	{
		while (!(del_list->empty()))
		{
			// temporary piece from "to delete list"
			piece* tmp = del_list->front();

			int x_d = tmp->get_x();
			int y_d = tmp->get_y();
			piece* piece_to_delete = (*m_board)[x_d][y_d];
			(*m_board)[x_d][y_d] = nullptr;

			delete_from_list(src_list, piece_to_delete);

			del_list->pop_front();
		}
	}

	void game::debug_info(std::ostream& os)
	{
		os << "Board:" << std::endl;
		os << m_board << std::endl;

		os << "console game: ";
		m_console_game ? os << "true" << std::endl : os << "false" << std::endl;

		m_player_1 ? os << "player 1: " << m_player_1 : os << "not set" << std::endl;

		m_player_2 ? os << "player 2: " << m_player_2 : os << "not set" << std::endl;

		m_game_state->get_current_player() ? os << "current player: " << m_game_state->get_current_player() : os << "not set" << std::endl;

		os << "available capture: ";
		m_available_capture ? os << "true" << std::endl : os << "false" << std::endl;

		os << "last capture direction: ";
		switch (m_last_capture_direction)
		{
		case 0:
			os << "top right" << std::endl;
			break;
		case 1:
			os << "top left" << std::endl;
			break;
		case 2:
			os << "bottom right" << std::endl;
			break;
		case 3:
			os << "bottom left" << std::endl;
			break;
		default:
			os << "not set" << std::endl;
		}

		os << "finished game: ";
		m_is_finished ? os << "true" << std::endl : os << "false" << std::endl;

		os << "first won: ";
		m_game_state->get_first_won() ? os << "true" << std::endl : os << "false" << std::endl;

		os << "second won: ";
		m_game_state->get_second_won() ? os << "true" << std::endl : os << "false" << std::endl;

		os << "selected: ";
		m_selected ? os << "true" << std::endl : os << "false" << std::endl;

		os << "selected piece: ";
		if (m_selected_piece)
		{
			os << "sign: " << m_selected_piece->get_owner()->get_sign() << "; x: " << m_selected_piece->get_x() << "; y: " << m_selected_piece->get_y() << "; list of available moves: " << m_selected_piece->get_av_list()->size();
			os << "; is king: ";
			dynamic_cast<king*>(m_selected_piece) ? os << "true" << std::endl : os << "false" << std::endl;
		}
		else
		{
			os << "nullptr" << std::endl;
		}

		os << "moving piece: ";
		if (m_moving_piece)
		{
			os << "sign: " << m_moving_piece->get_owner()->get_sign() << "; x: " << m_moving_piece->get_x() << "; y: " << m_moving_piece->get_y() << "; list of available moves: " << m_moving_piece->get_av_list()->size();
			os << "; is king: ";
			dynamic_cast<king*>(m_moving_piece) ? os << "true" << std::endl : os << "false" << std::endl;
		}
		else
		{
			os << "nullptr" << std::endl;
		}

		std::pair<int, int> coords = get_click_coordinates();
		int x = std::get<0>(coords);
		int y = std::get<1>(coords);
		if (!(x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1))
		{
			piece* hovered_piece = (*m_board)[x][y];
			os << "hovered piece: ";
			if (hovered_piece)
			{
				os << "sign: " << hovered_piece->get_owner()->get_sign() << "; x: " << hovered_piece->get_x() << "; y: " << hovered_piece->get_y() << "; list of available moves: " << hovered_piece->get_av_list()->size();
				os << "; is king: ";
				dynamic_cast<king*>(hovered_piece) ? os << "true" << std::endl : os << "false" << std::endl;
			}
			else
				os << "nullptr" << std::endl;
		}
		os << "fps: " << m_fps << std::endl;
	}

	void game::debug_ptrs(void)
	{
		if (m_board)
			m_os << "Board\t true" << std::endl;
		else
			m_os << "Board\t false" << std::endl;
		
		if (m_gui)
			m_os << "Gui\t true" << std::endl;
		else
			m_os << "Gui\t false" << std::endl;

		if (m_event_handler)
			m_os << "Event handler\t true" << std::endl;
		else
			m_os << "Event handler\t false" << std::endl;

		if (m_game_state)
			m_os << "Game state\t true" << std::endl;
		else
			m_os << "Game state\t false" << std::endl;

		if (m_player_1)
			m_os << "Player 1\t true" << std::endl;
		else
			m_os << "Player 1\t false" << std::endl;

		if (m_player_2)
			m_os << "Player 2\t true" << std::endl;
		else
			m_os << "Player 2\t false" << std::endl;

		if (m_selected_piece)
			m_os << "Selected piece\t true" << std::endl;
		else
			m_os << "Selected piece\t false" << std::endl;

		if (m_moving_piece)
			m_os << "Moving piece\t true" << std::endl;
		else
			m_os << "Moving piece\t false" << std::endl;
	}

	void game::debug_setup(void)
	{
		populate_board(4);
		int dummy = 0;
		m_available_capture = evaluate(m_game_state->get_current_player()->get_list(), m_board, &dummy, dummy, m_last_capture_direction, &m_to_delete_list, nullptr);
		m_os << "Game is evaluated" << std::endl;

		m_os << "List of pieces of first player" << std::endl;
		print_pieces(m_player_1->get_list());

		m_os << "List of pieces of second player" << std::endl;
		print_pieces(m_player_2->get_list());
	}
}
