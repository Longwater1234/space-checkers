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
    Piece(const sf::CircleShape &circle, PieceType pType, unsigned int idx_);
    PieceType getPieceType() const;
    void activateKing();
    bool getIsKing() const;
    void moveCustom(float posX, float posY);
    bool containsPoint(const sf::Vector2i &pos) const;
    void addOutline();
    void removeOutline();
    unsigned int getId() const;
    bool operator==(const Piece &other) const;

  private:
    sf::Texture texture;
    unsigned int id;
    sf::CircleShape myCircle;
    PieceType pieceType;
    bool isKing = false;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

inline Piece::Piece(const sf::CircleShape &circle, const PieceType pType, const unsigned int idx_)
{
    this->myCircle = circle;
    this->pieceType = pType;
    this->id = idx_;

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
 * \return TRUE or FALSE
 */
inline bool Piece::getIsKing() const
{
    return this->isKing;
}

/**
 * move piece across board to global position (x,y)
 * \param posX by x position
 * \param posY the y position
 */
inline void Piece::moveCustom(const float posX, const float posY)
{
    // TODO Validate move, and verify if is King.
    this->setPosition(sf::Vector2f(posX, posY));
}

/**
 * \brief Check whether mouse cursor is currently over this piece
 * \param pos Mouse position relative to main Window
 * \return TRUE or FALSE
 */
inline bool Piece::containsPoint(const sf::Vector2i &pos) const
{
    return this->myCircle.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

/**
 * \brief Highlight with yellow outline when focused
 */
inline void chk::Piece::addOutline()
{
    this->myCircle.setOutlineColor(sf::Color::Yellow);
    this->myCircle.setOutlineThickness(5.0f);
}

/**
 * \brief Removes the outline
 */
inline void Piece::removeOutline()
{
    this->myCircle.setOutlineThickness(0);
}

/**
 * Get piece's id
 */
inline unsigned int Piece::getId() const
{
    return this->id;
}

/**
 * \brief Custom equality operator. Compares 2 pieces
 * \param other The other Piece
 * \return TRUE or FALSE
 */
inline bool Piece::operator==(const Piece &other) const
{
    return this->id == other.id;
}

} // namespace chk
