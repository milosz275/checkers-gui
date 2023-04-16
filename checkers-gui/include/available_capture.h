#ifndef AVAILABLE_CAPTURE_H
#define AVAILABLE_CAPTURE_H

#include "include/available_move.h"

namespace checkers
{
	class available_capture : public available_move
	{
		// x coordinate of a piece to delete making a capture
		int m_x_d;
		// y coordinate of a piece to delete making a capture
		int m_y_d;
	public:
		available_capture(int x, int y, int x_d, int y_d);
		// returns x coordinate of a piece to delete making a capture
		int get_x_d(void);
		// returns y coordinate of a piece to delete making a capture
		int get_y_d(void);
	};
}

#endif