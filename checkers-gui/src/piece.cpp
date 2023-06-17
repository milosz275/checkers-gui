#include "include/piece.h"
#include "include/king.h"
#include "include/gui.h"

namespace checkers
{
    piece::piece(char sign, int x, int y, bool is_alive, base_player* owner) : m_sign(std::toupper(sign, std::locale())), m_x(x), m_y(y), m_is_alive(is_alive), m_owner(owner), m_av_list(new std::list<available_move*>)
    {
        // owner cannot be empty
        assert(m_owner); 

        // signs has to properly initialized
        assert(m_owner->get_sign() == m_sign); 
    }

    piece::piece(char sign, int x, int y, bool is_alive, base_player* owner, gui* gui) : piece(sign, x, y, is_alive, owner)
    {
        m_gui = gui;
        if (m_gui)
        {
            m_shape = new sf::CircleShape(m_gui->get_radius());
            setup_shape();
        }
    }

    piece::piece(const piece& piece) : m_sign(piece.m_sign), m_x(piece.m_x), m_y(piece.m_y), m_is_alive(piece.m_is_alive), m_owner(nullptr), m_av_list(new std::list<available_move*>)
    {
        if (m_gui)
            m_shape = new sf::CircleShape(m_gui->get_radius());
    }

    piece::~piece() {}

    void piece::setup_shape(void)
    {
        if (m_is_alive)
        {
            if (m_sign == 'W')
                m_shape->setFillColor(sf::Color(217, 216, 216, 255));
            else if (m_sign == 'B')
                m_shape->setFillColor(sf::Color(26, 23, 22, 255));
            else
                throw std::runtime_error("Alive piece creation: sign not supported");
        }
        else
        {
            if (m_sign == 'W')
                m_shape->setFillColor(sf::Color(187, 186, 186, 255));
            else if (m_sign == 'B')
                m_shape->setFillColor(sf::Color(59, 59, 59, 255));
            else
                throw std::runtime_error("Dead piece creation: sign not supported");
        }
    }

    int piece::get_x(void) { return m_x; }

    int piece::get_y(void) { return m_y; }

    int piece::set_x(int x) { return m_x = x; }

    int piece::set_y(int y) { return m_y = y; }

    char piece::get_sign(void) { return m_sign; }

    bool piece::is_alive(void) { return m_is_alive; }

    base_player* piece::get_owner(void) { return m_owner; }

    base_player* piece::set_owner(base_player* owner) { assert(!m_owner); return m_owner = owner; }

    sf::CircleShape& piece::get_shape(void) { return *m_shape; }

    std::list<available_move*>* piece::get_av_list(void) { return m_av_list; }

    gui* piece::get_gui(void) { return m_gui; }

    std::ostream& operator<<(std::ostream& os, const piece* piece)
    {
        if (piece)
            return os << piece->m_sign;
        else
            return os << " ";
    }

    void piece::draw(void)
    {
        assert(m_gui);
        m_gui->draw_piece(*m_shape, m_x, m_y);
    }
}