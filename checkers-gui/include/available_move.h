#ifndef AVAILABLEMOVE_H
#define AVAILABLEMOVE_H

#include <list>
//#include "piece.h"

namespace checkers
{
	class piece;
	class available_move
	{
		// x coordinate of available move
		int m_x;
		// y coordinate of available move
		int m_y;
		////
		//piece* m_piece = nullptr;
	public:
		// creates available move
		available_move(int x, int y);
		// deletes the move
		virtual ~available_move();
		// returns x coordinate of available move
		int get_x(void);
		// returns y coordinate of available move
		int get_y(void);
		////
		//piece* get_piece(void);
	};
}

#endif