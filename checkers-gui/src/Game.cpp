#include "Game.h"
#include <algorithm>

namespace Checkers
{
	Game::Game(int s) : is_finished(false), window(sf::VideoMode(square_size * size, square_size * size), "Checkers", sf::Style::Default, settings), selected_piece(NULL), first_turn(true)
	{
		//board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, NULL));
		board = new std::vector<std::vector<Piece*>>(size, std::vector<Piece*>(size, 0));
		
		// todo: choice
		std::string name_1 = "Some player 1";
		std::string name_2 = "Some player 2";
		player_1 = new Player('W', name_1);
		player_2 = new Player('B', name_2);

		// todo: change to algorithm
		// rows of the second player (upper)
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < size; ++j)
			{
				if ((i + j) % 2 != 0)
				{
					(*board)[j][i] = new Piece(player_2->get_sign(), j, i);
					p_list_2.push_back((*board)[j][i]);
					player_2->add_piece();
				}
			}
		}
		// rows of the first player (lower)
		for (int i = size - 1; i >= size - 3; --i)
		{
			for (int j = 0; j < size; ++j)
			{
				if ((i + j) % 2 != 0)
				{
					(*board)[j][i] = new Piece(player_1->get_sign(), j, i);
					p_list_1.push_back((*board)[j][i]);
					player_1->add_piece();
				}
			}
		}

		// set the antialiasing
		settings.antialiasingLevel = 2.0;

		// evaluate available moves for the first player
		evaluate_first();
	}

	std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<Piece*>>* board)
	{
		os << "\t  ";
		for (char a = 'a'; a < 'a' + Game::size; ++a)
			os << a << "   ";
		os << std::endl << std::endl << std::endl;
		for (int i = 0; i < Game::size; ++i)
		{
			os << Game::size - i << "\t| ";
			for (int j = 0; j < Game::size; ++j)
			{
				os << (*board)[i][j] << " | ";
			}
			os << std::endl << std::endl;
		}
		return os;
	}

	Game::~Game()
	{
		delete board;
		delete player_1;
		delete player_2;
	}

	void Game::switch_turn(void) { first_turn = !first_turn; }

	void Game::loop(void)
	{
		bool selected = false;
		//rotate_board();

		// main loop
		while (window.isOpen())
		{
			// 24 fps
			sf::sleep(sf::seconds(1.0f / 24));

			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();

				if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
				{
					if (selected_piece != NULL) // choice after highlighting
					{
						int x = sf::Mouse::getPosition(window).x / square_size;
						int y = sf::Mouse::getPosition(window).y / square_size;
						
						// find corresponding piece
						bool is_found = false;
						for_each(selected_piece->get_av_list()->begin(), selected_piece->get_av_list()->end(), [&x, &y, &is_found](AvailableMove* a)
						{
							if (a->get_x() == x && a->get_y() == y)
							{
								std::cout << a->get_x() << " " << a->get_y() << std::endl;
								is_found = true;
							}
						});
						if (!is_found)
						{
							selected = false;
							selected_piece = NULL;
						}
						else
						{
							// move the piece
							(*board)[selected_piece->get_x()][selected_piece->get_y()] = NULL;
							selected_piece->set_x(x);
							selected_piece->set_y(y);
							(*board)[x][y] = selected_piece;
							selected_piece = NULL;
							selected = false;
							switch_turn();

							// evaluate opposite player
							if (first_turn)
							{
								evaluate_first();
								clear_list(&p_list_2);
							}
							else
							{
								evaluate_second();
								clear_list(&p_list_1);
							}
						}
					}
					else
						selected = !selected;
				}
			}

			window.clear();
			draw(window);

			// first choice, nothing is already highlighted
			if (selected)
			{
				selected_piece = NULL;

				int x = sf::Mouse::getPosition(window).x / square_size;
				int y = sf::Mouse::getPosition(window).y / square_size;
				
				// if the correspoding field contains a piece
				if ((*board)[x][y] != NULL)
				{
					std::cout << "x: " << x << "; y: " << y << "; piece: " << (*board)[x][y] << std::endl;
					if (!(*board)[x][y]->get_av_list()->empty())
						for_each((*board)[x][y]->get_av_list()->begin(), (*board)[x][y]->get_av_list()->end(), [](AvailableMove* a) { std::cout << "available: x: " << a->get_x() << "; y: " << a->get_y() << std::endl; });
					selected_piece = (*board)[x][y];
				}
				else
				{
					std::cout << "x: " << x << "; y: " << y << std::endl;
					selected_piece = NULL;
				}
				
				/*if ((*board)[x][y] != NULL)
					selected_piece = (*board)[x][y];
				else
					selected_piece = NULL;*/

				selected = false;
			}

			// highlight selected piece and its corresponding moves, when moves exist
			if (selected_piece != NULL)
			{
				if (!(selected_piece->get_av_list()->empty()))
				{
					highlight_selected(window, selected_piece->get_x(), selected_piece->get_y());
					for_each(selected_piece->get_av_list()->begin(), selected_piece->get_av_list()->end(), [this](AvailableMove* a) { highlight_available(window, a->get_x(), a->get_y()); });
				}
				else
					selected_piece = NULL;
			}

			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < size; ++j)
				{
					if ((*board)[i][j] != NULL)
						(*board)[i][j]->draw(window);
				}
			}

			


			window.display();
		}


		while (!is_finished)
		{
			// move of the first player
			//system("cls");

			is_finished = player_1->move();
			std::cout << board << std::endl;
			
			// evaluate player 2
			if (is_finished) // break if finished
				break;

			// move of the second player
			//system("cls");
			is_finished = player_2->move();
			std::cout << board << std::endl;
			
			// evaluate player 1

			// tmp
			player_1->print_player();
			player_2->print_player();

			is_finished = true;
		}
		//print_results();

		std::cout << board << std::endl;

		player_1->print_player();
		player_2->print_player();

		while (!p_list_1.empty())
		{
			std::cout << "sign: " << p_list_1.front()->get_sign() << "; x: " << p_list_1.front()->get_x() << "; y: " << p_list_1.front()->get_y() << std::endl;
			while (!(p_list_1.front()->get_av_list()->empty()))
			{
				std::cout << "available move - x: " << p_list_1.front()->get_av_list()->front()->get_x() << "; y: " << p_list_1.front()->get_av_list()->front()->get_y() << std::endl;
				p_list_1.front()->get_av_list()->pop_front();
			}
			p_list_1.pop_front();
		}

		while (!p_list_2.empty())
		{
			std::cout << "sign: " << p_list_2.front()->get_sign() << "; x: " << p_list_2.front()->get_x() << "; y: " << p_list_2.front()->get_y() << std::endl;
			p_list_2.pop_front();
		}
	}

	std::vector<std::vector<Piece*>>* Game::get_board(void) { return board; }

	/*void Game::rotate_board(void)
	{
		is_rotated = !is_rotated;
		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < size - i; ++j)
			{
				Piece* tmp = (*board)[i][j];
				(*board)[i][j] = (*board)[size - 1 - i][size - 1 - j];
				(*board)[size - 1 - i][size - 1 - j] = tmp;

				if (i == j)
				{
					Piece* tmp = (*board)[i][size - 1 - i];
					(*board)[i][size - 1 - i] = (*board)[size - 1 - i][i];
					(*board)[size - 1 - i][i] = tmp;
				}
			}
		}
	}*/

	void Game::print_results(void)
	{
		if (!first_won && !second_won)
			std::cout << "Game wasn't finished" << std::endl;
		else if (first_won && second_won)
			std::cout << "Draw" << std::endl;
		else if (first_won)
			std::cout << "Player: \"" << player_1->get_name() << "\" won!" << std::endl;
		else
			std::cout << "Player: \"" << player_2->get_name() << "\" won!" << std::endl;
	}


	void Game::draw(sf::RenderWindow& window)
	{
		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(square_size, square_size));
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				tile.setPosition(sf::Vector2f(square_size * i, square_size * j));
				if ((i + j) % 2 == 0)
				{
					tile.setFillColor(sf::Color(193, 173, 158, 255));
				}
				else
				{
					tile.setFillColor(sf::Color(133, 94, 66, 255));
				}
				window.draw(tile);
			}
		}
	}

	void Game::highlight_selected(sf::RenderWindow& window, int x, int y)
	{
		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(square_size, square_size));
		tile.setFillColor(sf::Color(173, 134, 106, 255));
		tile.setPosition(sf::Vector2f(square_size * x, square_size * y));
		window.draw(tile);
	}

	void Game::highlight_available(sf::RenderWindow& window, int x, int y)
	{
		sf::RectangleShape tile;
		tile.setSize(sf::Vector2f(square_size, square_size));
		tile.setFillColor(sf::Color(103, 194, 106, 255));
		tile.setPosition(sf::Vector2f(square_size * x, square_size * y));
		window.draw(tile);
	}


	void Game::evaluate_first(void)
	{
		for_each(p_list_1.begin(), p_list_1.end(), [this](Piece* p)
			{
				int x = p->get_x();
				int y = p->get_y();

				std::cout << "evaluating" << std::endl;
				std::cout << "x: " << x << "; y: " << y << std::endl;

				if ((*board)[x][y] != NULL)
					std::cout << (*board)[x][y] << std::endl;

				// moves to right
				if (x != size - 1 && y != 0)
				{
					if ((*board)[x + 1][y - 1] == NULL)
					{
						std::cout << "available move to the right!" << std::endl;
						//AvailableMove move(x + 1, y + 1);
						(*p).get_av_list()->push_back(new AvailableMove(x + 1, y - 1));
						//p->get_av_list()->push_back(move);
					}
				}

				// moves to left
				if (x != 0 && y != 0)
				{
					if ((*board)[x - 1][y - 1] == NULL)
					{
						std::cout << "available move to the left!" << std::endl;
						//AvailableMove move(x + 1, y + 1);
						(*p).get_av_list()->push_back(new AvailableMove(x - 1, y - 1));
						//p->get_av_list()->push_back(move);
					}
				}

			});


	}

	void Game::evaluate_second(void)
	{
		for_each(p_list_2.begin(), p_list_2.end(), [this](Piece* p)
			{
				int x = p->get_x();
				int y = p->get_y();

				std::cout << "evaluating" << std::endl;
				std::cout << "x: " << x << "; y: " << y << std::endl;

				if ((*board)[x][y] != NULL)
					std::cout << (*board)[x][y] << std::endl;

				// moves to right
				if (x != size - 1 && y != size - 1)
				{
					if ((*board)[x + 1][y + 1] == NULL)
					{
						std::cout << "available move to the right!" << std::endl;
						//AvailableMove move(x + 1, y + 1);
						(*p).get_av_list()->push_back(new AvailableMove(x + 1, y + 1));
						//p->get_av_list()->push_back(move);
					}
				}

				// moves to left
				if (x != 0 && y != size - 1)
				{
					if ((*board)[x - 1][y + 1] == NULL)
					{
						std::cout << "available move to the left!" << std::endl;
						//AvailableMove move(x + 1, y + 1);
						(*p).get_av_list()->push_back(new AvailableMove(x - 1, y + 1));
						//p->get_av_list()->push_back(move);
					}
				}

			});
	}

	void Game::clear_list(std::list<Piece*>* list)
	{
		for_each(list->begin(), list->end(), [this](Piece* p)
			{
				while (!(p->get_av_list()->empty()))
				{
					p->get_av_list()->pop_front();
				}
			});
	}
}



/*
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

class Piece
{
	int x;
public:
	int get_x(void) { return x; }
	int set_x(int a) { return x = a; }
	Piece() : x(5) {}
	Piece(int a) : x(a) {}
	friend ostream& operator<<(ostream& os, const Piece& p);
};
ostream& operator<<(ostream& os, const Piece& p) { return os << p.x; }

int main(int argc, char* argv[])
{
	vector<vector<Piece*>>* board;
	board = new vector<vector<Piece*>>(10, vector<Piece*>(10, NULL));


	(*board)[1][1] = new Piece(4);
	(*board)[2][2] = new Piece(6);
	list<Piece*> lista;
	lista.push_back((*board)[1][1]);
	lista.back()->set_x(7);


	for_each(board->begin(), board->end(), [](vector<Piece*> v) { for_each(v.begin(), v.end(), [](Piece* p)
		{
			if (p == NULL)
				std::cout << "0; ";
			else
				std::cout << *p << "; ";
		}); std::cout << endl; });

	return 0;
}
*/