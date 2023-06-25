#include "include/available_move.h"

namespace checkers
{
	available_move::available_move(int x, int y) : m_x(x), m_y(y) {}

	available_move::~available_move() {}

	int available_move::get_x(void) { return m_x; }

	int available_move::get_y(void) { return m_y; }
}