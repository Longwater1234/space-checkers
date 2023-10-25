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

  private:
    sf::RectangleShape rec_;
    int index_;
    sf::Text sfText;
    sf::Font myFont;
    void Cell::draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

inline void Cell::draw(sf::RenderTarget &target, sf::RenderStates states) const
{

    target.draw(rec_);
    if (index_ != 0)
    {
        target.draw(sfText);
    }
}

inline Cell::Cell(const sf::RectangleShape &rec, const sf::Vector2f &pos, const int index)
{
    this->rec_ = rec;
    this->index_ = index;

    sf::Text text;
    text.setFont(this->myFont);
    text.setFillColor(sf::Color{255u, 255u, 255u, 100u});
    text.setString(std::to_string(this->index_));
    text.setPosition(pos);
    this->sfText = text;
}

/**
 * Set the given font to this cell
 * @param font sf::Font loaded from disk
 */
inline void Cell::setFont(const sf::Font &font)
{
    this->myFont = font;
}

} // namespace chk
