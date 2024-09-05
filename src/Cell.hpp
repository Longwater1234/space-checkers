#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <string>

namespace chk
{

class Cell final : public sf::Drawable
{
  public:
    explicit Cell(int idx, const sf::RectangleShape &rec, const sf::Vector2f &pos, const sf::Font &font);
    Cell() = delete;
    Cell &operator=(const Cell &) = delete;
    Cell(const Cell &) = delete;
    bool containsPoint(const sf::Vector2i &pos) const;
    bool isAtPosition(const sf::Vector2f &pos) const;
    const sf::Vector2f &getPos() const;
    int getIndex() const;
    void setEvenRow(bool is_even);
    bool getIsEvenRow() const;

  private:
    sf::RectangleShape rec;
    int index; // Darker cells have index in range [1~32]. Lighter cells are all -1
    bool isEvenRow = false;
    sf::Vector2f cell_pos;
    sf::Text sfText;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

inline Cell::Cell(const int idx, const sf::RectangleShape &rect, const sf::Vector2f &pos, const sf::Font &font)
{
    this->rec = rect;
    this->index = idx;
    this->cell_pos = pos;

    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color{255, 255, 255, 100});
    text.setString(std::to_string(this->index));
    text.setPosition(pos);
    this->sfText = text;
}

inline void Cell::draw(sf::RenderTarget &target, sf::RenderStates states) const
{

    target.draw(rec, states);
    if (this->index != -1)
    {
        // only draw text on Darker cells
        target.draw(sfText, states);
    }
}

/**
 * Whether this cell is in even numbered row
 * @return TRUE or FALSE
 */
inline bool Cell::getIsEvenRow() const
{
    return this->isEvenRow;
}

/**
 * Check whether mouse cursor is anywhere within this Cell region
 * @param pos position x,y (int) relative to main Window
 * @return TRUE or FALSE
 */
inline bool Cell::containsPoint(const sf::Vector2i &pos) const
{
    return this->rec.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

/**
 * Check whether given object's position is EQUAL to this Cell's position
 * @param pos position x,y (float) relative to main window
 * @return TRUE or FALSE
 */
inline bool Cell::isAtPosition(const sf::Vector2f &pos) const
{
    return this->cell_pos.x == pos.x && this->cell_pos.y == pos.y;
}

/**
 * Get index of this cell
 * @return index value
 */
inline int Cell::getIndex() const
{
    return this->index;
}

/**
 * Set whether this cell's row is even
 * @param is_even TRUE or FALSE
 */
inline void Cell::setEvenRow(bool is_even)
{
    this->isEvenRow = is_even;
}

/**
 * Get the position of this cell
 * @return local x,y position
 */
const inline sf::Vector2f &Cell::getPos() const
{
    return this->cell_pos;
}

} // namespace chk
