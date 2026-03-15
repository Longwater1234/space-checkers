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
    explicit Player(PlayerType playerType);
    Player() = delete;
    Player(const Player &) = delete;
    Player &operator=(const Player &) = delete;
    void receivePiece(PiecePtr &piecePtr);
    void losePiece(const short targetId);
    [[nodiscard]] const std::unordered_map<short, chk::PiecePtr> &getOwnPieces() const;
    void showMyHunters(const std::set<short> &hunterPieces) const;
    void clearBasket();
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

} // namespace chk
