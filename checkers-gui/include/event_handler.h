#ifndef EVENT_HANDER_H
#define EVENT_HANDER_H

#include "sfml.h"

namespace checkers
{
	class piece;
	class king;
	class game;
	class event_handler
	{
		//
		game* m_game_pointer;
	public:
		//
		void handle_events(void);
		//
		event_handler(game* game_pointer);
		//
		~event_handler();
	};
}

#endif