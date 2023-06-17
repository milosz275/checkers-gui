#ifndef KING_H
#define KING_H

#include "include/piece.h"

namespace checkers
{
	class king : public piece
	{
	public:
		// creates the king
		king(char sign, int x, int y, bool is_alive, base_player* owner);
		//
		king(char sign, int x, int y, bool is_alive, base_player* owner, gui* gui);
		// deletes the king
		~king();
		//
		void setup_shape(void);
	};
}

#endif
