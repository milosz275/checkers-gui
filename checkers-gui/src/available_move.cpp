#include "AvailableMove.h"

namespace Checkers
{
	AvailableMove::AvailableMove(int x, int y, bool capture) : m_x(x), m_y(y), m_capture(capture) {}

	AvailableMove::~AvailableMove() {}

	int AvailableMove::get_x(void) { return m_x; }

	int AvailableMove::get_y(void) { return m_y; }

	bool AvailableMove::is_capture(void) { return m_capture; }
}