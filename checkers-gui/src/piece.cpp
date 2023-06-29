#include "piece.h"
#include "king.h"
#include "gui.h"

namespace checkers
{
    piece::piece(int x, int y, bool is_alive, base_player* owner) : m_x(x), m_y(y), m_is_alive(is_alive), m_av_list(new std::list<available_move*>)
    {
        // owner init
        m_owner = owner;
        assert(m_owner); 
    }

    piece::piece(int x, int y, bool is_alive, base_player* owner, gui* gui) : piece(x, y, is_alive, owner)
    {
        m_gui = gui;
        if (m_gui)
        {
            m_shape = new sfml::circle_shape(m_gui->get_radius());
            setup_shape();
        }
    }

    piece::piece(const piece& piece) : m_x(piece.m_x), m_y(piece.m_y), m_is_alive(piece.m_is_alive), m_owner(nullptr), m_av_list(new std::list<available_move*>)
    {
        if (m_gui)
            m_shape = new sfml::circle_shape(m_gui->get_radius());
    }

    piece::~piece() {}

    void piece::setup_shape(void)
    {
        char sign = m_owner->get_sign();
        if (m_is_alive)
        {
            if (sign == 'W')
                m_shape->setFillColor(sfml::color(217, 216, 216, 255));
            else if (sign == 'B')
                m_shape->setFillColor(sfml::color(26, 23, 22, 255));
            else
                throw std::runtime_error("Alive piece creation: sign not supported");
        }
        else
        {
            if (sign == 'W')
                m_shape->setFillColor(sfml::color(187, 186, 186, 255));
            else if (sign == 'B')
                m_shape->setFillColor(sfml::color(59, 59, 59, 255));
            else
                throw std::runtime_error("Dead piece creation: sign not supported");
        }
    }

    int piece::get_x(void) { return m_x; }

    int piece::get_y(void) { return m_y; }

    int piece::set_x(int x) { return m_x = x; }

    int piece::set_y(int y) { return m_y = y; }

    bool piece::is_alive(void) { return m_is_alive; }

    base_player* piece::get_owner(void) { return m_owner; }

    base_player* piece::set_owner(base_player* owner) { assert(!m_owner); return m_owner = owner; }

    sfml::circle_shape& piece::get_shape(void) { return *m_shape; }

    std::list<available_move*>* piece::get_av_list(void) { return m_av_list; }

    gui* piece::get_gui(void) { return m_gui; }

    std::ostream& operator<<(std::ostream& os, const piece* piece)
    {
        if (piece)
            return os << piece->m_owner->get_sign();
        else
            return os << " ";
    }

    void piece::draw(void)
    {
        assert(m_gui);
        m_gui->draw_piece(*m_shape, m_x, m_y);
    }
}