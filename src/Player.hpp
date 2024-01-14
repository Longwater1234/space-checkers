#pragma once

#include "Piece.hpp"
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace chk
{
enum class PlayerType
{
    // RED
    PLAYER_1 = 4883834,
    // BLACK
    PLAYER_2 = 8594839
};

// alias for unique pointer of player's Piece
using PiecePtr = std::unique_ptr<chk::Piece>;

class Player
{
  public:
    explicit Player(PlayerType player_type);
    void receivePiece(PiecePtr &piecePtr);
    void losePiece(const short &targetId);
    [[nodiscard]] const std::unordered_map<short, chk::PiecePtr> &getOwnPieces() const;
    void showForcedPieces(const std::set<short> &hunterPieces) const;
    [[nodiscard]] size_t getPieceCount() const;
    [[nodiscard]] const std::string &getName() const;
    [[nodiscard]] PlayerType getPlayerType() const;
    [[nodiscard]] bool hasThisPiece(const short &pieceId) const;
    [[nodiscard]] bool movePiece(const short &pieceId, const sf::Vector2f &destPos) const;
    [[nodiscard]] bool captureEnemyWith(const short &pieceId, const sf::Vector2f &destPos) const;
    bool operator==(const Player &other) const;

  private:
    // name of this player (RED or BLACK)
    std::string name_;
    // my pieceId -> its Pointer
    std::unordered_map<short, chk::PiecePtr> basket_;

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
 * Give this Player full ownership of this piece
 * @param piecePtr unique_ptr of piece
 */
inline void Player::receivePiece(chk::PiecePtr &piecePtr)
{
    this->basket_.emplace(piecePtr->getId(), std::move_if_noexcept(piecePtr));
}

/**
 * When a player's piece is captured, -1 from list
 * @param targetId  the captured piece Id
 */
inline void Player::losePiece(const short &targetId)
{
    this->basket_.erase(targetId);
}

/**
 * Highlight all my pieces which must capture the opponent
 * @param hunterPieces set of piece IDs
 */
inline void Player::showForcedPieces(const std::set<short> &hunterPieces) const
{
    if (hunterPieces.empty())
        return;
    for (const auto &id : hunterPieces)
    {
        this->basket_.at(id)->markImportant();
    }
}

/**
 * Get player type, RED or BLACK
 *
 *@return enum value
 */
inline PlayerType Player::getPlayerType() const
{
    if (this->name_ == "RED")
    {
        return chk::PlayerType::PLAYER_1;
    }
    return chk::PlayerType::PLAYER_2;
}

/**
 * get this player's name
 * @return either RED or BLACK
 */
inline const std::string &Player::getName() const
{
    return this->name_;
}

/**
 * Get all pieces this player owns
 * @return list of pieces
 */
inline const std::unordered_map<short, chk::PiecePtr> &Player::getOwnPieces() const
{
    return this->basket_;
}

/**
 * check if player owns this piece
 * @param pieceId the pieceId
 * @return TRUE or FALSE
 */
inline bool Player::hasThisPiece(const short &pieceId) const
{
    return this->basket_.find(pieceId) != this->basket_.end();
}

/**
 * \brief How many pieces this player currently have?
 * \return count
 */
inline size_t Player::getPieceCount() const
{
    return this->basket_.size();
}

/**
 * Get vector index of selected piece by this player
 * @param pieceId the selected PieceId
 * @param destPos destination cell
 * @return TRUE if successful or FALSE
 */
inline bool Player::movePiece(const short &pieceId, const sf::Vector2f &destPos) const
{
    return this->basket_.at(pieceId)->moveSimple(destPos);
}

/**
 * \brief Move the given piece to destPos to complete capture opponent
 * \param pieceId my pieceId
 * \param destPos destination cell
 * \return TRUE if successful or FALSE
 */
inline bool Player::captureEnemyWith(const short &pieceId, const sf::Vector2f &destPos) const
{
    return this->basket_.at(pieceId)->moveCapture(destPos);
}

/**
 * Custom equality operator
 * @param other the other Player
 * @return true if their names equal
 */
inline bool Player::operator==(const Player &other) const
{
    return this->name_ == other.name_;
}

} // namespace chk
