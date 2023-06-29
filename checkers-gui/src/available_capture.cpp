#include "available_capture.h"

namespace checkers
{
	available_capture::available_capture(int x, int y, int x_d, int y_d, int max_score) : available_move(x, y), m_x_d(x_d), m_y_d(y_d), m_max_score(max_score) {}

	int available_capture::get_x_d(void) { return m_x_d; }

	int available_capture::get_y_d(void) { return m_y_d; }

	int available_capture::get_max_score(void) { return m_max_score; }
}