#include "player.h"

namespace checkers
{
	player::player(char sign, std::string name, std::function<std::pair<int, int>()> get_coords) : base_player(sign, name), m_get_coordinates_from_game(get_coords) {}

	player::player(const player& player) : base_player(player) { m_get_coordinates_from_game = player.m_get_coordinates_from_game; }

	player::~player() {}

	std::pair<int, int> player::get_coordinates(void) { return m_get_coordinates_from_game(); }
}