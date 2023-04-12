#include "include/base_player.h"

namespace checkers
{
	base_player::base_player(char sign, std::string name) : m_sign(sign), m_name(name), m_pieces(0), m_captured_pieces(0), m_combo(false) {}

	base_player::~base_player() {}

	char base_player::get_sign(void) { return m_sign; }

	std::string base_player::get_name(void) { return m_name; }

	bool base_player::get_combo(void) { return m_combo; }

	bool base_player::set_combo(bool combo) { return m_combo = combo; }

	void base_player::add_piece(int count) { m_pieces += count; }

	int base_player::get_pieces(void) { return m_pieces; }

	int base_player::get_captured_pieces(void) { return m_captured_pieces; }

	void base_player::capture(void) { m_pieces--; m_captured_pieces++; }

	void base_player::print_player(void) { std::cout << m_name << "; sign: " << m_sign << "; pieces: " << m_pieces << "; captured pieces: " << m_captured_pieces << std::endl; }
}