#ifndef PLAYER_H
#define PLAYER_H

#include "base_player.h"
#include <map>
#include <functional>

namespace checkers
{
	/**
	 * @brief Class representing player
	 * 
	 * This class is derived from base_player and represents player in checkers game
	 * 
	 * @see player::player
	 * 		- constructor
	 * @see player::player
	 * 		- copy constructor
	 * @see player::~player
	 * 		- destructor
	 * @see player::get_coordinates
	 * 		- method proxy using game (gui or stream) to get coordinates
	 */
	class player : public base_player
	{
		// pointer to game get_coordinates function
		std::function<std::pair<int, int>()> m_get_coordinates_from_game;
	public:
		// creates the player of given sign and name
		player(char sign, std::string name, std::function<std::pair<int, int>()> get_coords);
		// copies the player
		player(const player& player);
		// deletes the player
		~player();
		// method proxy using game (gui or stream) to get coordinates 
		std::pair<int, int> get_coordinates(void);
	};
}

#endif
