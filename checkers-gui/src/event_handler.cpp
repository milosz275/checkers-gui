#include "event_handler.h"
#include "game.h"
#include "piece.h"
#include "king.h"

namespace checkers
{
	event_handler::event_handler(game* game_pointer) : m_game_pointer(game_pointer) {}

	event_handler::~event_handler() {}

	void event_handler::handle_events(void)
	{
		std::ostream& os = m_game_pointer->get_os();
		std::ostream& log = m_game_pointer->get_log();
		sfml::window& window = m_game_pointer->get_gui()->get_window();
		sfml::event& event = m_game_pointer->get_gui()->get_event();
		while (window.pollEvent(event) || (bool)dynamic_cast<bot*>(m_game_pointer->get_game_state()->get_current_player()))
		{
			if (event.type == sfml::event::Closed || event.type == sfml::event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			{
				window.close();
#ifdef _DEBUG
				os << "Closing the game" << std::endl;
#endif
				break;
			}
#ifdef _DEBUG
			if (event.type == sfml::event::KeyPressed && event.key.code == sf::Keyboard::R)
			{
				os << "Reseted the game" << std::endl;
				m_game_pointer->get_game_state()->reset_completion();
				m_game_pointer->get_game_state()->reset_state();
				m_game_pointer->get_player_1()->set_captured_pieces(0);
				m_game_pointer->get_player_2()->set_captured_pieces(0);
				break;
			}
#endif
			if ((m_game_pointer->get_player_1()->get_pieces() == 0 || m_game_pointer->get_player_2()->get_pieces() == 0) && (m_game_pointer->get_player_1()->get_captured_pieces() > 0 || m_game_pointer->get_player_2()->get_captured_pieces() > 0))
				break;
			else if (m_game_pointer->get_game_state()->get_game_freeze())
			{
				if (event.type == sfml::event::KeyPressed || event.type == sfml::event::MouseButtonPressed)
				{
					if (event.key.code == sf::Keyboard::X)
					{
						os << "Unfrozen the game." << std::endl;
						m_game_pointer->get_game_state()->set_game_freeze(false);
					}
					else
						os << "Game is frozen" << std::endl;
					m_game_pointer->debug_info(os);
				}
				break;
			}
			else if (event.type == sfml::event::Resized)
			{
				m_game_pointer->get_game_state()->set_any_changes(true);
				os << "test" << std::endl;
			}
			else if (event.type == sfml::event::MouseButtonPressed && event.mouseButton.button == sfml::mouse::Left || (bool)dynamic_cast<bot*>(m_game_pointer->get_game_state()->get_current_player()) || m_game_pointer->m_console_game)
			{
				if (!m_game_pointer->get_selected())
				{
					m_game_pointer->select_piece();
					break;
				}
				if (m_game_pointer->get_selected_piece())
				{
					m_game_pointer->move_selected_piece();
					break;
				}
			}
#ifdef _DEBUG
			if (event.type == sfml::event::MouseButtonPressed && event.mouseButton.button == sfml::mouse::Right)
			{
				// display debug info
				m_game_pointer->debug_info(os);
				break;
			}
			else if (event.type == sfml::event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::E)
				{
					// re-evaluate the game
					log << "Manually re-evaluating the game" << std::endl;
					m_game_pointer->set_selected_piece(nullptr);
					m_game_pointer->set_selected(false);
					int dummy = 0;
					m_game_pointer->set_available_capture(m_game_pointer->evaluate(m_game_pointer->get_game_state()->get_current_player()->get_list(), m_game_pointer->get_board(), &dummy, dummy, m_game_pointer->get_last_capture_direction(), m_game_pointer->get_to_delete_list(), m_game_pointer->get_moving_piece()));
					os << "Game re-evaluated" << std::endl;
					break;
				}
				else if (event.key.code == sf::Keyboard::T)
				{
					// write a separator
					os << "--------------------------------------------------------------" << std::endl;
					break;
				}
				else if (event.key.code == sf::Keyboard::U && ((sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) || (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))))
				{
					// add new piece to the first player
					std::pair<int, int> coords = m_game_pointer->get_click_coordinates();
					int x = std::get<0>(coords);
					int y = std::get<1>(coords);
					if (!(x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1) && (x % 2 == 0 && y % 2 != 0 || x % 2 != 0 && y % 2 == 0) && (*m_game_pointer->get_board())[x][y] == nullptr)
					{
						log << "Manually adding king to the first player at coordinates: x: " << x << "; " << y << std::endl;
						m_game_pointer->add_new_piece(m_game_pointer->get_player_1()->get_list(), m_game_pointer->get_board(), m_game_pointer->get_player_1(), x, y, true, true, m_game_pointer->get_gui());
					}
					break;
				}
				else if (event.key.code == sf::Keyboard::I && ((sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) || (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))))
				{
					// add new piece to the second player
					std::pair<int, int> coords = m_game_pointer->get_click_coordinates();
					int x = std::get<0>(coords);
					int y = std::get<1>(coords);
					if (!(x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1) && (x % 2 == 0 && y % 2 != 0 || x % 2 != 0 && y % 2 == 0) && (*m_game_pointer->get_board())[x][y] == nullptr)
					{
						log << "Manually adding king to the second player at coordinates: x: " << x << "; " << y << std::endl;
						m_game_pointer->add_new_piece(m_game_pointer->get_player_2()->get_list(), m_game_pointer->get_board(), m_game_pointer->get_player_2(), x, y, true, true, m_game_pointer->get_gui());
						m_game_pointer->get_game_state()->set_any_changes(true);
					}
					break;
				}
				else if (event.key.code == sf::Keyboard::U)
				{
					// add new piece to the first player
					std::pair<int, int> coords = m_game_pointer->get_click_coordinates();
					int x = std::get<0>(coords);
					int y = std::get<1>(coords);
					if (!(x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1) && (x % 2 == 0 && y % 2 != 0 || x % 2 != 0 && y % 2 == 0) && (*m_game_pointer->get_board())[x][y] == nullptr)
					{
						log << "Manually adding piece to the first player at coordinates: x: " << x << "; " << y << std::endl;
						m_game_pointer->add_new_piece(m_game_pointer->get_player_1()->get_list(), m_game_pointer->get_board(), m_game_pointer->get_player_1(), x, y, true, m_game_pointer->get_gui());
						m_game_pointer->get_game_state()->set_any_changes(true);
					}
					break;
				}
				else if (event.key.code == sf::Keyboard::I)
				{
					// add new piece to the second player
					std::pair<int, int> coords = m_game_pointer->get_click_coordinates();
					int x = std::get<0>(coords);
					int y = std::get<1>(coords);
					if (!(x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1) && (x % 2 == 0 && y % 2 != 0 || x % 2 != 0 && y % 2 == 0) && (*m_game_pointer->get_board())[x][y] == nullptr)
					{
						log << "Manually adding piece to the second player at coordinates: x: " << x << "; " << y << std::endl;
						m_game_pointer->add_new_piece(m_game_pointer->get_player_2()->get_list(), m_game_pointer->get_board(), m_game_pointer->get_player_2(), x, y, true, m_game_pointer->get_gui());
						m_game_pointer->get_game_state()->set_any_changes(true);
					}
					break;
				}
				else if (event.key.code == sf::Keyboard::D)
				{
					// add new piece to the second player
					std::pair<int, int> coords = m_game_pointer->get_click_coordinates();
					int x = std::get<0>(coords);
					int y = std::get<1>(coords);
					os << "x: " << x << "; y: " << y << std::endl;
					if (!(x < 0 || x > s_size - 1 || y < 0 || y > s_size - 1) && (x % 2 == 0 && y % 2 != 0 || x % 2 != 0 && y % 2 == 0) && (*m_game_pointer->get_board())[x][y] != nullptr)
					{
						piece* p = (*m_game_pointer->get_board())[x][y];
						if (p)
						{
							log << "Manually deleting piece at coordinates: x: " << x << "; " << y << std::endl;
							m_game_pointer->delete_piece(p, m_game_pointer->get_board(), p->get_owner());
							m_game_pointer->get_game_state()->set_any_changes(true);
						}
					}
					break;
				}
				else if (event.key.code == sf::Keyboard::L)
				{
					log << "Manually populating the board with normal setup" << std::endl;
					m_game_pointer->populate_board(s_size / 2 - 1);
					m_game_pointer->get_game_state()->set_any_changes(true);
					break;
				}
				else if (event.key.code == sf::Keyboard::F)
				{
					// load pieces from file
					log << "Manually loading gamestate from file" << std::endl;
					m_game_pointer->load_pieces_from_file("pieces.txt");
					m_game_pointer->get_game_state()->set_any_changes(true);
					break;
				}
				else if (event.key.code == sf::Keyboard::J)
				{
					// save pieces to file
					log << "Manually saving gamestate to file" << std::endl;
					m_game_pointer->save_to_file("pieces.txt");
					break;
				}
				else if (event.key.code == sf::Keyboard::S)
				{
					log << "Manually switching game turn" << std::endl;
					m_game_pointer->get_game_state()->switch_turn();
					break;
				}
				else if (event.key.code == sf::Keyboard::R)
				{
					// refresh the board
					os << "Refreshing the board." << std::endl;
					m_game_pointer->get_game_state()->set_any_changes(true);
					break;
				}
				else if (event.key.code == sf::Keyboard::X)
				{
					os << "Frozen the game." << std::endl;
					m_game_pointer->get_game_state()->set_game_freeze(true);
					break;
				}
				else if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift)
				{
					break;
				}
				else
				{
					os << "Key code: \'" << event.key.code << "\' is not programmed." << std::endl;
					break;
				}
			}
#endif
		}
	}
}