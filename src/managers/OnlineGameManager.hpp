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
    explicit OnlineGameManager(sf::RenderWindow *windowPtr);
    OnlineGameManager() = delete;
    void createAllPieces(std::vector<chk::PiecePtr> &pieceList) override;
    void setMyPlayerType(const chk::PlayerType &ptype);

    // Inherited via GameManager
    void handleEvents() override;
    void drawScreen() override;
    void setOnReadyCreatePiecesCallback(const onReadyCreatePieces &callback) override;

  private:
    chk::PlayerType myType{};
};

inline OnlineGameManager::OnlineGameManager(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->sourceCell = -1;
    this->forcedMoves.clear();
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
}

inline void chk::OnlineGameManager::createAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    // TODO: complete me
    // make request to backend, wait for response
    spdlog::info(pieceList.size());
    this->_onReadyCreatePieces(false);
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

void OnlineGameManager::setOnReadyCreatePiecesCallback(const onReadyCreatePieces &callback)
{
    this->_onReadyCreatePieces = callback;
}

} // namespace chk