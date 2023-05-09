#ifndef PLAYER_H
#define PLAYER_H

#include "include/base_player.h"
#include <map>
#include <functional>

namespace checkers
{
	class player : public base_player
	{
		// pointer to game get_coordinates function
		std::function<std::tuple<int, int>()> m_get_coordinates_from_game;
	public:
		// creates the player of given sign and name
		player(char sign, std::string name, std::function<std::tuple<int, int>()> get_coords);
		// deletes the player
		~player();
		//
		std::tuple<int, int> get_coordinates(void);
	};
}

#endif