#include "include/gui.h"
#include "include/piece.h"
#include "include/available_move.h"

namespace checkers
{
	gui::gui(int fps) : m_fps(fps), m_window(sf::VideoMode(s_square_size * s_size, s_square_size * s_size), "Checkers", sf::Style::Default, m_settings), m_tiles(s_size, std::vector<sf::RectangleShape>(s_size, sf::RectangleShape())), m_clock(), m_event(), m_settings()
	{
		if (m_fps < 1)
			throw std::runtime_error("Fps cannot be less than 1");

		// SFML setup, using original methods in camelCase style instead of snake_case
		m_settings.antialiasingLevel = 8;
		m_window.setFramerateLimit(m_fps);
		m_window.setVerticalSyncEnabled(true);

		// fill in the checkers board
		for_each(m_tiles.begin(), m_tiles.end(), [i = 0](std::vector<sf::RectangleShape>& row) mutable
			{
				for_each(row.begin(), row.end(), [&i, j = 0](sf::RectangleShape& tile) mutable
					{
						tile.setSize(sf::Vector2f(s_square_size, s_square_size));
						tile.setPosition(sf::Vector2f(s_square_size * i, s_square_size * j));
						if ((i + j) % 2 == 0)
							tile.setFillColor(sf::Color(193, 173, 158, 255));
						else
							tile.setFillColor(sf::Color(133, 94, 66, 255));
						++j;
					});
				++i;
			});
	}

	void gui::draw_board(std::list<piece*>& list_1, std::list<piece*>& list_2, std::list<piece*>& dead_list, piece* selected_piece)
	{
		m_window.clear();
		draw();

		// highlight selected piece and its corresponding moves, when moves exist
		if (selected_piece != nullptr)
		{
			highlight_selected(selected_piece->get_x(), selected_piece->get_y());
			for_each(selected_piece->get_av_list()->begin(), selected_piece->get_av_list()->end(), [this](available_move* a) { highlight_available(a->get_x(), a->get_y()); });
		}

		// print alive pieces
		for_each(list_1.begin(), list_1.end(), [this](piece* p) { p->draw(m_window); });
		for_each(list_2.begin(), list_2.end(), [this](piece* p) { p->draw(m_window); });

		// print dead pieces in multicapture
		for_each(dead_list.begin(), dead_list.end(), [this](piece* p) { p->draw(m_window); });

		// sleep according to fps
		sf::Time elapsed_time = m_clock.restart();
		if (elapsed_time.asSeconds() < m_frame_duration)
			sf::sleep(sf::seconds(m_frame_duration - elapsed_time.asSeconds()));

		m_window.display();
	}

	void gui::draw(void)
	{
		for_each(m_tiles.begin(), m_tiles.end(), [this](auto row)
			{
				for_each(row.begin(), row.end(), [this](auto tile)
					{
						m_window.draw(tile);
					});
			});
	}

	void gui::highlight_selected(int x, int y)
	{
		sf::RectangleShape selected_tile;
		selected_tile.setSize(sf::Vector2f(s_square_size, s_square_size));
		selected_tile.setFillColor(sf::Color(173, 134, 106, 255));
		selected_tile.setPosition(sf::Vector2f(s_square_size * x, s_square_size * y));
		m_window.draw(selected_tile);
	}

	void gui::highlight_available(int x, int y)
	{
		sf::RectangleShape available_tile;
		available_tile.setSize(sf::Vector2f(s_square_size, s_square_size));
		available_tile.setFillColor(sf::Color(103, 194, 106, 255));
		available_tile.setPosition(sf::Vector2f(s_square_size * x, s_square_size * y));
		m_window.draw(available_tile);
	}

	sf::Event& gui::get_event(void) { return m_event; }

	sf::Window& gui::get_window(void) { return m_window; }

	sf::Clock& gui::get_clock(void) { return m_clock; }

	std::pair<int, int> gui::get_click_coordinates(void)
	{
		int x = sf::Mouse::getPosition(m_window).x / (m_window.getSize().x / s_size);
		int y = sf::Mouse::getPosition(m_window).y / (m_window.getSize().y / s_size);
		return std::make_pair(x, y);
	}
}