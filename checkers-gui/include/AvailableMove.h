#ifndef AVAILABLEMOVE_H
#define AVAILABLEMOVE_H

#include <list>

class AvailableMove
{
	int x;
	int y;
public:
	AvailableMove(int x1, int y1);
	int get_x(void);
	int get_y(void);
};

#endif