#include "Piece.h"

namespace Checkers
{
    Piece::Piece(char s, int x1, int y1) : sign(s), x(x1), y(y1), is_captured(false), is_king(false), av_list(new std::list<AvailableMove*>) {}

    Piece::~Piece() {}

    int Piece::get_x(void) { return x; }

    int Piece::get_y(void) { return y; }

    int Piece::set_x(int x1) { return x = x1; }

    int Piece::set_y(int y1) { return y = y1; }

    char Piece::get_sign(void) { return sign; }

    bool Piece::set_captured(bool t) { return is_captured = t; }

    bool Piece::get_is_captured(void) { return is_captured; }

    std::list<AvailableMove*>* Piece::get_av_list(void) { return av_list; }

    std::ostream& operator<<(std::ostream& os, const Piece* piece)
    {
        if (piece == NULL)
            return os << " ";
        else
            return os << piece->sign;
    }

    void Piece::draw(sf::RenderWindow& window)
    {
        sf::CircleShape shape(radius);

        switch (sign)
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

        shape.setPosition(sf::Vector2f(x * square_size + (square_size - radius * 2) / 2, y * square_size + (square_size - 2 * radius) / 2));
        window.draw(shape);
    }
}