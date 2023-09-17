#include "base_player.h"
#include "piece.h"
#include "king.h"

namespace checkers
{
	base_player::base_player(char sign, std::string name) : m_sign(std::toupper(sign, std::locale())), m_name(name), m_captured_pieces(0), m_combo(false), m_piece_list(nullptr), m_is_first(false)
	{
		assert(std::isalpha(sign, std::locale()));
	}

	base_player::base_player(const base_player& player) : m_sign(player.m_sign), m_name(player.m_name), m_captured_pieces(player.m_combo), m_combo(player.m_combo), m_piece_list(nullptr), m_is_first(player.m_is_first) {}

	base_player::~base_player() {}

	char base_player::get_sign(void) { return m_sign; }

	std::string base_player::get_name(void) { return m_name; }

	bool base_player::is_first(void) { return m_is_first; }
	
	bool base_player::set_first(bool is_first) { return m_is_first = is_first; }

	std::list<piece*>* base_player::get_list(void) { return m_piece_list; }

	std::list<piece*>* base_player::set_list(std::list<piece*>* piece_list) { assert(!m_piece_list); return m_piece_list = piece_list; }

	bool base_player::get_combo(void) { return m_combo; }

	bool base_player::set_combo(bool combo) { return m_combo = combo; }

	int base_player::get_pieces(void) { return (int)m_piece_list->size(); }

	int base_player::get_captured_pieces(void) { return m_captured_pieces; }

	int base_player::get_kings(void)
	{
		int count = 0;
		for_each(m_piece_list->begin(), m_piece_list->end(), [&count](piece* p) { if (dynamic_cast<king*>(p)) ++count; });
		return count;
	}

	int base_player::set_captured_pieces(int pieces)
	{
#ifndef _DEBUG
		assert(m_captured_pieces == 0);
#endif
		return m_captured_pieces = pieces;
	}

	void base_player::add_capture(void) { assert(m_piece_list->size() >= 0); m_captured_pieces++; }

	void base_player::print_player(std::ostream& os) { os << m_name << "; sign: " << m_sign << "; pieces: " << m_piece_list->size() << "; captured pieces: " << m_captured_pieces << std::endl; }

	bool base_player::change_to_king(piece* target, std::vector<std::vector<piece*>>* board) // std::list<piece*>* list
	{
		// make sure the list is not set
		assert(target->get_owner()->get_list() != nullptr);

		// make sure the list is not empty
		assert(!target->get_owner()->get_list()->empty());

		// cannot change dead pieces
		assert(target->is_alive());

		// add asertion if this is not the current player and if has no captures

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
			return false;

		// change the piece to king
		int x = target->get_x();
		int y = target->get_y();
		(*board)[x][y] = nullptr;
		target->get_owner()->get_list()->remove(target);
		
		(*board)[x][y] = new king(x, y, true, this, target->get_gui());
		target->get_owner()->get_list()->push_back((*board)[x][y]);

		return true;
	}

	std::ostream& operator<<(std::ostream& os, const base_player* player)
	{
		os << "name: " << player->m_name << "; sign: " << player->m_sign << "; alive pieces: " << player->m_piece_list->size() << "(" << player->m_piece_list->size() << ")" << "; dead pieces: " << player->m_captured_pieces;
		os << "; combo: ";
		player->m_combo ? os << "true" : os << "false";
		os << "; is first player: ";
		player->m_is_first ? os << "true" << std::endl : os << "false" << std::endl;
		return os;
	}
}