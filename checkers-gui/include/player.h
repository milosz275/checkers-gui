#ifndef PLAYER_H
#define PLAYER_H

#include "BasePlayer.h"
#include <map>

namespace Checkers
{
	class Player : public BasePlayer
	{

	public:
		// creates the player of given sign and name
		Player(char s, std::string n);
		// deletes the player
		~Player();
	};
}

#endif