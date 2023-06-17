#include "include/gui.h"
#include "include/piece.h"
#include "include/available_move.h"

namespace checkers
{
	//sf::Color hsl_to_rgb(float hue, float saturation, float lightness);
	//sf::Vector3f rgb_to_hsl(sf::Color color);

	gui::gui(int fps) : m_fps(fps), m_square_size(sf::VideoMode::getDesktopMode().height * 0.8 / s_size), m_radius(m_square_size / 2.5), m_clock(), m_event(), m_settings()
	{
		if (m_fps < 1)
			throw std::runtime_error("Fps cannot be less than 1");
		std::cout << m_square_size << std::endl;
		
		m_window = new sf::RenderWindow(sf::VideoMode(m_square_size * s_size, m_square_size * s_size), "Checkers", sf::Style::Default, m_settings);
		m_tiles = new std::vector<std::vector<sf::RectangleShape>>(s_size, std::vector<sf::RectangleShape>(s_size, sf::RectangleShape()));

		// SFML setup, using original methods in camelCase style instead of snake_case
		m_settings.antialiasingLevel = 8;
		m_window->setFramerateLimit(m_fps);
		m_window->setVerticalSyncEnabled(true);

		// fill in the checkers board
		for_each(m_tiles->begin(), m_tiles->end(), [i = 0, this](std::vector<sf::RectangleShape>& row) mutable
			{
				for_each(row.begin(), row.end(), [&i, j = 0, this](sf::RectangleShape& tile) mutable
					{
						tile.setSize(sf::Vector2f(m_square_size, m_square_size));
						tile.setPosition(sf::Vector2f(m_square_size * i, m_square_size * j));
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
		m_window->clear();
		//chroma_effect(list_1);
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
		sf::Time elapsed_time = m_clock.restart();
		if (elapsed_time.asSeconds() < m_frame_duration)
			sf::sleep(sf::seconds(m_frame_duration - elapsed_time.asSeconds()));

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

	/*void gui::chroma_effect(std::list<piece*>& list)
	{
		for_each(list.begin(), list.end(), [this, i = 0](piece* p) mutable
			{
				int x = p->get_x();
				int y = p->get_y();
				sf::CircleShape& shape = p->get_shape();
				sf::Color current_color = shape.getFillColor();
				sf::Vector3f hsl_values = RGBToHSL(current_color);
				float hue = hsl_values.x;
				float saturation = hsl_values.y;
				float lightness = hsl_values.z;
				sf::Color new_color = HSLToRGB(hue + 1, saturation, lightness);
				shape.setFillColor(new_color);
				++i;
			});
	}*/

	void gui::highlight_selected(int x, int y)
	{
		sf::RectangleShape selected_tile;
		selected_tile.setSize(sf::Vector2f(m_square_size, m_square_size));
		selected_tile.setFillColor(sf::Color(173, 134, 106, 255));
		selected_tile.setPosition(sf::Vector2f(m_square_size * x, m_square_size * y));
		m_window->draw(selected_tile);
	}

	void gui::highlight_available(int x, int y)
	{
		sf::RectangleShape available_tile;
		available_tile.setSize(sf::Vector2f(m_square_size, m_square_size));
		available_tile.setFillColor(sf::Color(103, 194, 106, 255));
		available_tile.setPosition(sf::Vector2f(m_square_size * x, m_square_size * y));
		m_window->draw(available_tile);
	}

	sf::Event& gui::get_event(void) { return m_event; }

	sf::Window& gui::get_window(void) { return *m_window; }

	sf::Clock& gui::get_clock(void) { return m_clock; }

	float gui::get_square_size(void) { return m_square_size; }
	
	float gui::get_radius(void) { return m_radius; }

	std::pair<int, int> gui::get_click_coordinates(void)
	{
		int x = sf::Mouse::getPosition(*m_window).x / (m_window->getSize().x / s_size);
		int y = sf::Mouse::getPosition(*m_window).y / (m_window->getSize().y / s_size);
		return std::make_pair(x, y);
	}
	
	void gui::draw_piece(sf::CircleShape& shape, int x, int y)
	{
		// update location
		shape.setPosition(sf::Vector2f(x * m_square_size + (m_square_size - m_radius * 2) / 2, y * m_square_size + (m_square_size - 2 * m_radius) / 2));
		// draw on the board
		m_window->draw(shape);
	}

	//sf::Color hsl_to_rgb(float hue, float saturation, float lightness)
	//{
	//	float chroma = (1 - std::abs(2 * lightness - 1)) * saturation;
	//	float huePrime = hue / 60.0f;
	//	float x = chroma * (1 - std::abs(std::fmod(huePrime, 2) - 1));
	//	float r, g, b;

	//	if (huePrime >= 0 && huePrime < 1) {
	//		r = chroma;
	//		g = x;
	//		b = 0;
	//	}
	//	else if (huePrime >= 1 && huePrime < 2) {
	//		r = x;
	//		g = chroma;
	//		b = 0;
	//	}
	//	else if (huePrime >= 2 && huePrime < 3) {
	//		r = 0;
	//		g = chroma;
	//		b = x;
	//	}
	//	else if (huePrime >= 3 && huePrime < 4) {
	//		r = 0;
	//		g = x;
	//		b = chroma;
	//	}
	//	else if (huePrime >= 4 && huePrime < 5) {
	//		r = x;
	//		g = 0;
	//		b = chroma;
	//	}
	//	else {
	//		r = chroma;
	//		g = 0;
	//		b = x;
	//	}

	//	float lightnessMatch = lightness - chroma / 2.0f;
	//	r += lightnessMatch;
	//	g += lightnessMatch;
	//	b += lightnessMatch;

	//	return sf::Color(static_cast<sf::Uint8>(r * 255), static_cast<sf::Uint8>(g * 255), static_cast<sf::Uint8>(b * 255));
	//}

	//sf::Vector3f rgb_to_hsl(sf::Color color)
	//{
	//	float r = color.r / 255.0f;
	//	float g = color.g / 255.0f;
	//	float b = color.b / 255.0f;

	//	float maxComponent = std::max(r, std::max(g, b));
	//	float minComponent = std::min(r, std::min(g, b));

	//	float hue, saturation, lightness;
	//	hue = saturation = lightness = (maxComponent + minComponent) / 2.0f;

	//	if (maxComponent == minComponent) {
	//		hue = saturation = 0; // achromatic
	//	}
	//	else {
	//		float delta = maxComponent - minComponent;
	//		saturation = lightness > 0.5f ? delta / (2.0f - maxComponent - minComponent) : delta / (maxComponent + minComponent);

	//		if (maxComponent == r) {
	//			hue = (g - b) / delta + (g < b ? 6.0f : 0.0f);
	//		}
	//		else if (maxComponent == g) {
	//			hue = (b - r) / delta + 2.0f;
	//		}
	//		else if (maxComponent == b) {
	//			hue = (r - g) / delta + 4.0f;
	//		}

	//		hue /= 6.0f;
	//	}

	//	return sf::Vector3f(hue, saturation, lightness);
	//}
}