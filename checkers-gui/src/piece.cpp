#include "include/piece.h"

namespace checkers
{
    piece::piece(char sign, int x, int y) : m_sign(sign), m_x(x), m_y(y), m_is_captured(false), m_is_king(false), m_av_list(new std::list<available_move*>) {}

    piece::~piece() {}

    int piece::get_x(void) { return m_x; }

    int piece::get_y(void) { return m_y; }

    int piece::set_x(int x) { return m_x = x; }

    int piece::set_y(int y) { return m_y = y; }

    char piece::get_sign(void) { return m_sign; }

    bool piece::set_captured(bool captured) { return m_is_captured = captured; }

    bool piece::is_captured(void) { return m_is_captured; }

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
        sf::CircleShape shape(radius);

        switch (m_sign)
        {
        case 'W':
            shape.setFillColor(sf::Color(217, 216, 216, 255));
            break;
        case 'B':
            shape.setFillColor(sf::Color(26, 23, 22, 255));
            break;
        case 'w':
            shape.setFillColor(sf::Color(197, 196, 196, 255));
            break;
        case 'b':
            shape.setFillColor(sf::Color(59, 59, 59, 255));
            break;
        default:
            throw std::runtime_error("Wrong piece sign");
        }

        shape.setPosition(sf::Vector2f(m_x * square_size + (square_size - radius * 2) / 2, m_y * square_size + (square_size - 2 * radius) / 2));
        window.draw(shape);
    }
}