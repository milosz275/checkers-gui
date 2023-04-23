#include "include/piece.h"

namespace checkers
{
    piece::piece(char sign, int x, int y) : m_sign(sign), m_x(x), m_y(y), m_is_king(false), m_av_list(new std::list<available_move*>), m_shape(radius)
    {
        switch (m_sign)
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
        }
    }

    piece::~piece() {}

    int piece::get_x(void) { return m_x; }

    int piece::get_y(void) { return m_y; }

    int piece::set_x(int x) { return m_x = x; }

    int piece::set_y(int y) { return m_y = y; }

    char piece::get_sign(void) { return m_sign; }

    std::list<available_move*>* piece::get_av_list(void) { return m_av_list; }

    std::ostream& operator<<(std::ostream& os, const piece* piece)
    {
        if (piece == NULL)
            return os << " ";
        else
            return os << piece->m_sign;
    }

    void piece::draw(sf::RenderWindow& window)
    {
        // update location
        m_shape.setPosition(sf::Vector2f(m_x * s_square_size + (s_square_size - radius * 2) / 2, m_y * s_square_size + (s_square_size - 2 * radius) / 2));
        // draw on the board
        window.draw(m_shape);
    }
}