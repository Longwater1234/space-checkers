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
    explicit Cell(int idx, const sf::RectangleShape &rec, const sf::Font &font);
    Cell() = delete;
    Cell &operator=(const Cell &) = delete;
    Cell(const Cell &) = delete;
    bool containsPoint(const sf::Vector2i &pos) const;
    bool isAtPosition(const sf::Vector2f &pos) const;
    const sf::Vector2f &getPos() const;
    int getIndex() const;
    void setEvenRow(bool val);
    bool getIsEvenRow() const;
    void highlightActive();
    void resetColor();

  private:
    const int index; // Darker cells are in range [1~32]. Lighter cells are all -1
    sf::RectangleShape rec;
    inline static const sf::Color DARK_BROWN{82, 55, 27};
    inline static const sf::Color BABY_BLUE{98, 174, 239};
    bool isEvenRow = false;
    sf::Vector2f cell_pos;
    sf::Text sfText;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

} // namespace chk
