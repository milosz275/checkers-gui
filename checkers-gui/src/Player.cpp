#include "Player.h"

namespace Checkers
{
	Player::Player(char s, std::string n) : BasePlayer(s, n)
	{

	}

	Player::~Player()
	{

	}

	bool Player::move(void)
	{
		//// return true if no possible moves

		//// coordinates
		//int x, y;
		//bool goodCoordinates = false;
		//while (!goodCoordinates)
		//{
		//	// coordinates
		//	std::string coordinates;
		//	std::cout << "Give coordinates of the shot (ex. A1): ";
		//	std::getline(std::cin, coordinates);
		//	if (coordinates.size() > 3 || coordinates.size() < 2)
		//	{
		//		std::cout << "Wrong coordinates length. Try again" << std::endl;
		//		continue;
		//	}

		//	// getting first coordinate
		//	transform(coordinates.begin(), coordinates.end(), coordinates.begin(), tolower);
		//	char first_sign = coordinates[0];
		//	int tmp = coords[first_sign];
		//	if (tmp < 1 || tmp > 10)
		//	{
		//		std::cout << "Wrong first coordinates supplied: " << first_sign << std::endl;
		//		std::cout << "Try again" << std::endl;
		//		continue;
		//	}
		//	x = tmp - 1;

		//	// getting second coordinate
		//	char second_sign = coordinates[1];
		//	tmp = second_sign - '0';
		//	if ((coordinates.size() == 3 && second_sign != '1') || (coordinates.size() == 2 && (tmp < 1 || tmp > 10)))
		//	{
		//		std::cout << "Wrong second coordinates supplied: " << second_sign << coordinates[2] << std::endl;
		//		std::cout << "Try again" << std::endl;
		//		continue;
		//	}
		//	y = tmp - 1;

		//	////
		//	//if (check_if_possible_move(x, y))
		//	//	goodCoordinates = true;
		//	
		//	// check if given coordinates are able to move
		//}


		return false;
	}
}