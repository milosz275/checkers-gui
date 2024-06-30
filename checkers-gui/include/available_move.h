#ifndef AVAILABLEMOVE_H
#define AVAILABLEMOVE_H

#include <list>

namespace checkers
{
	class piece;

	/**
	 * @brief Class representing available move
	 * 
	 * This class is used to store information about available move in checkers game
	 * 
	 * @see available_move::available_move
	 * 		- constructor
	 * @see available_move::get_x
	 * 		- returns x coordinate of available move
	 * @see available_move::get_y
	 * 		- returns y coordinate of available move
	 */
	class available_move
	{
		// x coordinate of available move
		int m_x;
		// y coordinate of available move
		int m_y;
	public:
		// creates available move
		available_move(int x, int y);
		// deletes the move
		virtual ~available_move();
		// returns x coordinate of available move
		int get_x(void);
		// returns y coordinate of available move
		int get_y(void);
	};
}

#endif
