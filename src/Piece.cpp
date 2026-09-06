#include "Piece.hpp"

namespace chk
{

Piece::Piece(const sf::CircleShape &circle, const PieceType pType, const int32_t id)
    : pid(id), pieceType(pType), myCircle(circle)
{
    this->setPosition(circle.getPosition());
    this->logicalPosition = circle.getPosition();
    this->myCircle.setTexture(&getSharedTexture(pieceType, isKing));
}

/**
 * Get the shared texture for a given piece type and king status
 * @param type the piece type (Red or Black)
 * @param isKing whether the piece is a king
 * @return reference to the shared texture
 */
const sf::Texture &Piece::getSharedTexture(PieceType type, bool isKing)
{
    static sf::Texture redNormal;
    static sf::Texture blackNormal;
    static sf::Texture redKing;
    static sf::Texture blackKing;
    static bool loaded = false;

    if (!loaded)
    {
        (void)redNormal.loadFromFile(chk::getResourcePath(RED_NORMAL));
        (void)blackNormal.loadFromFile(chk::getResourcePath(BLACK_NORMAL));
        (void)redKing.loadFromFile(chk::getResourcePath(RED_KING));
        (void)blackKing.loadFromFile(chk::getResourcePath(BLACK_KING));
        loaded = true;
    }

    if (type == PieceType::Red)
    {
        return isKing ? redKing : redNormal;
    }
    return isKing ? blackKing : blackNormal;
}

/**
 * Override the draw function from sf::Drawable to render the piece on the window
 * @param target The render target (window) to draw on
 * @param states The render states to use for drawing
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
    this->myCircle.setTexture(&getSharedTexture(pieceType, isKing));
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
 * Where this piece logically sits, ignoring any slide animation in flight.
 * @return board position of the piece
 */
const sf::Vector2f &Piece::getLogicalPos() const
{
    return this->logicalPosition;
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
    // Measured from the logical position, NOT the animated one.
    const float deltaX = destPos.x - this->logicalPosition.x;
    const float deltaY = destPos.y - this->logicalPosition.y;

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
    this->startPosition = this->getPosition(); // where we are being drawn right now
    this->targetPosition = destPos;            // where we must end up
    this->logicalPosition = destPos;           // logically we are already there
    this->animationProgress = 0.0f;            // start the clock at 0%

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
    const float deltaX = destPos.x - this->logicalPosition.x;
    const float deltaY = destPos.y - this->logicalPosition.y;

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
    this->logicalPosition = destPos;
    this->animationProgress = 0.0f;

    if ((this->pieceType == PieceType::Red && destPos.y == 0) ||
        (this->pieceType == PieceType::Black && destPos.y == 7 * chk::SIZE_CELL))
    {
        this->activateKing();
    }
    return true;
}

} // namespace chk
