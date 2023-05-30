#ifndef GLOBAL_H
#define GLOBAL_H

#include <map>

namespace checkers
{
	// board size
	static const int s_size = 10;
	// map containing letters and their corresponding integers
	static std::map<char, int> s_coords{ {'a', 1}, { 'b', 2 }, { 'c', 3 }, { 'd', 4 }, { 'e', 5 }, { 'f', 6 }, { 'g', 7 }, { 'h', 8 }, { 'i', 9 }, { 'j', 10 } };
	// square/tile size
	static const float s_square_size = 100.0;
	// radius of one piece
	static const float s_radius = s_square_size / 2.5;
}

#endif