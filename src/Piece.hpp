#pragma once

#include "utils/ResourcePath.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <iostream>

namespace chk
{
enum class PieceType
{
    Red = 67895,
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
    explicit Piece(const sf::CircleShape &circle, const PieceType pType, short id);
    Piece() = delete;
    Piece(const Piece &) = delete;
    Piece &operator=(const Piece &) = delete;
    [[nodiscard]] const PieceType getPieceType() const;
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
    const short pid; // random positive ID assigned at Launch
    const PieceType pieceType;
    sf::CircleShape myCircle;
    bool isKing = false;
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

} // namespace chk
