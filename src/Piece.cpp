#include "Piece.hpp"

namespace chk
{

Piece::Piece(const sf::CircleShape &circle, const PieceType pType, const int32_t id) : pid(id), pieceType(pType)
{
    this->myCircle = circle;
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
    if (pieceType == PieceType::Red)
    {
        if (this->texture.loadFromFile(chk::getResourcePath(RED_KING)))
        {
            this->myCircle.setTexture(&this->texture);
        }
    }
    else
    {
        if (this->texture.loadFromFile(chk::getResourcePath(BLACK_KING)))
        {
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
int32_t Piece::getId() const
{
    return this->pid;
}

/**
 * Update the animation for this piece
 * @param deltaTime Time elapsed since the last frame
 */
void Piece::updateAnimation(float deltaTime)
{
    // If the animation is finished, don't execute extra math
    if (animationProgress >= 1.0f)
        return;

    // Advance the progress percentage based on frame delta time
    animationProgress += animationSpeed * deltaTime;

    // Clamp it to 1.0f maximum so it doesn't overshoot the landing tile
    if (animationProgress > 1.0f)
    {
        animationProgress = 1.0f;
    }

    // Standard LERP Vector Formula
    sf::Vector2f currentPos = startPosition + animationProgress * (targetPosition - startPosition);

    // Update SFML visual components smoothly frame by frame
    this->setPosition(currentPos);
    this->myCircle.setPosition(currentPos);
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

    //  Trigger Smooth Animation
    this->startPosition = this->getPosition(); // Where we are right now
    this->targetPosition = destPos;            // Where the server says we must go
    this->animationProgress = 0.0f;            // Start the clock at 0%!

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

    if (std::abs(deltaX) != 2 * SIZE_CELL || std::abs(deltaY) != 2 * SIZE_CELL)
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

    //  Trigger Smooth Animation
    this->startPosition = this->getPosition();
    this->targetPosition = destPos;
    this->animationProgress = 0.0f;

    if ((this->pieceType == PieceType::Red && destPos.y == 0) ||
        (this->pieceType == PieceType::Black && destPos.y == 7 * chk::SIZE_CELL))
    {
        this->activateKing();
    }
    return true;
}

} // namespace chk
