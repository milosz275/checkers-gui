#include "AvailableMove.h"

AvailableMove::AvailableMove(int x1, int y1, bool capt) : x(x1), y(y1), capture(capt) {}

AvailableMove::~AvailableMove() {}

int AvailableMove::get_x(void) { return x; }

int AvailableMove::get_y(void) { return y; }

bool AvailableMove::is_capture(void) { return capture; }