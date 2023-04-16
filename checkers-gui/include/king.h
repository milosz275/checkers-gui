#ifndef KING_H
#define KING_H

#include "include/piece.h"

namespace checkers
{
	class king : public piece
	{

	public:
		//
		king(char sign, int x, int y);
		//
		~king();
	};
}

#endif
