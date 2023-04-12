#include "AvailableCapture.h"

namespace Checkers
{
	AvailableCapture::AvailableCapture(int x, int y, int x_d, int y_d) : AvailableMove(x, y, true), m_x_d(x_d), m_y_d(y_d) {}

	int AvailableCapture::get_x_d(void) { return m_x_d; }

	int AvailableCapture::get_y_d(void) { return m_y_d; }
}