#pragma once

#include "Piece.hpp"
#include <algorithm>
#include <list>
#include <memory>
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

// unique pointer of player's Piece
using PiecePtr = std::unique_ptr<chk::Piece>;

class Player
{
  public:
    Player(PlayerType player_type);
    void givePiece(PiecePtr piece);
    void losePiece(const chk::Piece &captured);
    const std::list<PiecePtr> &getOwnPieces() const;
    [[nodiscard]] size_t getPieceCount() const;

  private:
    std::string name_;
    std::list<PiecePtr> basket_;
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
 * \brief Give Player full ownership of this piece
 * \param piece Checker piece
 */
inline void Player::givePiece(PiecePtr piece)
{
    this->basket_.emplace_back(std::move(piece));
}

/**
 * When a player's piece is captured, -1 from list
 * @param captured  the captured piece
 */
inline void Player::losePiece(const chk::Piece &captured)
{
    this->basket_.remove_if([&captured](const PiecePtr &piece) { return *piece == captured; });
}

/**
 * Get all pieces this player owns
 * @return list of pieces
 */
inline const std::list<PiecePtr> &Player::getOwnPieces() const
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
