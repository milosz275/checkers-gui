#ifndef BOT_H
#define BOT_H

#include "include/base_player.h"
#include "include/game.h"
#include <cassert>

namespace checkers
{
	class game;

	class bot : public base_player
	{
		// level of bot's "intelligence"
		int m_depth;
		// pointer to the game
		const game* m_game;
		// x coordinate of planned move
		int m_x;
		// y coordinate of planned move
		int m_y;
		// anti-loop field reseting, when a piece is selected
		int m_counter_select;
		// anti-loop field reseting, when a piece is moved
		int m_counter_move;
	public:
		//
		bot(char sign, const game* game);
		//
		bot(const bot& bot);
		//
		~bot();
		//
		std::tuple<int, int> get_coordinates(void);
		//
		std::tuple<int, int> find_best_move(game* game_copy);
		//
		void add_to_game_copy_list(std::list<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>>& list_of_games, game* game_copy, std::pair<int, int>* source_coords, std::pair<int, int>* destination_coords);
		//
		int minimax(game* game_copy, int depth, int alpha, int beta, int is_maximizing);
	};
}

#endif