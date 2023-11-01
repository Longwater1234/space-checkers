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
    Cell(const sf::RectangleShape &rec, const sf::Vector2f &pos, int index);
    void setFont(const sf::Font &font);
    bool containsPoint(const sf::Vector2i &pos) const;
    bool containsOrigin(const sf::Vector2f &pos) const;
    const sf::Vector2f &getCellPos() const;
    int getIndex() const;

  private:
    sf::RectangleShape rec_;
    int index_;
    sf::Vector2f cell_pos;

  private:
    sf::Text sfText;
    sf::Font myFont;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

inline Cell::Cell(const sf::RectangleShape &rec, const sf::Vector2f &pos, const int index)
{
    this->rec_ = rec;
    this->index_ = index;
    this->cell_pos = pos;

    sf::Text text;
    text.setFont(this->myFont);
    text.setFillColor(sf::Color{255u, 255u, 255u, 100u});
    text.setString(std::to_string(this->index_));
    text.setPosition(pos);
    this->sfText = text;
}

inline void Cell::draw(sf::RenderTarget &target, sf::RenderStates states) const
{

    target.draw(rec_, states);
    if (index_ != -1)
    {
        target.draw(sfText, states);
    }
}

/**
 * Set the given font to this cell
 * @param font sf::Font loaded from disk
 */
inline void Cell::setFont(const sf::Font &font)
{
    this->myFont = font;
}

/**
 * Check whether mouse cursor is within this Cell
 * @param pos 2D position (int) relative to main Window
 * @return TRUE or FALSE
 */
inline bool Cell::containsPoint(const sf::Vector2i &pos) const
{
    return this->rec_.getGlobalBounds().contains(static_cast<float>(pos.x), static_cast<float>(pos.y));
}

/**
 * Check whether object is within this Cell location
 * @param pos 2D position (float) relative to main window
 * @return TRUE or FALSE
 */
bool Cell::containsOrigin(const sf::Vector2f &pos) const
{
    return this->cell_pos.x == pos.x && this->cell_pos.y == pos.y;
}

/**
 * Get index of this cell
 * @return index value
 */
inline int Cell::getIndex() const
{
    return this->index_;
}

/**
 * Get the position of this cell
 * @return local x,y position
 */
const inline sf::Vector2f &Cell::getCellPos() const
{
    return this->cell_pos;
}

} // namespace chk
