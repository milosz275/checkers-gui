#include "include/king.h"

namespace checkers
{
    king::king(char sign, int x, int y, base_player* owner) : piece(sign, x, y, owner)
    {
        m_shape.setOutlineColor(sf::Color(232, 27, 16, 255));
        m_shape.setOutlineThickness(s_radius / 12.5);
    }

    king::~king() {}
}

