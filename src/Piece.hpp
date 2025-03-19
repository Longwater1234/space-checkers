#pragma once
#include "utils/ResourcePath.hpp"
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
    Black,
};

constexpr auto BLACK_NORMAL = "black_normal.png";
constexpr auto BLACK_KING = "black_king.png";
constexpr auto RED_NORMAL = "red_normal.png";
constexpr auto RED_KING = "red_king.png";
constexpr auto SIZE_CELL = 75.0f; // length of square cell
constexpr uint16_t NUM_PIECES{24};

class Piece final : public sf::Drawable, public sf::Transformable
{

  public:
    explicit Piece(const sf::CircleShape &circle, const PieceType &pType, short id);
    Piece() = delete;
    Piece(const Piece &) = delete;
    Piece &operator=(const Piece &) = delete;
    [[nodiscard]] const PieceType &getPieceType() const;
    void activateKing();
    bool getIsKing() const;
    bool containsPoint(const sf::Vector2i &pos) const;
    bool moveSimple(const sf::Vector2f &destPos);
    bool moveCapture(const sf::Vector2f &destPos);
    void addOutline();
    void markImportant();
    void removeOutline();
    short getId() const;
    bool operator==(const Piece &other) const;

  private:
    sf::Texture texture;
    short pid; // random positive ID assigned at Launch
    sf::CircleShape myCircle;
    PieceType pieceType;
    bool isKing = false;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

inline Piece::Piece(const sf::CircleShape &circle, const PieceType &pType, const short id)
{
    this->myCircle = circle;
    this->pieceType = pType;
    this->pid = id;
    this->setPosition(circle.getPosition());

    sf::Texture localTxr;
    if (pieceType == PieceType::Red)
    {
        if (localTxr.loadFromFile(chk::getResourcePath(RED_NORMAL)))
        {
            this->texture = std::move_if_noexcept(localTxr);
            this->myCircle.setTexture(&this->texture);
        }
    }
    else
    {
        if (localTxr.loadFromFile(chk::getResourcePath(BLACK_NORMAL)))
        {
            this->texture = std::move_if_noexcept(localTxr);
            this->myCircle.setTexture(&this->texture);
        }
    }
}

/**
 * Override the draw function
 */
inline void Piece::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(this->myCircle, states);
}

/**
 * Get piece type, whether it's Black or Red
 * @return the pieceType
 */
inline const PieceType &Piece::getPieceType() const
{
    return this->pieceType;
}

/**
 *
 * Crown this piece as King. Will also change its texture
 */
inline void Piece::activateKing()
{
    this->isKing = true;
    sf::Texture localTxr;
    if (pieceType == PieceType::Red)
    {
        if (localTxr.loadFromFile(chk::getResourcePath(RED_KING)))
        {
            this->texture = std::move_if_noexcept(localTxr);
            this->myCircle.setTexture(&this->texture);
        }
    }
    else
    {
        if (localTxr.loadFromFile(chk::getResourcePath(BLACK_KING)))
        {
            this->texture = std::move_if_noexcept(localTxr);
            this->myCircle.setTexture(&this->texture);
        }
    }
}

/**
 * Determines whether this piece is King
 * @return TRUE or FALSE
 */
inline bool Piece::getIsKing() const
{
    return this->isKing;
}

/**
 * Check whether mouse cursor position is anywhere over this Piece
 * @param pos position relative to main Window
 * @return TRUE or FALSE
 */
inline bool Piece::containsPoint(const sf::Vector2i &pos) const
{
    return this->myCircle.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

/**
 * Highlight with yellow outline when focused
 */
inline void chk::Piece::addOutline()
{
    this->myCircle.setOutlineColor(sf::Color::Yellow);
    this->myCircle.setOutlineThickness(5.0f);
}

/**
 * Highlight with GREEN outline, to indicate it MUST capture opponent
 */
inline void Piece::markImportant()
{
    this->myCircle.setOutlineColor(sf::Color::Green);
    this->myCircle.setOutlineThickness(5.0f);
}

/**
 * Removes the outline when no longer in focus
 */
inline void Piece::removeOutline()
{
    // if marked important (GREEN), don't remove
    if (this->myCircle.getOutlineColor() == sf::Color::Green)
    {
        return;
    }
    this->myCircle.setOutlineThickness(0);
}

/**
 * Get unique ID of this piece
 */
inline short Piece::getId() const
{
    return this->pid;
}

/**
 * Custom equality operator, compares ID of the pieces
 * @param other The other Piece
 * @return TRUE or FALSE
 */
inline bool Piece::operator==(const Piece &other) const
{
    return this->pid == other.pid;
}

/**
 * Simply move piece to given cell. Validate first, then Move the Piece diagonally to the given position.
 * @param destPos destination
 * @return TRUE if successful, else FALSE
 */
inline bool Piece::moveSimple(const sf::Vector2f &destPos)
{
    const float deltaX = destPos.x - this->getPosition().x;
    const float deltaY = destPos.y - this->getPosition().y;

    if (std::abs(deltaX) != chk::SIZE_CELL || std::abs(deltaY) != chk::SIZE_CELL)
    {
        return false;
    }
    if (this->pieceType == PieceType::Red && deltaY > 0.0f && !this->isKing)
    {
        return false;
    }
    if (this->pieceType == PieceType::Black && deltaY < 0.0f && !this->isKing)
    {
        return false;
    }

    this->move(sf::Vector2f{deltaX, deltaY});
    this->myCircle.setPosition(this->getPosition());
    if ((this->pieceType == PieceType::Red && destPos.y == 0) ||
        (this->pieceType == PieceType::Black && destPos.y == 7 * chk::SIZE_CELL))
    {
        this->activateKing();
    }
    return true;
}

/**
 * When capturing opponent, Validate first, then move by 2 cells diagonally to the given position.
 * @param destPos destination
 * @return TRUE if successful, else FALSE
 */
inline bool Piece::moveCapture(const sf::Vector2f &destPos)
{
    const float deltaX = destPos.x - this->getPosition().x;
    const float deltaY = destPos.y - this->getPosition().y;

    if (std::abs(deltaX) != 2 * SIZE_CELL && std::abs(deltaY) != 2 * SIZE_CELL)
    {
        return false;
    }
    if (this->pieceType == PieceType::Red && deltaY > 0.0f && !this->isKing)
    {
        return false;
    }
    if (this->pieceType == PieceType::Black && deltaY < 0.0f && !this->isKing)
    {
        return false;
    }

    this->move(sf::Vector2f{deltaX, deltaY}); // means "move by (x,y) amount"
    this->myCircle.setPosition(this->getPosition());
    if ((this->pieceType == PieceType::Red && destPos.y == 0) ||
        (this->pieceType == PieceType::Black && destPos.y == 7 * chk::SIZE_CELL))
    {
        this->activateKing();
    }
    return true;
}

} // namespace chk
