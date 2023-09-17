#ifndef	EVALUATOR_H
#define EVALUATOR_H

#include "game_state.h"

namespace checkers
{
	class evaluator
	{
		//
		game_state* m_game_state;
		// evaluate possible moves of one piece
		bool evaluate_piece(piece* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive);
		// evaluate possible moves of one king
		bool evaluate_piece(king* p, std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list);
		//
		std::ostream& m_os;

	public:
		//
		evaluator(game_state* game_state, std::ostream& os);
		//
		~evaluator();
		// clears available moves list for every piece in pieces list (gets through lists in list)
		void clear_list(std::list<piece*>* list);
		// evaluate all possible moves of the given player, returns true if there is at least on possible capture
		bool evaluate(std::list<piece*>* list, std::vector<std::vector<piece*>>* board, int* counter, int recursive, int last_capture_direction, std::list<piece*>* dead_list, piece* moving_piece);
	};
}

#endif