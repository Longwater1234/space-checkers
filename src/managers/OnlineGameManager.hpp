#pragma once
#include "../GameManager.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

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
    void handleEvents(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2, chk::CircularBuffer<short> &buffer) override;
    void drawScreen(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2) override;
    void setOnReadyPiecesCallback(const onReadyCreatePieces &callback) override;

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

inline void OnlineGameManager::handleEvents(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2,
                                            chk::CircularBuffer<short> &circularBuffer)
{
    for (auto event = sf::Event{}; window->pollEvent(event);)
    {
        ImGui::SFML::ProcessEvent(*this->window, event);
        if (event.type == sf::Event::Closed)
        {
            window->close();
        }
        if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            const auto clickedPos = sf::Mouse::getPosition(*window);
            /* Check window bounds */
            if (clickedPos.y > chk::SIZE_CELL * 8)
            {
                continue;
            }
            for (auto &cell : this->getBlockList())
            {
                // inner loop
                if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
                {
                    const auto &hunter = this->isPlayerRedTurn() ? p1 : p2;
                    const auto &prey = this->isPlayerRedTurn() ? p2 : p1;

                    if (this->hasPendingCaptures())
                    {
                        this->handleJumpPiece(hunter, prey, cell);
                        this->updateMatchStatus(hunter, prey);
                        circularBuffer.clean();
                    }
                    else
                    {
                        this->handleCellTap(hunter, prey, circularBuffer, cell);
                    }
                    break;
                    // END inner loop
                }
            }
        }
    }
}

inline void OnlineGameManager::drawScreen(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2)
{
    auto mousePos = sf::Mouse::getPosition(*window);

    // RENDER CHECKERBOARD
    for (const auto &cell : this->getBlockList())
    {
        window->draw(*cell);
    }
    // DRAW RED PIECES
    for (const auto &[id, red_piece] : p1->getOwnPieces())
    {
        if (this->isPlayerRedTurn() && red_piece->containsPoint(mousePos))
        {
            red_piece->addOutline();
        }
        else
        {
            red_piece->removeOutline();
        }
        window->draw(*red_piece);
    }
    // DRAW BLACK PIECES
    for (const auto &[id, black_piece] : p2->getOwnPieces())
    {
        if (!this->isPlayerRedTurn() && black_piece->containsPoint(mousePos))
        {
            black_piece->addOutline();
        }
        else
        {
            black_piece->removeOutline();
        }
        window->draw(*black_piece);
    }
}

inline void OnlineGameManager::setOnReadyPiecesCallback(const onReadyCreatePieces &callback)
{
    this->_onReadyCreatePieces = callback;
}

} // namespace chk