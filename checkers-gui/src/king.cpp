#include "include/king.h"

namespace checkers
{
    king::king(char sign, int x, int y) : piece(sign, x, y, true)
    {
        m_shape.setOutlineColor(sf::Color(232, 27, 16, 255));
        m_shape.setOutlineThickness(s_radius / 12.5);
    }

    king::~king() {}
}

