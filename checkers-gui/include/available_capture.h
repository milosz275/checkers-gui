#ifndef AVAILABLECAPTURE_H
#define AVAILABLECAPTURE_H

#include <list>
#include "available_move.h"

namespace checkers
{
	class available_capture : public available_move
	{
		// x coordinate of a piece to delete making a capture
		int m_x_d;
		// y coordinate of a piece to delete making a capture
		int m_y_d;
		// integer representing maximal possible recursive capture
		int m_max_score;

	public:
		// creates new capture move with given goto, delete coords and max score of evaluated multicapture
		available_capture(int x, int y, int x_d, int y_d, int max_score);
		// returns x coordinate of a piece to delete making a capture
		int get_x_d(void);
		// returns y coordinate of a piece to delete making a capture
		int get_y_d(void);
		// returns maximal possible capture score after multicapture
		int get_max_score(void);
	};
}

#endif
