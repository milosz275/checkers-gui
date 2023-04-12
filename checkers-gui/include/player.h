#ifndef PLAYER_H
#define PLAYER_H

#include "include/base_player.h"
#include <map>

namespace checkers
{
	class player : public base_player
	{

	public:
		// creates the player of given sign and name
		player(char sign, std::string name);
		// deletes the player
		~player();
	};
}

#endif