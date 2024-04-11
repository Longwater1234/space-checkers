#pragma once
#include "GameManager.hpp"

namespace chk
{
/**
 * This class is responsible for online gameplay
 * @since 2024-11-04
 */
class OnlineGameManager : public chk::GameManager
{
  public:
    OnlineGameManager() = default;
    void createAllPieces(std::vector<chk::PiecePtr> &pieceList) override;
    void setMyPlayerType(const chk::PlayerType &ptype);

  private: 
      chk::PlayerType myType;
};


void chk::OnlineGameManager::createAllPieces(std::vector<chk::PiecePtr>& pieceList)
{
}

inline void OnlineGameManager::setMyPlayerType(const chk::PlayerType &ptype)
{
    this->myType = ptype;
}

} // namespace chk