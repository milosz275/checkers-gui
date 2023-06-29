#include "king.h"

namespace checkers
{
    king::king(int x, int y, bool is_alive, base_player* owner) : piece(x, y, is_alive, owner) {}

    king::king(int x, int y, bool is_alive, base_player* owner, gui* ui) : piece(x, y, is_alive, owner, ui) { setup_shape(); }

    king::~king() {}

    void king::setup_shape(void)
    {
        if (m_gui)
        {
            m_shape->setOutlineColor(sfml::color(232, 27, 16, 255));
            m_shape->setOutlineThickness(m_gui->get_radius() / 12.5);
        }
    }
}

