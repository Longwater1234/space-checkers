#pragma once

#include "Piece.hpp"
#include "PlayerType.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace chk
{

// alias for unique pointer of `Piece`
using PiecePtr = std::unique_ptr<chk::Piece>;

class Player final
{
  public:
    explicit Player(PlayerType playerType);
    Player() = delete;
    Player(const Player &) = delete;
    Player &operator=(const Player &) = delete;
    void receivePiece(PiecePtr &piecePtr);
    void losePiece(const int32_t targetId);
    [[nodiscard]] const std::unordered_map<int32_t, chk::PiecePtr> &getOwnPieces() const;
    void showMyHunters(const std::unordered_set<int32_t> &hunterPieces) const;
    void clearBasket();
    [[nodiscard]] size_t getPieceCount() const;
    [[nodiscard]] const std::string &getName() const;
    [[nodiscard]] PlayerType getPlayerType() const;
    [[nodiscard]] bool hasThisPiece(const int32_t pieceId) const;
    [[nodiscard]] bool movePiece(const int32_t pieceId, const sf::Vector2f &destPos);
    [[nodiscard]] bool captureEnemyWith(const int32_t pieceId, const sf::Vector2f &destPos);
    bool operator==(const Player &other) const;

  private:
    // name of this player (RED or BLACK)
    std::string name;
    // hashmap of my PieceId -> its Pointer
    std::unordered_map<int32_t, chk::PiecePtr> basket;
};

} // namespace chk
