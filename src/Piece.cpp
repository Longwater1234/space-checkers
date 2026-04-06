#include "Piece.hpp"

namespace chk
{

Piece::Piece(const sf::CircleShape &circle, const PieceType pType, const short id) : pid(id)
{
    this->myCircle = circle;
    this->pieceType = pType;
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
void Piece::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(this->myCircle, states);
}

/**
 * Get piece type, whether it's Black or Red
 * @return the pieceType
 */
const PieceType Piece::getPieceType() const
{
    return this->pieceType;
}

/**
 *
 * Crown this piece as King. Will also change its texture
 */
void Piece::activateKing()
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
bool Piece::getIsKing() const
{
    return this->isKing;
}

/**
 * Check whether mouse cursor position is anywhere over this Piece
 * @param pos position relative to main Window
 * @return TRUE or FALSE
 */
bool Piece::containsPoint(const sf::Vector2i &pos) const
{
    return this->myCircle.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

/**
 * Highlight with yellow outline when focused
 */
void chk::Piece::addOutline()
{
    this->myCircle.setOutlineColor(sf::Color::Yellow);
    this->myCircle.setOutlineThickness(5.0f);
}

/**
 * Highlight with GREEN outline, to indicate it MUST capture opponent
 */
void Piece::markImportant()
{
    this->myCircle.setOutlineColor(sf::Color::Green);
    this->myCircle.setOutlineThickness(5.0f);
}

/**
 * Removes the outline when no longer in focus
 */
void Piece::removeOutline()
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
short Piece::getId() const
{
    return this->pid;
}

/**
 * Custom equality operator, compares ID of the pieces
 * @param other The other Piece
 * @return TRUE or FALSE
 */
bool Piece::operator==(const Piece &other) const
{
    return this->pid == other.pid;
}

/**
 * Simply move piece to given cell. Validate first, then Move the Piece diagonally to the given position.
 * @param destPos destination
 * @return TRUE if successful, else FALSE
 */
bool Piece::moveSimple(const sf::Vector2f &destPos)
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
bool Piece::moveCapture(const sf::Vector2f &destPos)
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
