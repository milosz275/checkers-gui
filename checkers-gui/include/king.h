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
		// deletes the king
		~king();
	};
}

#endif
