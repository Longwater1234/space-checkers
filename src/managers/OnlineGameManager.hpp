#pragma once
#include "../GameManager.hpp"
#include "../WsClient.hpp"
#include "../payloads/ServerStructs.hpp"
#include "imgui-SFML.h"
#include <atomic>

namespace chk
{
/**
 * This class is responsible for online gameplay
 * @since 2024-11-04
 */
class OnlineGameManager final : public chk::GameManager
{
  public:
    explicit OnlineGameManager(sf::RenderWindow *windowPtr);
    OnlineGameManager() = delete;
    void createAllPieces() override;

    // Inherited via GameManager
    void handleEvents(chk::CircularBuffer<short> &circularBuffer) override;
    void drawBoard() override;

  protected:
    // Inherited via GameManager
    void handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                         const short &currentPieceId) override;
    void handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                         const chk::Block &targetCell) override;
    void handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, chk::CircularBuffer<short> &buffer,
                       const chk::Block &cell) override;

  private:
    chk::PlayerType _myTeam{};
    std::unique_ptr<chk::WsClient> wsClient = nullptr;
    std::atomic_bool isMyTurn = false;
    std::atomic_bool gameReady = false;
};

inline OnlineGameManager::OnlineGameManager(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->sourceCell = std::nullopt;
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
    this->wsClient = std::make_unique<chk::WsClient>();
    // CREATE TWO unique PLAYERS
    this->player1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_RED);
    this->player2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_BLACK);
    assert(!(*player1 == *player2));
}

/**
 * Wait for server to generate random IDs and deliver the response, then give each player their own set of pieces
 * @param pieceList destination of created pieces
 */
inline void chk::OnlineGameManager::createAllPieces()
{
    this->wsClient->setOnReadyPiecesCallback([this](chk::payload::Welcome &welcome) {
        this->_myTeam = welcome.myTeam;
        if (this->_myTeam == PlayerType::PLAYER_RED)
        {
            this->isMyTurn = true; // Red always starts game
        }
        this->updateMessage(welcome.notice);
    });

    // wait for start game signal
    this->wsClient->setOnReadyStartGameCallback([this](chk::payload::StartGame &payload) {
        this->gameReady = true;
        this->updateMessage(payload.notice);

        std::vector<chk::PiecePtr> pieceList;
        pieceList.reserve(chk::NUM_PIECES);

        auto redItr = payload.piecesRed.begin();
        auto blackItr = payload.piecesBlack.begin();
        // create pieces objects, using id's from Server
        for (uint16_t row = 0; row < chk::NUM_ROWS; row++)
        {
            for (uint16_t col = 0; col < chk::NUM_COLS; col++)
            {
                if ((row + col) % 2 != 0)
                {
                    sf::CircleShape circle(0.5 * chk::SIZE_CELL);
                    const float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                    circle.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                    if (row < 3 && blackItr != payload.piecesBlack.end())
                    {
                        // Half Top cells, put BLACK piece
                        auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, *blackItr);
                        pieceList.emplace_back(std::move_if_noexcept(kete));
                        ++blackItr;
                    }
                    else if (row > 4 && redItr != payload.piecesRed.end())
                    {
                        // Half Bottom cells, put RED piece
                        auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, *redItr);
                        pieceList.emplace_back(std::move_if_noexcept(kete));
                        ++redItr;
                    }
                }
            }
        }

        // GIVE EACH PLAYER their own piece
        this->matchCellsToPieces(pieceList);
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
    });
}

/**
 * This will be called in the main game loop, every 60 FPS, drawing elements on screen
 */
inline void OnlineGameManager::drawBoard()
{
    const auto mousePos = sf::Mouse::getPosition(*window);
    // DRAW CHECKERBOARD
    for (const auto &cell : this->getBlockList())
    {
        window->draw(*cell);
    }
    // run the Websocket client
    if (this->wsClient != nullptr)
    {
        wsClient->runMainLoop();
    }
    // DRAW RED PIECES
    for (const auto &[id, red_piece] : this->player1->getOwnPieces())
    {
        if (this->_myTeam == PlayerType::PLAYER_RED && this->isMyTurn && red_piece->containsPoint(mousePos))
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
        if (this->_myTeam == PlayerType::PLAYER_BLACK && this->isMyTurn && black_piece->containsPoint(mousePos))
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

inline void OnlineGameManager::handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent,
                                               const Block &destCell, const short &currentPieceId)
{
    // TODO complete me
}

inline void OnlineGameManager::handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                               const chk::Block &targetCell)
{
    // TODO complete me
}

inline void OnlineGameManager::handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                             chk::CircularBuffer<short> &buffer, const chk::Block &cell)
{
    // TODO complete me
    if (!this->gameReady || this->isGameOver())
    {
        return;
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
            // START inner loop:
            for (auto &cell : this->getBlockList())
            {
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

} // namespace chk