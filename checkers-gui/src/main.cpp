#include "Game.h"
// https://en.wikipedia.org/wiki/International_draughts

int main(int argc, char* argv[])
{
	try
	{
		Checkers::Game ch_g(10);
		ch_g.loop();
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}