#pragma once

#include "Piece.hpp"
#include <algorithm>
#include <map>
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
    void givePiece(PiecePtr &piecePtr);
    void losePiece(const chk::Piece &target);
    [[nodiscard]] const std::vector<PiecePtr> &getOwnPieces() const;
    [[nodiscard]] size_t getPieceCount() const;
    [[nodiscard]] int getPieceVecIndex(const int &pieceId);
    bool operator==(Player &other) const;

  private:
    std::string name_;
    std::vector<PiecePtr> basket_;
    // map of piece_id --> vector index
    std::map<int, int> cellMap;
    int counter;
};

inline Player::Player(PlayerType player_type)
{
    this->counter = 0;
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
 * Give Player full ownership of this piece
 * @param piece unique_ptr of piece
 */
inline void Player::givePiece(PiecePtr &piece)
{
    cellMap[piece->getId()] = counter++;
    this->basket_.emplace_back(std::move(piece));
}

/**
 * When a player's piece is captured, -1 from list
 * @param target  the captured piece
 */
inline void Player::losePiece(const chk::Piece &target)
{

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
 * Get vector index of selected piece by this player
 * @param pieceId the selected PieceId, stored in gameState
 * @return index inside Vector
 */
inline int Player::getPieceVecIndex(const int &pieceId)
{

    return cellMap[pieceId];
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
