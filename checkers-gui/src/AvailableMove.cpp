#include "AvailableMove.h"

AvailableMove::AvailableMove(int x1, int y1) : x(x1), y(y1) {}

int AvailableMove::get_x(void) { return x; }

int AvailableMove::get_y(void) { return y; }