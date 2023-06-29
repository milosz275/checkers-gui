#include "include/game.h"
// The game is based on: https://en.wikipedia.org/wiki/International_draughts
// Project uses snake_case name convention, however SFML is the exception as an external library

int main(int argc, char* argv[])
{
	// TODO: make use argc and argv
	try
	{
		checkers::game* checkers = new checkers::game();
		checkers->loop();
		delete checkers;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}