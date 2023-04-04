#ifndef PLAYER_H
#define PLAYER_H

#include "BasePlayer.h"
#include <map>

// map containing letters and their corresponding integers
static std::map<char, int> coords{ {'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}, {'e', 5}, {'f', 6}, {'g', 7}, {'h', 8}, {'i', 9}, {'j', 10} };

class Player : public BasePlayer
{

public:
	bool move(void);
	// creates the player of given sign and name
	Player(char s, std::string n);
	// deletes the player
	~Player();
};

#endif