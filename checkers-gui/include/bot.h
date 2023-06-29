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
		// considered steps ahead
		int m_depth;
		// pointer to the game
		game* m_game;
		// anti-loop field reseting, when a piece is selected
		int m_counter_select;
		// anti-loop field reseting, when a piece is moved
		int m_counter_move;
		// x and y coordinates of save planned move after selection
		std::pair<int, int> m_saved_move;
	public:
		// creates bot player with its corresponding sign and depth for simulation
		bot(char sign, int depth);
		// deletes the bot
		~bot();
		// returns the game, the bot is operating on
		game* get_game(void);
		// sets and returns the game, the bot is operating on
		game* set_game(game* game);
		// calculates coordinates of piece to move and coordinates where to go after selecting 
		std::pair<int, int> get_coordinates(void);
		// returns coordinates of best possible move for all combinations of game_copy within given conditions
		std::pair<std::pair<int, int>, std::pair<int, int>> find_best_move(game* game_copy, int depth, int min_score, int max_score, bool is_maximizing);
		// returns vector of games and corresponding moves with the highest score, uses find_best_move recursively to move pieces to the given depth
		std::vector<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>> find_best_games(game* game_copy, int depth, int alpha, int beta, bool maximizing_player);
		// recursively runs through the given game_copy and adds game after one turn to the given list (goes through all possible captures till no more possible, or goes through all possible moves)
		void add_to_game_copy_list(std::list<std::tuple<game*, std::pair<int, int>, std::pair<int, int>>>& list_of_games, game* game_copy, std::pair<int, int>* src_coords, std::pair<int, int>* dest_coords);
	};
}

#endif