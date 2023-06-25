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
		std::vector<std::vector<sfml::rectangle_shape>>* m_tiles = nullptr;
		// SFML clock
		sfml::clock m_clock;
		// SFML settings
		sfml::context_settings m_settings;
		// SFML window
		sfml::render_window* m_window = nullptr;
		// SFML event
		sfml::event m_event;
		// square/tile size
		const float m_square_size;
		// radius of one piece
		const float m_radius;
	protected:
		// executes drawing
		void draw(void);
		// 
		void highlight_selected(int x, int y);
		//
		void highlight_available(int x, int y);
	public:
		//
		gui(int fps);
		//
		sfml::event& get_event(void);
		//
		sfml::window& get_window(void);
		//
		sfml::clock& get_clock(void);
		//
		float get_square_size(void);
		//
		float get_radius(void);
		//
		std::pair<int, int> get_click_coordinates(void);
		//
		void draw_board(std::list<piece*>& list_1, std::list<piece*>& list_2, std::list<piece*>& dead_list, piece* selected_piece);
		//
		void draw_piece(sfml::circle_shape& shape, int x, int y);
	};
}

#endif