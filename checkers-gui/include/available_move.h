#ifndef AVAILABLEMOVE_H
#define AVAILABLEMOVE_H

#include <list>

namespace checkers
{
	class available_move
	{
		// x coordinate of available move
		int m_x;
		// y coordinate of available move
		int m_y;
		// flag indicating if the move will be a capture -> please use AvailableCapture if so;
		bool m_capture;
	public:
		// creates available move
		available_move(int x, int y, bool capture = false);
		// deletes the move
		virtual ~available_move();
		// returns x coordinate of available move
		int get_x(void);
		// returns y coordinate of available move
		int get_y(void);
		// returns true, if the move is a capture move
		bool is_capture(void);
	};
}

#endif