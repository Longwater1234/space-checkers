#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace chk
{
enum class PieceType
{
    Red = 69995,
    Black = 78885,
};

constexpr auto BLACK_NORMAL = "resources/black_normal.png";
constexpr auto BLACK_KING = "resources/black_king.png";
constexpr auto RED_NORMAL = "resources/red_normal.png";
constexpr auto RED_KING = "resources/red_king.png";

class Piece final : public sf::Drawable, public sf::Transformable
{

  public:
    Piece(const sf::CircleShape &circle, const PieceType pType, const unsigned int idx_);
    PieceType getPieceType() const;
    void activateKing();
    bool getIsKing() const;
    void moveCustom(const float posX, const float posY);
    // Custom equality operator
    bool operator==(const Piece &other) const;

  private:
    sf::Texture texture;
    unsigned int index;
    sf::CircleShape myCircle;
    PieceType pieceType;
    bool isKing = false;
    void Piece::draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

inline Piece::Piece(const sf::CircleShape &circle, const PieceType pType, const unsigned int idx_)
{
    this->myCircle = circle;
    this->pieceType = pType;
    this->index = idx_;

    sf::Texture localTxr;
    if (pieceType == PieceType::Red)
    {
        if (localTxr.loadFromFile(RED_NORMAL))
        {
            this->texture = localTxr;
            this->myCircle.setTexture(&this->texture);
        }
    }
    else
    {
        if (localTxr.loadFromFile(BLACK_NORMAL))
        {
            this->texture = localTxr;
            this->myCircle.setTexture(&this->texture);
        }
    }
}

inline void Piece::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(this->myCircle, states);
}

/**
 * get piece type, whether black or red
 */
inline PieceType Piece::getPieceType() const
{
    return this->pieceType;
}

/**
 * set piece as King. Will also change Piece Texture
 */
inline void Piece::activateKing()
{
    this->isKing = true;
    sf::Texture localTxr;
    if (pieceType == PieceType::Red)
    {
        /* code */
        if (localTxr.loadFromFile(RED_KING))
        {
            this->texture = localTxr;
            this->myCircle.setTexture(&this->texture);
        }
    }
    else
    {
        if (localTxr.loadFromFile(BLACK_KING))
        {
            this->texture = localTxr;
            this->myCircle.setTexture(&this->texture);
        }
    }
}

/**
 * get whether this piece is king
 * @return TRUE or FALSE
 */
inline bool Piece::getIsKing() const
{
    return this->isKing;
}

/**
 * move piece across board to global position (x,y)
 * @param posX by x position
 * @param posY the y position
 */
inline void Piece::moveCustom(const float posX, const float posY)
{
    // TODO Validate move, and verify if is King.
    this->setPosition(sf::Vector2f(posX, posY));
}

inline bool Piece::operator==(const Piece &other) const
{
    return this->index == other.index;
}

} // namespace chk
