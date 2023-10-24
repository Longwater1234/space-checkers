#pragma once

#include "Piece.hpp"
#include <stack>
#include <string>

namespace chk
{
enum class PlayerType
{
    // RED
    PLAYER_1 = 4883834,
    // BLACK
    PLAYER_2 = 8594839
};

class Player
{
  public:
    Player(PlayerType player_type);
    void Player::givePiece(const chk::Piece &piece);
    void Player::losePiece();
    [[nodiscard]] size_t getPieceCount() const;

  private:
    std::string name_;
    std::stack<Piece> basket_;
};

inline Player::Player(PlayerType player_type)
{
    if (player_type == PlayerType::PLAYER_1)
    {
        this->name_ = "RED";
    }
    else
    {
        this->name_ = "BLACK";
    }
}

/**
 * \brief Give this Player their own piece
 * \param piece Checker piece
 */
inline void Player::givePiece(const chk::Piece &piece)
{
    this->basket_.push(piece);
}

/**
 * \brief When a player's piece is captured, -1 from stack
 */
inline void Player::losePiece()
{
    this->basket_.pop();
}

/**
 * \brief How many pieces this player currently have?
 * \return count as int
 */
inline size_t Player::getPieceCount() const
{
    return this->basket_.size();
}
} // namespace chk
