#include "Cell.hpp"

namespace chk
{

Cell::Cell(const int idx, const sf::RectangleShape &rect, const sf::Font &font) : index(idx)
{
    this->rec = rect;
    this->cell_pos = rec.getPosition();
    this->sfText.setFont(font);
    this->sfText.setFillColor(sf::Color{255, 255, 255, 100});
    this->sfText.setString(std::to_string(this->index));
    this->sfText.setPosition(this->cell_pos);
}

void Cell::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(rec, states);
    if (this->index != -1)
    {
        // only draw text on Darker cells
        target.draw(sfText, states);
    }
}

/**
 * Whether this cell is in even-numbered row
 * @return TRUE or FALSE
 */
bool Cell::getIsEvenRow() const
{
    return this->isEvenRow;
}

/**
 * Check whether mouse cursor is anywhere INSIDE this Cell region
 *
 * @param pos position x,y (int) relative to main Window
 * @return TRUE or FALSE
 */
bool Cell::containsPoint(const sf::Vector2i &pos) const
{
    return this->rec.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

/**
 * Check whether given object's position is EQUAL to this Cell's position
 *
 * @param pos position x,y (float) relative to main window
 * @return TRUE or FALSE
 */
bool Cell::isAtPosition(const sf::Vector2f &pos) const
{
    return this->cell_pos.x == pos.x && this->cell_pos.y == pos.y;
}

/**
 * Get index of this cell
 * @return index value
 */
int Cell::getIndex() const
{
    return this->index;
}

/**
 * Set whether this cell's row is even
 * @param val TRUE or FALSE
 */
void Cell::setEvenRow(bool val)
{
    this->isEvenRow = val;
}

/**
 * Get the position of this cell
 * @return local x,y position
 */
const sf::Vector2f &Cell::getPos() const
{
    return this->cell_pos;
}

/**
 * (ONLY FOR PLAYABLE CELLS) Highlight with BLUE the currently clicked cell with a piece
 */
void Cell::highlightActive()
{
    this->rec.setFillColor(BABY_BLUE);
}

/**
 * Restore the original color
 */
void Cell::resetColor()
{
    this->rec.setFillColor(DARK_BROWN);
}

} // namespace chk
