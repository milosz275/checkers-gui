#ifndef SFML_H
#define SFML_H

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

namespace sfml
{
	// snake_case aliases for SFML classes
	using window = sf::Window;
	using render_window = sf::RenderWindow;
	using event = sf::Event;
	using clock = sf::Clock;
	using keyboard = sf::Keyboard;
	using color = sf::Color;
	using rectangle_shape = sf::RectangleShape;
	using circle_shape = sf::CircleShape;
	using video_mode = sf::VideoMode;
	using vector_2f = sf::Vector2f;
	using mouse = sf::Mouse;
	using time = sf::Time;
	using context_settings = sf::ContextSettings;
}

#endif
