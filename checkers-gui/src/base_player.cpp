#include "include/base_player.h"
#include "include/piece.h"
#include "include/king.h"

namespace checkers
{
	base_player::base_player(char sign, std::string name) : m_sign(sign), m_name(name), m_pieces(0), m_captured_pieces(0), m_combo(false), m_next_player(NULL), m_piece_list(NULL), m_is_first(false) {}

	base_player::~base_player() {}

	char base_player::get_sign(void) { return m_sign; }

	std::string base_player::get_name(void) { return m_name; }

	bool base_player::is_first(void) { return m_is_first; }
	
	bool base_player::set_first(bool is_first) { return m_is_first = is_first; }

	base_player* base_player::get_next_player(void) { assert(m_next_player != NULL); return m_next_player; }

	base_player* base_player::set_next_player(base_player* next_player) { assert(next_player != this); return m_next_player = next_player; }

	std::list<piece*>* base_player::get_list(void) { return m_piece_list; }

	std::list<piece*>* base_player::set_list(std::list<piece*>* piece_list) { assert(m_piece_list == NULL); return m_piece_list = piece_list; }

	bool base_player::get_combo(void) { return m_combo; }

	bool base_player::set_combo(bool combo) { return m_combo = combo; }

	void base_player::add_piece(int count) { m_pieces += count; }

	int base_player::get_pieces(void) { return m_pieces; }

	int base_player::get_captured_pieces(void) { return m_captured_pieces; }

	void base_player::make_capture(void) { assert(m_pieces > 0); m_pieces--; m_captured_pieces++; }

	void base_player::print_player(void) { std::cout << m_name << "; sign: " << m_sign << "; pieces: " << m_pieces << "; captured pieces: " << m_captured_pieces << std::endl; }

	void base_player::kings(piece* target, std::vector<std::vector<piece*>>* board)
	{
		// make sure the list is not set
		assert(m_piece_list != NULL);

		// make sure the list is not empty
		assert(!m_piece_list->empty());

		// check if any new kings are made
		bool is_king = false;
		if (is_first())
		{
			if (target->get_y() == 0)
				is_king = true;
		}
		else // is second
		{
			if (target->get_y() == s_size - 1)
				is_king = true;
		}
		if (!is_king)
			return;

		//// make sure piece is owned by the player
		//is_king = false;
		//all_of(m_piece_list->begin(), m_piece_list->end(), [&target, &is_king](piece* p)
		//	{
		//		if (p == target)
		//		{
		//			is_king = true;
		//			return false;
		//		}
		//	});
		//assert(is_king); // breaks stuff

		// change the piece to king
		int x = target->get_x();
		int y = target->get_y();
		(*board)[x][y] = NULL;
		m_piece_list->remove(target);

		(*board)[x][y] = new king(get_sign(), x, y);
		m_piece_list->push_back((*board)[x][y]);
	}
}