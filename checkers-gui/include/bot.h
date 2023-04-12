#ifndef BOT_H
#define BOT_H

#include "BasePlayer.h"

namespace Checkers
{
	class Bot : public BasePlayer
	{

	public:
		//
		Bot(char sign);
		//
		~Bot();
	};
}

#endif