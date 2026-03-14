#include "Player.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <gtest/gtest.h>

TEST(PlayerTests, CaptureEnemyWith_Success)
{
    chk::Player player(chk::PlayerType::PLAYER_RED);

    // Create a RED piece positioned 2 cells diagonally from destination
    sf::CircleShape circle(37.5f);
    circle.setPosition(150.0f, 150.0f);
    chk::PiecePtr piece = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, 1);
    player.receivePiece(piece);

    // RED piece captures by jumping 2 cells diagonally upward (deltaX=-150, deltaY=-150)
    sf::Vector2f destination(0.0f, 0.0f);
    EXPECT_TRUE(player.captureEnemyWith(1, destination));
}

TEST(PlayerTests, LosePiece_RemovesPieceFromBasket)
{
    chk::Player player(chk::PlayerType::PLAYER_RED);

    sf::CircleShape circle(37.5f);
    circle.setPosition(0.0f, 0.0f);
    chk::PiecePtr piece = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, 1);
    player.receivePiece(piece);

    player.losePiece(1);
    EXPECT_FALSE(player.hasThisPiece(1));
}
