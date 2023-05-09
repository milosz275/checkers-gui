#include "include/player.h"

namespace checkers
{
	player::player(char sign, std::string name, std::function<std::tuple<int, int>()> get_coords) : base_player(sign, name), m_get_coordinates_from_game(get_coords) {}

	player::~player() {}

	std::tuple<int, int> player::get_coordinates(void) { return m_get_coordinates_from_game(); }
}