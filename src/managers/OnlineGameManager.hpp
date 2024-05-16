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

    // Inherited via GameManager
    void handleEvents(chk::CircularBuffer<short> &buffer) override;
    void drawScreen() override;
    void setOnReadyPiecesCallback(const onReadyCreatePieces &callback) override;

  private:
    chk::PlayerType myType{};
};

inline OnlineGameManager::OnlineGameManager(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->sourceCell = std::nullopt;
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
    // CREATE TWO unique PLAYERS
    this->player1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    this->player2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);
    assert(!(*player1 == *player2));
}

/**
 * Create all pieces for both players and add them to pieceList, using game Server number generator
 * @param pieceList destination of created pieces
 */
inline void chk::OnlineGameManager::createAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    // TODO: complete me
    // make request to backend, wait for response
    spdlog::info(pieceList.size());

    if (false)
    {
        // TODO WAIT FOR BACKEND , use callback here for WsClient
        this->matchCellsToPieces(pieceList);
        // GIVE EACH PLAYER their own piece
        for (auto &kete : pieceList)
        {
            if (kete->getPieceType() == chk::PieceType::Red)
            {
                this->player1->receivePiece(kete);
            }
            else
            {
                this->player2->receivePiece(kete);
            }
        }
    }
}

/**
 * This will be called in the main game loop, every 60 FPS, drawing elements on screen
 */
inline void OnlineGameManager::drawScreen()
{
    auto mousePos = sf::Mouse::getPosition(*window);

    // RENDER CHECKERBOARD
    for (const auto &cell : this->getBlockList())
    {
        window->draw(*cell);
    }
    // DRAW RED PIECES
    for (const auto &[id, red_piece] : this->player1->getOwnPieces())
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
    for (const auto &[id, black_piece] : this->player2->getOwnPieces())
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

/**
 * This will be handling all events
 * @param circularBuffer stores the currently selected piece
 */
inline void OnlineGameManager::handleEvents(chk::CircularBuffer<short> &circularBuffer)
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
                    const auto &hunter = this->isPlayerRedTurn() ? this->player1 : this->player2;
                    const auto &prey = this->isPlayerRedTurn() ? this->player2 : this->player1;

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
                    // END inner loop
                    break;
                }
            }
        }
    }
}

inline void OnlineGameManager::setOnReadyPiecesCallback(const onReadyCreatePieces &callback)
{
    this->_onReadyCreatePieces = callback;
}

} // namespace chk