#include "BasePlayer.h"

char BasePlayer::get_sign(void) { return sign; }

std::string BasePlayer::get_name(void) { return name; }

bool BasePlayer::check_if_possible_move(int x_p, int y_p, int x, int y)
{
	

	return false;
}

void BasePlayer::add_piece(void) { pieces++; }

int BasePlayer::get_pieces(void) { return pieces; }

int BasePlayer::get_captured_pieces(void) { return captured_pieces; }

void BasePlayer::capture(void) { pieces--; captured_pieces++; }

void BasePlayer::print_player(void) { std::cout << name << "; sign: " << sign << "; pieces: " << pieces << "; captured pieces: " << captured_pieces << std::endl; }

BasePlayer::BasePlayer(char s, std::string n) : sign(s), name(n), pieces(0), captured_pieces(0) {}

BasePlayer::~BasePlayer() {}