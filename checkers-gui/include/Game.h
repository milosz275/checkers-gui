#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include "BasePlayer.h"
#include "Player.h"
#include "Bot.h"
#include "Piece.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>


namespace Checkers
{
	static std::map<char, int> coords{ {'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}, {'e', 5}, {'f', 6}, {'g', 7}, {'h', 8}, {'i', 9}, {'j', 10} };

	class Game
	{
		//board
		std::vector<std::vector<Piece*>>* board;

		bool is_rotated;
		// player 1
		BasePlayer* player_1;
		// player 2
		BasePlayer* player_2;
		// piece list of player 1
		std::list<Piece*> p_list_1;
		// piece list of player 2
		std::list<Piece*> p_list_2;
		// flag indicating finished game
		bool is_finished;
		// flag indicating that first player won
		bool first_won;
		// flag indicating that second player won
		bool second_won;
		//

		// square/tile size
		float square_size = 75.0;

		sf::ContextSettings settings;
		sf::RenderWindow window;
		sf::Event event;


		Piece* selected_piece;

	public:
		// board size
		static const int size = 10;

		Game(int s);

		~Game();

		void rotate_board(void);

		std::vector<std::vector<Piece*>>* get_board(void);

		void loop(void);

		void print_results(void);



		void draw(sf::RenderWindow& window);

		void highlight_selected(sf::RenderWindow& window, int x, int y);

		void highlight_available(sf::RenderWindow& window, int x, int y);


		void evaluate(std::list<Piece*>* list);
		
	};

	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<Piece*>>* board);
}

#endif
