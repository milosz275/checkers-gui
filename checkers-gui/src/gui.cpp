#include "gui.h"
#include "piece.h"
#include "available_move.h"

namespace checkers
{
	gui::gui(int fps) : m_fps(fps), m_square_size(sfml::video_mode::getDesktopMode().height * 0.8 / s_size), m_radius(m_square_size / 2.5), m_clock(), m_event(), m_settings()
	{
		if (m_fps < 1)
			throw std::runtime_error("Fps cannot be less than 1");
		
		m_window = new sfml::render_window(sfml::video_mode((int)(m_square_size * s_size), (int)(m_square_size * s_size)), "Checkers", sf::Style::Default, m_settings);
		m_tiles = new std::vector<std::vector<sfml::rectangle_shape>>(s_size, std::vector<sfml::rectangle_shape>(s_size, sfml::rectangle_shape()));

		// SFML setup, using original methods in camelCase style instead of snake_case
		m_settings.antialiasingLevel = 8;
		m_window->setFramerateLimit(m_fps);
		m_window->setVerticalSyncEnabled(true);

		// fill in the checkers board
		for_each(m_tiles->begin(), m_tiles->end(), [i = 0, this](std::vector<sfml::rectangle_shape>& row) mutable
			{
				for_each(row.begin(), row.end(), [&i, j = 0, this](sfml::rectangle_shape& tile) mutable
					{
						tile.setSize(sfml::vector_2f((float)m_square_size, (float)m_square_size));
						tile.setPosition(sfml::vector_2f((float)(m_square_size * i), (float)(m_square_size * j)));
						if ((i + j) % 2 == 0)
							tile.setFillColor(sfml::color(193, 173, 158, 255));
						else
							tile.setFillColor(sfml::color(133, 94, 66, 255));
						++j;
					});
				++i;
			});
	}

	void gui::draw_board(std::list<piece*>& list_1, std::list<piece*>& list_2, std::list<piece*>& dead_list, piece* selected_piece)
	{
		m_window->clear();
		draw();

		// highlight selected piece and its corresponding moves, when moves exist
		if (selected_piece != nullptr)
		{
			highlight_selected(selected_piece->get_x(), selected_piece->get_y());
			for_each(selected_piece->get_av_list()->begin(), selected_piece->get_av_list()->end(), [this](available_move* a) { highlight_available(a->get_x(), a->get_y()); });
		}

		// print alive pieces
		for_each(list_1.begin(), list_1.end(), [this](piece* p) { p->draw(); });
		for_each(list_2.begin(), list_2.end(), [this](piece* p) { p->draw(); });

		// print dead pieces in multicapture
		for_each(dead_list.begin(), dead_list.end(), [this](piece* p) { p->draw(); });

		// sleep according to fps
		sfml::time elapsed_time = m_clock.restart();
		if (elapsed_time.asSeconds() < m_frame_duration)
			sf::sleep(sf::seconds((float)(m_frame_duration - elapsed_time.asSeconds())));

		m_window->display();
	}

	void gui::draw(void)
	{
		for_each(m_tiles->begin(), m_tiles->end(), [this](auto row)
			{
				for_each(row.begin(), row.end(), [this](auto tile)
					{
						m_window->draw(tile);
					});
			});
	}

	void gui::highlight_selected(int x, int y)
	{
		sfml::rectangle_shape selected_tile;
		selected_tile.setSize(sfml::vector_2f((float)m_square_size, (float)m_square_size));
		selected_tile.setFillColor(sfml::color(173, 134, 106, 255));
		selected_tile.setPosition(sfml::vector_2f((float)(m_square_size * x), (float)(m_square_size * y)));
		m_window->draw(selected_tile);
	}

	void gui::highlight_available(int x, int y)
	{
		sfml::rectangle_shape available_tile;
		available_tile.setSize(sfml::vector_2f((float)m_square_size, (float)m_square_size));
		available_tile.setFillColor(sfml::color(103, 194, 106, 255));
		available_tile.setPosition(sfml::vector_2f((float)(m_square_size * x), (float)(m_square_size * y)));
		m_window->draw(available_tile);
	}

	sfml::event& gui::get_event(void) { return m_event; }

	sfml::window& gui::get_window(void) { return *m_window; }

	sfml::clock& gui::get_clock(void) { return m_clock; }

	double gui::get_square_size(void) { return m_square_size; }
	
	double gui::get_radius(void) { return m_radius; }

	std::pair<int, int> gui::get_click_coordinates(void)
	{
		int x = sfml::mouse::getPosition(*m_window).x / (m_window->getSize().x / s_size);
		int y = sfml::mouse::getPosition(*m_window).y / (m_window->getSize().y / s_size);
		return std::make_pair(x, y);
	}
	
	void gui::draw_piece(sfml::circle_shape& shape, int x, int y)
	{
		// update location
		shape.setPosition(sfml::vector_2f((float)(x * m_square_size + (m_square_size - m_radius * 2) / 2), (float)(y * m_square_size + (m_square_size - 2 * m_radius) / 2)));
		// draw on the board
		m_window->draw(shape);
	}
}
