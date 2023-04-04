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
#include "AvailableCapture.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>


namespace Checkers
{
	// xnor gate
	bool xnor(bool A, bool B);

	// map containing letters and their corresponding integers
	static std::map<char, int> coords{ {'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}, {'e', 5}, {'f', 6}, {'g', 7}, {'h', 8}, {'i', 9}, {'j', 10} };

	class Game
	{
		// main game board
		std::vector<std::vector<Piece*>>* board;
		// flag indicating turn of the first player
		bool first_turn;
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
		// pointer to selected piece on the board
		Piece* selected_piece;
		// flag indicating if there is one or more captures, not allowing other moves
		bool available_capture;

		// square/tile size
		float square_size = 75.0;

		// sfml fields
		sf::ContextSettings settings;
		sf::RenderWindow window;
		sf::Event event;

	public:
		// board size
		static const int size = 10;
		// create the game with s size of the board
		Game(int s);
		// deletes the game
		~Game();
		// rotates the vector of vectors board, sets the is rotated flag to opposite
		//void rotate_board(void);
		void switch_turn(void);
		// returns main game board
		std::vector<std::vector<Piece*>>* get_board(void);
		// executes the game
		void loop(void);
		// prints result to given stream
		void print_results(std::ostream& os = std::cout);

		void print_pieces(std::list<Piece*>* list, std::ostream& os = std::cout);

		// draws main game board in the given window
		void draw(sf::RenderWindow& window);
		// highlights selected piece of given coords (brown)
		void highlight_selected(sf::RenderWindow& window, int x, int y);
		// higlight selected piece of given coords (green)
		void highlight_available(sf::RenderWindow& window, int x, int y);

		// evaluate possible moves of a player starting on the bottom of the board (first)
		bool evaluate(std::list<Piece*> list, std::vector<std::vector<Piece*>>* board_p, int* counter);
		// evaluate possible moves of a player starting on the top of the board (second)
		bool evaluate_inv(std::list<Piece*> list, std::vector<std::vector<Piece*>>* board_p, int* counter);
		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<Piece*>* list);

		// deletes given piece from given list
		void delete_from_list(std::list<Piece*>* list, Piece* piece_to_delete);

	};
	// returns board to the stream
	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<Piece*>>* board);
}

#endif
