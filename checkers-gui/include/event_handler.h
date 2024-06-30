#ifndef EVENT_HANDER_H
#define EVENT_HANDER_H

#include "sfml.h"

namespace checkers
{
	class piece;
	class king;
	class game;
	
	/**
	 * @brief Class representing event handler
	 * 
	 * This class is used to handle all events in the game
	 * 
	 * @see event_handler::event_handler
	 * 		- constructor
	 * @see event_handler::~event_handler
	 * 		- destructor
	 * @see event_handler::handle_events
	 * 		- handles all events
	 */
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
