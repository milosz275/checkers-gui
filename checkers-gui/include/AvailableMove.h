#ifndef AVAILABLEMOVE_H
#define AVAILABLEMOVE_H

#include <list>

class AvailableMove
{
	int x;
	int y;
	bool capture;
public:
	AvailableMove(int x1, int y1, bool capt = false);
	virtual ~AvailableMove();
	int get_x(void);
	int get_y(void);
	bool is_capture(void);
};

class AvailableCapture : public AvailableMove
{
	int x_d;
	int y_d;
public:
	AvailableCapture(int x1, int y1, int x_d1, int y_d1) : AvailableMove(x1, y1, true), x_d(x_d1), y_d(y_d1) {}
	int get_x_d(void) { return x_d; }
	int get_y_d(void) { return y_d; }
};

#endif