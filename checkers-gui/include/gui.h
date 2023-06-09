#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <list>

#include "global_variables.h"
#include "sfml.h"

namespace checkers
{
	class piece;
	class gui
	{
		// target frames per second
		int m_fps;
		// fraction of second that is time of one frame
		float m_frame_duration = 1.0f / m_fps;
		// SFML object drawing checkerboard
		std::vector<std::vector<sf::RectangleShape>> m_tiles;
		// SFML clock
		sf::Clock m_clock;
		// SFML settings
		sf::ContextSettings m_settings;
		// SFML window
		sf::RenderWindow m_window;
		// SFML event
		sf::Event m_event;
	protected:
		//
		void draw(void);
		//
		void highlight_selected(int x, int y);
		//
		void highlight_available(int x, int y);
		//// applies chroma effect to the given piece list
		//void chroma_effect(std::list<piece*>& list);
	public:
		//
		gui(int fps);
		//
		sf::Event& get_event(void);
		//
		sf::Window& get_window(void);
		//
		sf::Clock& get_clock(void);
		//
		std::pair<int, int> get_click_coordinates(void);
		//
		void draw_board(std::list<piece*>& list_1, std::list<piece*>& list_2, std::list<piece*>& dead_list, piece* selected_piece);
	};
}

#endif