#include "include/game.h"
// The game is based on: https://en.wikipedia.org/wiki/International_draughts
// project uses snake_case name convention, however SFML is the exception as an external library

int main(int argc, char* argv[])
{
	// todo: make use argc and argv
	try
	{
		checkers::game checkers;
		checkers.loop();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}