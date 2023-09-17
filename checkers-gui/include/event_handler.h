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
		// pointer to the game
		game* m_game_pointer;

	public:
		// creates event handler for given game pointer
		event_handler(game* game_pointer);
		// deletes the event handler
		~event_handler();
		// handles all events
		void handle_events(void);
	};
}

#endif