#include "Player.hpp"

namespace chk
{

Player::Player(PlayerType playerType)
{
    if (playerType == PlayerType::PLAYER_RED)
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
void Player::receivePiece(chk::PiecePtr &piecePtr)
{
    this->basket.emplace(piecePtr->getId(), std::move_if_noexcept(piecePtr));
}

/**
 * When a player's piece is captured, remove it from basket
 * @param targetId  the captured piece Id
 */
void Player::losePiece(const short targetId)
{
    this->basket.erase(targetId);
}

/**
 * Highlight all my hunter pieces which must capture the opponent
 * @param hunterPieces set of my piece IDs
 */
void Player::showMyHunters(const std::set<short> &hunterPieces) const
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
 * Remove all pieces from this player's ownership
 */
void Player::clearBasket()
{
    this->basket.clear();
}

/**
 * Get player type, RED or BLACK
 *
 *@return enum value
 */
PlayerType Player::getPlayerType() const
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
const std::string &Player::getName() const
{
    return this->name;
}

/**
 * Get all pieces this player owns
 * @return list of pieces
 */
const std::unordered_map<short, chk::PiecePtr> &Player::getOwnPieces() const
{
    return this->basket;
}

/**
 * check if player owns this piece
 * @param pieceId the pieceId
 * @return TRUE or FALSE
 */
bool Player::hasThisPiece(const short pieceId) const
{
    return this->basket.find(pieceId) != this->basket.end();
}

/**
 * \brief How many pieces this player currently have?
 * \return total count
 */
size_t Player::getPieceCount() const
{
    return this->basket.size();
}

/**
 * Move the specified piece by ONE cell to given destination on the board
 * @param pieceId the selected PieceId
 * @param destPos destination cell position
 * @return TRUE if successful, else FALSE
 */
bool Player::movePiece(const short pieceId, const sf::Vector2f &destPos) const
{
    return this->basket.at(pieceId)->moveSimple(destPos);
}

/**
 * \brief Move the given piece by TWO cells to the given destPos when capturing opponent
 * \param pieceId my pieceId
 * \param destPos destination cell
 * \return TRUE if successful, else FALSE
 */
bool Player::captureEnemyWith(const short pieceId, const sf::Vector2f &destPos) const
{
    return this->basket.at(pieceId)->moveCapture(destPos);
}

/**
 * Custom equality operator
 * @param other the other Player
 * @return true if their names equal
 */
bool Player::operator==(const Player &other) const
{
    return this->name == other.name;
}
} // namespace chk
