#include "AvailableCapture.h"

AvailableCapture::AvailableCapture(int x1, int y1, int x_d1, int y_d1) : AvailableMove(x1, y1, true), x_d(x_d1), y_d(y_d1) {}

int AvailableCapture::get_x_d(void) { return x_d; }

int AvailableCapture::get_y_d(void) { return y_d; }