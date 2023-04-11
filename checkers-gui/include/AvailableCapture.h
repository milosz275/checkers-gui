#ifndef AVAILABLECAPTURE_H
#define AVAILABLECAPTURE_H

#include <list>
#include "AvailableMove.h"

namespace Checkers
{
	class AvailableCapture : public AvailableMove
	{
		// x coordinate of a piece to delete making a capture
		int m_x_d;
		// y coordinate of a piece to delete making a capture
		int m_y_d;
	public:
		AvailableCapture(int x, int y, int x_d, int y_d);
		// returns x coordinate of a piece to delete making a capture
		int get_x_d(void);
		// returns y coordinate of a piece to delete making a capture
		int get_y_d(void);
	};
}

#endif