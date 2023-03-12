#ifndef BASEPLAYER_H
#define BASEPLAYER_H
#include <iostream>
#include <string>
#include <algorithm>
#include <list>

class BasePlayer
{
	char sign;
	std::string name;
	int pieces;
	int captured_pieces;
public:

	char get_sign(void);
	std::string get_name(void);
	// checks if the piece at given coordinates can move
	bool check_if_possible_move(int x_p, int y_p);
	// checks if the given coordinates are allowed to go: x_p, y_p: coordinates of the piece; x,y: coordinates of target place
	bool check_if_possible_move(int x_p, int y_p, int x, int y);
	virtual bool move(void) = 0;
	void add_piece(void);
	void print_player(void);
	BasePlayer(char s, std::string n);
	~BasePlayer();
};

#endif