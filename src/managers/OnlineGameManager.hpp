#pragma once
#include "../GameManager.hpp"
#include "../WsClient.hpp"
#include "../payloads/base_payload.pb.hpp"
#include "imgui-SFML.h"
#include <atomic>

namespace chk
{
using chk::payload::TeamColor;
/**
 * This class is responsible for online gameplay
 * @since 2024-04-11
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
    void handleCapturePiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                            const chk::Block &targetCell) override;
    void handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, chk::CircularBuffer<short> &buffer,
                       const chk::Block &cell) override;

  private:
    mutable chk::PlayerType myTeam{};
    std::unique_ptr<chk::WsClient> wsClient = nullptr;
    std::atomic_bool isMyTurn = false;
    std::atomic_bool gameReady = false;
    void startMoveListener();
    void startCaptureListener();
    void startDeathListener();
};

inline OnlineGameManager::OnlineGameManager(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->sourceCell = std::nullopt;
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
    this->wsClient = std::make_unique<chk::WsClient>();
    // CREATE TWO unique PLAYERS
    this->playerRed = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_RED);
    this->playerBlack = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_BLACK);

    // set Listener for connection success
    this->wsClient->setOnReadyConnectedCallback(
        [this](const chk::payload::WelcomePayload &welcome, std::string_view notice) {
            if (welcome.my_team() & TeamColor::TEAM_RED)
            {
                this->myTeam = chk::PlayerType::PLAYER_RED;
                this->isMyTurn = true;
                spdlog::info("I AM PLAYER RED");
            }
            else
            {
                this->myTeam = chk::PlayerType::PLAYER_BLACK;
                this->isMyTurn = false;
                spdlog::info("I AM PLAYER BLACK");
            }
            this->updateMessage(notice);
        });
}

/**
 * Wait for server to generate random IDs and deliver the response, then give each player their own set of pieces
 * @param pieceList destination of created pieces
 */
inline void chk::OnlineGameManager::createAllPieces()
{
    // wait for server to send Piece Ids
    this->wsClient->setOnReadyStartGameCallback([this](const chk::payload::StartPayload &payload,
                                                       std::string_view notice) {
        this->gameReady = true;
        this->updateMessage(notice);

        // Reserve container for pieces on board
        std::vector<chk::PiecePtr> pieceList;
        pieceList.reserve(chk::NUM_PIECES);

        auto redItr = payload.pieces_red().begin();
        auto blackItr = payload.pieces_black().begin();
        // create pieces objects, and position them on Board
        for (uint16_t row = 0; row < chk::NUM_ROWS; row++)
        {
            for (uint16_t col = 0; col < chk::NUM_COLS; col++)
            {
                if ((row + col) % 2 != 0)
                {
                    sf::CircleShape circle(0.5 * chk::SIZE_CELL);
                    const float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                    circle.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                    if (row < 3 && blackItr != payload.pieces_black().end())
                    {
                        // Half Top cells, put BLACK piece
                        auto kete =
                            std::make_unique<chk::Piece>(circle, chk::PieceType::Black, static_cast<short>(*blackItr));
                        pieceList.emplace_back(std::move_if_noexcept(kete));
                        ++blackItr;
                    }
                    else if (row > 4 && redItr != payload.pieces_red().end())
                    {
                        // Half Bottom cells, put RED piece
                        auto kete =
                            std::make_unique<chk::Piece>(circle, chk::PieceType::Red, static_cast<short>(*redItr));
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
                this->playerRed->receivePiece(kete);
            }
            else
            {
                this->playerBlack->receivePiece(kete);
            }
        }
        pieceList.clear();
        this->startMoveListener();
        this->startCaptureListener();
        this->startDeathListener();
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
    for (const auto &[id, red_piece] : this->playerRed->getOwnPieces())
    {
        if (this->myTeam == chk::PlayerType::PLAYER_RED && this->isMyTurn && red_piece->containsPoint(mousePos))
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
    for (const auto &[id, black_piece] : this->playerBlack->getOwnPieces())
    {
        if (this->myTeam == chk::PlayerType::PLAYER_BLACK && this->isMyTurn && black_piece->containsPoint(mousePos))
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
 * Move the selected piece to clicked cell, then update the gameMap and notify Server
 * @param player current player
 * @param opponent opposing player
 * @param destCell target cell
 * @param currentPieceId the selected PieceId
 */
inline void OnlineGameManager::handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent,
                                               const Block &destCell, const short &currentPieceId)
{
    // VERIFY if move is successful
    const bool success = player->movePiece(currentPieceId, destCell->getPos());
    if (!success)
    {
        return;
    }
    int sourceCellCopy = this->sourceCell.value();
    gameMap.erase(this->sourceCell.value());               // set old location empty!
    gameMap.emplace(destCell->getIndex(), currentPieceId); // fill in the new location
    this->sourceCell = std::nullopt;                       // reset source cell
    this->identifyTargets(opponent);                       // check  opportunities for Opponent

    if (!this->getForcedMoves().empty())
    {
        spdlog::info("YOU ARE IN DANGER ");
    }

    // SEND TO SERVER for validation
    auto newDestCell = new chk::payload::MovePayload_DestCell();
    newDestCell->set_cell_index(destCell->getIndex());
    newDestCell->set_x(destCell->getPos().x);
    newDestCell->set_y(destCell->getPos().y);

    // create Movepayload Protobuf
    auto movePayload = new chk::payload::MovePayload();
    movePayload->set_source_cell(sourceCellCopy);
    movePayload->set_piece_id(currentPieceId);
    movePayload->set_from_team(TeamColor::TEAM_RED);
    if (this->myTeam == chk::PlayerType::PLAYER_BLACK)
    {
        movePayload->set_from_team(TeamColor::TEAM_BLACK);
    }
    movePayload->set_allocated_dest_cell(newDestCell);

    // finally, create Base request
    chk::payload::BasePayload requestBody;
    requestBody.set_allocated_move_payload(movePayload);
    if (!this->wsClient->replyServerAsync(&requestBody))
    {
        spdlog::error("failed to send message to Server");
        return;
    }

    this->isMyTurn = !this->isMyTurn; // toggle player turns
    this->updateMessage("You have moved to " + std::to_string(destCell->getIndex()) + ". It's " + opponent->getName() +
                        "'s turn.");
}

/**
 * Perform capturing of "prey's" pieces by "hunter", then update gameMap, and notify Server
 * @param hunter the attacking player
 * @param prey the defensive player
 * @param targetCell the destination of hunter
 */
inline void OnlineGameManager::handleCapturePiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                                  const chk::Block &targetCell)
{
    // TODO complete me
}

/**
 * When current player taps any playable cell.
 * @param hunter currentPlayer
 * @param prey the opposing player
 * @param buffer Temporary store for clicked Pieces
 * @param cell Tapped cell
 */
inline void OnlineGameManager::handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                             chk::CircularBuffer<short> &buffer, const chk::Block &cell)
{
    if (!this->gameReady || !this->isMyTurn)
    {
        return;
    }
    // CHECK IF this cell has a Piece
    const short pieceId = this->getPieceFromCell(cell->getIndex());
    if (pieceId != -1)
    {
        // YES, it has one! CHECK IF THERE IS ANY PENDING "forced jumps"
        if (!this->getForcedMoves().empty())
        {
            this->showForcedMoves(hunter, cell);
            return;
        }
        // OTHERWISE, store it in buffer (for a simple move next)!
        buffer.addItem(pieceId);
        this->setSourceCell(cell->getIndex());
    }
    else
    {
        // Cell is Empty! Let's move a piece (from buffer) here!
        if (!buffer.isEmpty())
        {
            const short movablePieceId = buffer.getTop();
            if (!hunter->hasThisPiece(movablePieceId))
            {
                return;
            }
            this->handleMovePiece(hunter, prey, cell, movablePieceId);
            buffer.clean();
        }
    }
}

/**
 * This will be handling all UI events
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
                    // Me
                    const auto &mine = myTeam == chk::PlayerType::PLAYER_RED ? this->playerRed : this->playerBlack;
                    const auto &opponent = myTeam == chk::PlayerType::PLAYER_RED ? this->playerBlack : this->playerRed;

                    if (this->hasPendingCaptures())
                    {
                        this->handleCapturePiece(mine, opponent, cell);
                        GameManager::updateMatchStatus(mine, opponent);
                        circularBuffer.clean();
                    }
                    else
                    {
                        this->handleCellTap(mine, opponent, circularBuffer, cell);
                    }
                    // END inner loop
                    break;
                }
            }
        }
    }
}

/**
 * Listening for "MovePiece" events from server, and update gameBoard
 */
inline void OnlineGameManager::startMoveListener()
{
    this->wsClient->setOnMovePieceCallback([this](const chk::payload::MovePayload &payload) {
        // which player made the Move?
        const chk::PlayerPtr &opponent =
            payload.from_team() & TeamColor::TEAM_RED ? this->playerRed : this->playerBlack;
        const chk::PlayerPtr &myTeam = payload.from_team() & TeamColor::TEAM_RED ? this->playerBlack : this->playerRed;
        const auto targetPosition = sf::Vector2f{payload.dest_cell().x(), payload.dest_cell().y()};
        const bool success = opponent->movePiece(static_cast<short>(payload.piece_id()), targetPosition);
        if (!success)
        {
            return;
        }
        gameMap.erase(payload.source_cell());                                  // set old location empty!
        gameMap.emplace(payload.dest_cell().cell_index(), payload.piece_id()); // fill in the new location
        this->identifyTargets(myTeam);                                         // check  opportunities for Opponent

        if (!this->getForcedMoves().empty())
        {
            spdlog::info("OPPONENT IS IN DANGER");
        }

        this->isMyTurn = !this->isMyTurn; // toggle player turns
        this->updateMessage("Opponent moved to " + std::to_string(payload.dest_cell().cell_index()) +
                            ". It's your turn.");
    });
}

/**
 * Listening for CapturePiece events from server, and update gameBoard
 */
inline void OnlineGameManager::startCaptureListener()
{
    // TODO complete me
}

/**
 * Listening for killed connection from WsClient
 */
inline void OnlineGameManager::startDeathListener()
{
    this->wsClient->setOnDeathCallback([this](std::string_view notice) {
        this->updateMessage(notice);
        this->doCleanup();
    });
}

} // namespace chk