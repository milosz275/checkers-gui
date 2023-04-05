#ifndef AVAILABLECAPTURE_H
#define AVAILABLECAPTURE_H

#include <list>
#include "AvailableMove.h"

namespace Checkers
{
	class AvailableCapture : public AvailableMove
	{
		// x coordinate of a piece to delete making a capture
		int x_d;
		// y coordinate of a piece to delete making a capture
		int y_d;
	public:
		AvailableCapture(int x1, int y1, int x_d1, int y_d1);
		// returns x coordinate of a piece to delete making a capture
		int get_x_d(void);
		// returns y coordinate of a piece to delete making a capture
		int get_y_d(void);
	};
}

#endif