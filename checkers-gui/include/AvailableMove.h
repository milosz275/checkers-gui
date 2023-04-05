#ifndef AVAILABLEMOVE_H
#define AVAILABLEMOVE_H

#include <list>

namespace Checkers
{
	class AvailableMove
	{
		// x coordinate of available move
		int x;
		// y coordinate of available move
		int y;
		// flag indicating if the move will be a capture -> please use AvailableCapture if so;
		bool capture;
	public:
		// creates available move
		AvailableMove(int x1, int y1, bool capt = false);
		// deletes the move
		virtual ~AvailableMove();
		// returns x coordinate of available move
		int get_x(void);
		// returns y coordinate of available move
		int get_y(void);
		// returns true, if the move is a capture move
		bool is_capture(void);
	};
}

#endif