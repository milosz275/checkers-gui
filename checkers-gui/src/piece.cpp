#include "include/piece.h"
#include "include/king.h"

namespace checkers
{
    piece::piece(char sign, int x, int y, bool is_alive, base_player* owner) : m_sign(std::toupper(sign)), m_x(x), m_y(y), m_is_alive(is_alive), m_owner(owner), m_av_list(new std::list<available_move*>), m_shape(s_radius)
    {
        // owner cannot be empty
        assert(m_owner); 

        // signs has to properly initialized
        assert(m_owner->get_sign() == m_sign); 

        if (m_is_alive)
        {
            if (m_sign == 'W')
                m_shape.setFillColor(sf::Color(217, 216, 216, 255));
            else if (m_sign == 'B')
                m_shape.setFillColor(sf::Color(26, 23, 22, 255));
            else
                throw std::runtime_error("Alive piece creation: sign not supported");
        }
        else
        {
            if (m_sign == 'W')
                m_shape.setFillColor(sf::Color(187, 186, 186, 255));
            else if (m_sign == 'B')
                m_shape.setFillColor(sf::Color(59, 59, 59, 255));
            else
                throw std::runtime_error("Dead piece creation: sign not supported");
        }

        /*switch (m_sign)
        {
        case 'W':
            m_shape.setFillColor(sf::Color(217, 216, 216, 255));
            break;
        case 'B':
            m_shape.setFillColor(sf::Color(26, 23, 22, 255));
            break;
        case 'w':
            m_shape.setFillColor(sf::Color(187, 186, 186, 255));
            break;
        case 'b':
            m_shape.setFillColor(sf::Color(59, 59, 59, 255));
            break;
        default:
            throw std::runtime_error("Wrong piece sign");
        }*/
    }

    piece::piece(const piece& piece) : m_sign(piece.m_sign), m_x(piece.m_x), m_y(piece.m_y), m_is_alive(piece.m_is_alive), m_owner(nullptr), m_av_list(new std::list<available_move*>), m_shape(s_radius) {}

    piece::~piece() {}

    int piece::get_x(void) { return m_x; }

    int piece::get_y(void) { return m_y; }

    int piece::set_x(int x) { return m_x = x; }

    int piece::set_y(int y) { return m_y = y; }

    char piece::get_sign(void) { return m_sign; }

    bool piece::is_alive(void) { return m_is_alive; }

    base_player* piece::get_owner(void) { return m_owner; }

    base_player* piece::set_owner(base_player* owner) { assert(!m_owner); return m_owner = owner; }

    std::list<available_move*>* piece::get_av_list(void) { return m_av_list; }

    std::ostream& operator<<(std::ostream& os, const piece* piece)
    {
        if (piece)
            return os << piece->m_sign;
        else
            return os << " ";
    }

    void piece::draw(sf::RenderWindow& window)
    {
        // update location
        m_shape.setPosition(sf::Vector2f(m_x * s_square_size + (s_square_size - s_radius * 2) / 2, m_y * s_square_size + (s_square_size - 2 * s_radius) / 2));
        // draw on the board
        window.draw(m_shape);
    }
}