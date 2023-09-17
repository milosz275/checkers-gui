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
		double m_frame_duration = 1.0f / m_fps;
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
		const double m_square_size;
		// radius of one piece
		const double m_radius;
	protected:
		// executes drawing
		void draw(void);
		// highlights selected field of given coordinates
		void highlight_selected(int x, int y);
		// highlights available field of given coordinates
		void highlight_available(int x, int y);
	public:
		// creates graphical user interface refreshing with given fps
		gui(int fps);
		// returns event 
		sfml::event& get_event(void);
		// returns window
		sfml::window& get_window(void);
		// returns clock
		sfml::clock& get_clock(void);
		// returns size of field
		double get_square_size(void);
		// returns radius of piece
		double get_radius(void);
		// returns coordinates of performed click
		std::pair<int, int> get_click_coordinates(void);
		// draws the board
		void draw_board(std::list<piece*>& list_1, std::list<piece*>& list_2, std::list<piece*>& dead_list, piece* selected_piece);
		// draws the piece
		void draw_piece(sfml::circle_shape& shape, int x, int y);
	};
}

#endif
