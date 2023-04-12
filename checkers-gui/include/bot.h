#ifndef BOT_H
#define BOT_H

#include "include/base_player.h"

namespace checkers
{
	class bot : public base_player
	{

	public:
		//
		bot(char sign);
		//
		~bot();
	};
}

#endif