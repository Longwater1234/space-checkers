#pragma once

#include "Piece.hpp"
#include <list>
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
    void givePiece(const chk::Piece &piece);
    void losePiece(const chk::Piece &captured);
    const std::list<Piece> getOwnPieces() const;
    [[nodiscard]] size_t getPieceCount() const;

  private:
    std::string name_;
    std::list<Piece> basket_;
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
    this->basket_.emplace_back(std::move(piece));
}

/**
 * When a player's piece is captured, -1 from list
 * @param captured  the captured piece
 */
inline void Player::losePiece(const chk::Piece &captured)
{
    this->basket_.remove_if([&captured](const Piece &p) { return p == captured; });
}

/**
 * get all pieces this player owns
 * @return list of pieces
 */
inline const std::list<Piece> Player::getOwnPieces() const
{
    return this->basket_;
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
