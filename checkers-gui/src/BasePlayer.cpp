#include "BasePlayer.h"

namespace Checkers
{
	BasePlayer::BasePlayer(char sign, std::string name) : m_sign(sign), m_name(name), m_pieces(0), m_captured_pieces(0), m_combo(false) {}

	BasePlayer::~BasePlayer() {}

	char BasePlayer::get_sign(void) { return m_sign; }

	std::string BasePlayer::get_name(void) { return m_name; }

	bool BasePlayer::get_combo(void) { return m_combo; }

	bool BasePlayer::set_combo(bool combo) { return m_combo = combo; }

	void BasePlayer::add_piece(int count) { m_pieces += count; }

	int BasePlayer::get_pieces(void) { return m_pieces; }

	int BasePlayer::get_captured_pieces(void) { return m_captured_pieces; }

	void BasePlayer::capture(void) { m_pieces--; m_captured_pieces++; }

	void BasePlayer::print_player(void) { std::cout << m_name << "; sign: " << m_sign << "; pieces: " << m_pieces << "; captured pieces: " << m_captured_pieces << std::endl; }
}