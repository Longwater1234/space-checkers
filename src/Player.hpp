#pragma once

#include "Piece.hpp"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

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
    explicit Player(PlayerType player_type);
    void givePiece(PiecePtr piece);
    void losePiece(const chk::Piece &target);
    [[nodiscard]] const std::vector<PiecePtr> &getOwnPieces() const;
    [[nodiscard]] size_t getPieceCount() const;
    bool operator==(Player &other) const;

  private:
    std::string name_;
    std::vector<PiecePtr> basket_;
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
 * @param target  the captured piece
 */
inline void Player::losePiece(const chk::Piece &target)
{
    // for std::list ONLY
    //   this->basket_.remove_if([&captured](const PiecePtr &piece) { return *piece == captured; });

    for (auto it = this->basket_.begin(); it != this->basket_.end();)
    {
        if (**it == target)
        {
            it = this->basket_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/**
 * Get all pieces this player owns
 * @return list of pieces
 */
inline const std::vector<PiecePtr> &Player::getOwnPieces() const
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

/**
 * Custom equality operator
 * @param other the other Player
 * @return true if their names equal
 */
bool Player::operator==(Player &other) const
{
    return this->name_ == other.name_;
}
} // namespace chk
