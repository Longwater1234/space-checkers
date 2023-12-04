#pragma once

#include "Piece.hpp"
#include <algorithm>
#include <unordered_map>
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
    void losePiece(const uint16_t &targetId);
    const std::unordered_map<int, chk::PiecePtr> &getOwnPieces() const;
    [[nodiscard]] size_t getPieceCount() const;
    [[nodiscard]] std::string getName() const;
    [[nodiscard]] PlayerType getPlayerType() const;
    [[nodiscard]] bool hasThisPiece(const int &pieceId);
    [[nodiscard]] bool movePiece(const int &pieceId, const sf::Vector2f &dest);
    bool operator==(Player &other) const;

  private:
    //name of this player (RED or BLACK)
    std::string name_;
    // my pieceId --> its Pointer
    std::unordered_map<int, chk::PiecePtr> basket_;
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
 * Give Player full ownership of this piece
 * @param piece unique_ptr of piece
 */
inline void Player::givePiece(chk::PiecePtr &piece)
{
    basket_[piece->getId()] = std::move_if_noexcept(piece);
}

/**
 * When a player's piece is captured, -1 from list
 * @param target  the captured piece Id
 */
inline void Player::losePiece(const uint16_t &targetId)
{
    this->basket_.erase(targetId);
}

/**
 * Get player type RED or BLACK
 *
 *@return enum value
 */
inline PlayerType Player::getPlayerType() const
{
    if (this->name_ == "RED")
    {
        return chk::PlayerType::PLAYER_1;
    }
    else
    {
        return chk::PlayerType::PLAYER_2;
    }
}

/**
 * get this player's name
 * @return either RED or BLACK
 */
inline std::string Player::getName() const
{
    return this->name_;
}

/**
 * Get all pieces this player owns
 * @return list of pieces
 */
inline const std::unordered_map<int, chk::PiecePtr> &Player::getOwnPieces() const
{
    return this->basket_;
}

/**
 * check if player owns this piece
 * @param pieceId the pieceId
 * @return TRUE or FALSE
 */
inline bool Player::hasThisPiece(const int &pieceId)
{
    return this->basket_.count(pieceId) > 0;
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
 * @param pieceId the selected PieceId
 * @param destPos destination
 * @return success true or FALSE
 */
inline bool Player::movePiece(const int &pieceId, const sf::Vector2f &destPos)
{
    return this->basket_[pieceId]->moveCustom(destPos);
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
