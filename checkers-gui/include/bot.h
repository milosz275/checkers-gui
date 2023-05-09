#ifndef BOT_H
#define BOT_H

#include "include/base_player.h"
#include "include/game.h"

namespace checkers
{
	class game;

	class bot : public base_player
	{
		//
		int m_considered_moves_ahead;
		//
		game* m_game;
		// x coordinate of planned move
		int m_x;
		// y coordinate of planned move
		int m_y;
	public:
		//
		bot(char sign, game* game);
		//
		~bot();
		//
		std::tuple<int, int> get_coordinates(void);
	};
}

#endif