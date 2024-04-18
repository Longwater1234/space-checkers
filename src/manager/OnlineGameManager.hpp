#pragma once
#include "../GameManager.hpp"

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

    // Inherited via GameManager
    void handleEvents() override;
    void drawScreen() override;

  private:
    chk::PlayerType myType{};
};

inline void chk::OnlineGameManager::createAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    // TODO: complete me
}

inline void OnlineGameManager::setMyPlayerType(const chk::PlayerType &ptype)
{
    this->myType = ptype;
}

void OnlineGameManager::handleEvents()
{
}

void OnlineGameManager::drawScreen()
{
}

} // namespace chk