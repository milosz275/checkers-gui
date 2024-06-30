#ifndef KING_H
#define KING_H

#include "piece.h"

namespace checkers
{
	/**
	 * @brief Class representing king piece
	 * 
	 * This class is derived from piece and represents king piece in checkers game
	 * 
	 * @see king::king
	 * 		- constructor
	 * @see king::~king
	 * 		- destructor
	 * @see king::setup_shape
	 * 		- setups outer outline of the king
	 */
	class king : public piece
	{
	public:
		// creates the king
		king(int x, int y, bool is_alive, base_player* owner);
		// creates the king and adds to gui
		king(int x, int y, bool is_alive, base_player* owner, gui* gui);
		// deletes the king
		~king();
		// setups outer outline of the king
		void setup_shape(void);
	};
}

#endif
