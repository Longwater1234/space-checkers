#include "Piece.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <gtest/gtest.h>

TEST(PieceTests, GetId_ReturnsCorrectId)
{
    sf::CircleShape circle(37.5f);
    circle.setPosition(0.0f, 0.0f);
    chk::Piece piece(circle, chk::PieceType::Red, 42);
    EXPECT_EQ(piece.getId(), 42);
}

TEST(PieceTests, NewPiece_IsNotKing)
{
    sf::CircleShape circle(37.5f);
    circle.setPosition(0.0f, 0.0f);
    chk::Piece piece(circle, chk::PieceType::Red, 1);
    EXPECT_FALSE(piece.getIsKing());
}

TEST(PieceTests, MoveSimple_ValidRedMove_ReturnsTrue)
{
    sf::CircleShape circle(37.5f);
    circle.setPosition(150.0f, 150.0f);
    chk::Piece piece(circle, chk::PieceType::Red, 1);

    // RED piece moves up-left by one cell (deltaX=-75, deltaY=-75)
    sf::Vector2f dest(75.0f, 75.0f);
    EXPECT_TRUE(piece.moveSimple(dest));
}
