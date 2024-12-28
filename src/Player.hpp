#pragma once

#include "Piece.hpp"
#include "PlayerType.hpp"
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>

namespace chk
{

// alias for unique pointer of `Piece`
using PiecePtr = std::unique_ptr<chk::Piece>;

class Player final
{
  public:
    explicit Player(PlayerType player_type);
    Player() = delete;
    Player(const Player &) = delete;
    Player &operator=(const Player &) = delete;
    void receivePiece(PiecePtr &piecePtr);
    void losePiece(const short targetId);
    [[nodiscard]] const std::unordered_map<short, chk::PiecePtr> &getOwnPieces() const;
    void showForcedPieces(const std::set<short> &hunterPieces) const;
    void emptyBasket();
    [[nodiscard]] size_t getPieceCount() const;
    [[nodiscard]] const std::string &getName() const;
    [[nodiscard]] PlayerType getPlayerType() const;
    [[nodiscard]] bool hasThisPiece(const short pieceId) const;
    [[nodiscard]] bool movePiece(const short pieceId, const sf::Vector2f &destPos) const;
    [[nodiscard]] bool captureEnemyWith(const short pieceId, const sf::Vector2f &destPos) const;
    bool operator==(const Player &other) const;

  private:
    // name of this player (RED or BLACK)
    std::string name;
    // hashmap of my PieceId -> its Pointer
    std::unordered_map<short, chk::PiecePtr> basket;
};

inline Player::Player(PlayerType player_type)
{
    if (player_type == PlayerType::PLAYER_RED)
    {
        this->name = "RED";
    }
    else
    {
        this->name = "BLACK";
    }
}

/**
 * Give this Player full ownership of this piece
 * @param piecePtr unique_ptr of piece
 */
inline void Player::receivePiece(chk::PiecePtr &piecePtr)
{
    this->basket.emplace(piecePtr->getId(), std::move_if_noexcept(piecePtr));
}

/**
 * When a player's piece is captured, remove it from basket
 * @param targetId  the captured piece Id
 */
inline void Player::losePiece(const short targetId)
{
    this->basket.erase(targetId);
}

/**
 * Highlight all my hunter pieces which must capture the opponent
 * @param hunterPieces set of piece IDs
 */
inline void Player::showForcedPieces(const std::set<short> &hunterPieces) const
{
    if (hunterPieces.empty())
    {
        return;
    }
    for (const auto &id : hunterPieces)
    {
        this->basket.at(id)->markImportant();
    }
}

/**
 * Remove all pieces from my basket
 */
inline void Player::emptyBasket()
{
    this->basket.clear();
}

/**
 * Get player type, RED or BLACK
 *
 *@return enum value
 */
inline PlayerType Player::getPlayerType() const
{
    if (this->name == "RED")
    {
        return chk::PlayerType::PLAYER_RED;
    }
    return chk::PlayerType::PLAYER_BLACK;
}

/**
 * Get this player's name
 * @return either RED or BLACK
 */
inline const std::string &Player::getName() const
{
    return this->name;
}

/**
 * Get all pieces this player owns
 * @return list of pieces
 */
inline const std::unordered_map<short, chk::PiecePtr> &Player::getOwnPieces() const
{
    return this->basket;
}

/**
 * check if player owns this piece
 * @param pieceId the pieceId
 * @return TRUE or FALSE
 */
inline bool Player::hasThisPiece(const short pieceId) const
{
    return this->basket.find(pieceId) != this->basket.end();
}

/**
 * \brief How many pieces this player currently have?
 * \return total count
 */
inline size_t Player::getPieceCount() const
{
    return this->basket.size();
}

/**
 * Move the specified piece by ONE cell to given destination on the board
 * @param pieceId the selected PieceId
 * @param destPos destination cell position
 * @return TRUE if successful, else FALSE
 */
inline bool Player::movePiece(const short pieceId, const sf::Vector2f &destPos) const
{
    return this->basket.at(pieceId)->moveSimple(destPos);
}

/**
 * \brief Move the given piece by TWO cells to the given destPos when capturing opponent
 * \param pieceId my pieceId
 * \param destPos destination cell
 * \return TRUE if successful, else FALSE
 */
inline bool Player::captureEnemyWith(const short pieceId, const sf::Vector2f &destPos) const
{
    return this->basket.at(pieceId)->moveCapture(destPos);
}

/**
 * Custom equality operator
 * @param other the other Player
 * @return true if their names equal
 */
inline bool Player::operator==(const Player &other) const
{
    return this->name == other.name;
}

} // namespace chk
