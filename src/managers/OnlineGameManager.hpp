#pragma once
#include "../GameManager.hpp"
#include "../WsClient.hpp"
#include "../payloads/base_payload.pb.hpp"
#include "imgui-SFML.h"

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

    // Inherited via GameManager
    void createAllPieces() override;
    void handleEvents(chk::CircularBuffer<short> &circularBuffer) override;
    void drawBoard() override;

  protected:
    // Inherited via GameManager
    void handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                         const short currentPieceId) override;
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
            if (welcome.my_team() == chk::payload::TeamColor::TEAM_RED)
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
            this->startDeathListener();
        });
}

/**
 * Wait for server to generate random IDs and deliver the response, then give each player their own set of pieces
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
                    sf::CircleShape circle{0.5 * chk::SIZE_CELL};
                    const float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                    circle.setPosition(sf::Vector2f{x, row * chk::SIZE_CELL});
                    if (row < 3 && blackItr != payload.pieces_black().end())
                    {
                        // Half Top cells, put BLACK piece
                        auto pb =
                            std::make_unique<chk::Piece>(circle, chk::PieceType::Black, static_cast<short>(*blackItr));
                        pieceList.emplace_back(std::move_if_noexcept(pb));
                        ++blackItr;
                    }
                    else if (row > 4 && redItr != payload.pieces_red().end())
                    {
                        // Half Bottom cells, put RED piece
                        auto pr =
                            std::make_unique<chk::Piece>(circle, chk::PieceType::Red, static_cast<short>(*redItr));
                        pieceList.emplace_back(std::move_if_noexcept(pr));
                        ++redItr;
                    }
                }
            }
        }

        // GIVE EACH PLAYER their own piece
        GameManager::matchCellsToPieces(pieceList);
        for (auto &pp : pieceList)
        {
            if (pp->getPieceType() == chk::PieceType::Red)
            {
                this->playerRed->receivePiece(pp);
            }
            else
            {
                this->playerBlack->receivePiece(pp);
            }
        }
        pieceList.clear(); // SAFE! no longer used.
        this->startMoveListener();
        this->startCaptureListener();
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
 * This will be handling all UI events.
 * @param circularBuffer stores the currently selected piece
 */
inline void OnlineGameManager::handleEvents(chk::CircularBuffer<short> &buffer)
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
                    // clang-format off
                    const auto &me = (myTeam == chk::PlayerType::PLAYER_RED) ? this->playerRed : this->playerBlack;
                    const auto &opponent = (myTeam == chk::PlayerType::PLAYER_RED) ? this->playerBlack : this->playerRed;
                    this->handleCellTap(me, opponent, buffer, cell);
                    break;
                    // clang-format on
                }
            }
            //^ END inner loop
        }
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
                                               const Block &destCell, const short currentPieceId)
{
    // VERIFY if move is successful
    if (!player->movePiece(currentPieceId, destCell->getPos()))
    {
        return;
    }
    const int copySrcCell = this->sourceCell.value();
    gameMap.erase(this->sourceCell.value());               // set old location empty!
    gameMap.emplace(destCell->getIndex(), currentPieceId); // fill in the new location
    this->sourceCell = std::nullopt;                       // reset source cell
    chk::GameManager::identifyTargets(opponent);           // check  opportunities for Opponent

    if (!this->getForcedMoves().empty())
    {
        spdlog::warn("YOU ARE IN DANGER ");
    }

    // prepare to send to SERVER
    // (must use raw ptr for embedded protobuf)
    auto *newDestCell = new chk::payload::MovePayload_Detination();
    newDestCell->set_cell_index(destCell->getIndex());
    newDestCell->set_x(destCell->getPos().x);
    newDestCell->set_y(destCell->getPos().y);

    // create Movepayload Protobuf
    auto *movePayload = new chk::payload::MovePayload();
    movePayload->set_source_cell(copySrcCell);
    movePayload->set_piece_id(currentPieceId);
    movePayload->set_from_team(TeamColor::TEAM_RED);
    if (this->myTeam == chk::PlayerType::PLAYER_BLACK)
    {
        movePayload->set_from_team(TeamColor::TEAM_BLACK);
    }
    movePayload->set_allocated_destination(newDestCell);

    // finally, create BasePayload
    chk::payload::BasePayload requestBody;
    requestBody.set_allocated_move_payload(movePayload);
    if (!this->wsClient->replyServer(requestBody))
    {
        this->updateMessage("failed to send message to Server");
        return;
    }

    this->isMyTurn = !this->isMyTurn; // toggle player turns
    this->updateMessage("You have moved to " + std::to_string(destCell->getIndex()) + ". It's " + opponent->getName() +
                        "'s turn.");
}

/**
 * Perform capturing of "prey's" pieces by me (the "hunter"), then update gameMap, and notify Server
 * @param hunter the attacking player
 * @param prey the defensive player
 * @param targetCell the destination of hunter
 */
inline void OnlineGameManager::handleCapturePiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                                  const chk::Block &targetCell)
{
    if (!this->isMyTurn || GameManager::getPieceFromCell(targetCell->getIndex()) != -1)
    {
        return;
    }

    // CREATE SOME COPIES BEFORE UPDATING:
    int copyHunterPiece = 0; // hunter pieceId
    int copySrcCell = 0;     // hunter src cell index
    int copyPreyPieceId = 0;
    int copyPreyCell = 0;
    // for tracking King status of Hunter piece
    bool isKingBefore = false;
    bool isKingNow = false;

    bool isCaptured = false; // external guard to verify capture is completed

    for (const auto &[hunterPieceId, target] : this->getForcedMoves())
    {
        if (target.hunterNextCell == targetCell->getIndex())
        {
            isKingBefore = hunter->getOwnPieces().at(hunterPieceId)->getIsKing();
            if (!hunter->captureEnemyWith(hunterPieceId, targetCell->getPos()))
            {
                return;
            }
            isCaptured = true; // verified
            this->updateMessage("You have captured " + prey->getName() + "'s piece!");
            copySrcCell = this->sourceCell.value();
            gameMap.erase(this->sourceCell.value());                           // set hunter's old location empty!
            gameMap.erase(target.preyCellIdx);                                 // set Prey's old location empty!
            gameMap.emplace(targetCell->getIndex(), hunterPieceId);            // fill in hunter new location
            prey->losePiece(target.preyPieceId);                               // the defending player loses 1 piece
            this->sourceCell = std::nullopt;                                   // reset source cell
            isKingNow = hunter->getOwnPieces().at(hunterPieceId)->getIsKing(); // update changes for hunter piece
            copyHunterPiece = hunterPieceId;
            copyPreyPieceId = target.preyPieceId;
            copyPreyCell = target.preyCellIdx;
            break;
        }
    }
    if (!isCaptured)
    {
        return;
    }
    // prey details (must use raw ptr for embedded protobuf)
    auto *details = new chk::payload::CapturePayload_TargetDetails();
    details->set_hunter_src_cell(copySrcCell);
    details->set_prey_cell_idx(copyPreyCell);
    details->set_prey_piece_id(copyPreyPieceId);

    // hunter landing cell
    auto *hunterDestCell = new chk::payload::CapturePayload_HunterDestination();
    hunterDestCell->set_cell_index(targetCell->getIndex());
    hunterDestCell->set_x(targetCell->getPos().x);
    hunterDestCell->set_y(targetCell->getPos().y);

    // prepare to send to server:
    auto *capturePayload = new chk::payload::CapturePayload();
    capturePayload->set_hunter_piece_id(copyHunterPiece);
    capturePayload->set_allocated_details(details);
    capturePayload->set_allocated_destination(hunterDestCell);
    capturePayload->set_from_team(TeamColor::TEAM_RED);
    if (this->myTeam == chk::PlayerType::PLAYER_BLACK)
    {
        capturePayload->set_from_team(TeamColor::TEAM_BLACK);
    }

    // ATTACH all children to root
    chk::payload::BasePayload basePayload;
    basePayload.set_allocated_capture_payload(capturePayload);
    if (!this->wsClient->replyServer(basePayload))
    {
        this->updateMessage("failed to send message to Server");
        return;
    }

    // Check for extra opportunities (for myself, single cell)! Skip if I just became King recently.
    this->forcedMoves.clear();
    if (isKingBefore == isKingNow)
    {
        GameManager::identifyTargets(hunter, targetCell);
    }

    if (this->getForcedMoves().empty())
    {
        // NO MORE JUMPS AVAILABLE. SWITCH TURNS to opponent
        chk::GameManager::identifyTargets(prey);
        this->isMyTurn = !this->isMyTurn;
        this->updateMessage("It's " + prey->getName() + "'s turn");
    }
    else
    {
        this->updateMessage("Continue. You can capture another piece!");
    }
}

/**
 * When current player taps any playable cell.
 * @param hunter currentPlayer
 * @param prey the opposing player
 * @param buffer Temporary store for clicked Piece
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
        // YES, it has one! VERIFY IF THERE IS ANY PENDING "forced captures".
        const bool notSelected = this->getForcedMoves().find(pieceId) == this->getForcedMoves().end();
        if (!this->getForcedMoves().empty() && notSelected)
        {
            this->showForcedMoves(hunter, cell);
            return;
        }
        // OTHERWISE, store it in buffer (for a simple move, on next turn)
        buffer.addItem(pieceId);
        this->setSourceCell(cell->getIndex());
    }
    else
    {
        // Cell is Empty! Let's evaluate if this is SIMPLE move or ATTACK move
        if (!buffer.isEmpty())
        {
            const short movablePieceId = buffer.getFront();
            if (!hunter->hasThisPiece(movablePieceId))
            {
                return;
            }
            else if (this->isHunterActive())
            {
                // it's an ATTACK
                this->handleCapturePiece(hunter, prey, cell);
                GameManager::updateMatchStatus(hunter, prey);
                buffer.clean();
            }
            else
            {
                // it's a SIMPLE move
                this->handleMovePiece(hunter, prey, cell, movablePieceId);
                buffer.clean();
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
        // which color is the Opponent?
        // clang-format off
        const chk::PlayerPtr &enemy = (payload.from_team() == TeamColor::TEAM_RED) ? this->playerRed : this->playerBlack;
        const chk::PlayerPtr &myTeam = (enemy->getPlayerType() == PlayerType::PLAYER_RED) ? this->playerBlack : this->playerRed;
        // clang-format on
        const auto targetPosition = sf::Vector2f{payload.destination().x(), payload.destination().y()};
        const short movingPieceId = static_cast<short>(payload.piece_id());
        if (!enemy->movePiece(movingPieceId, targetPosition))
        {
            return;
        }
        this->gameMap.erase(payload.source_cell());                               // set old location empty!
        this->gameMap.emplace(payload.destination().cell_index(), movingPieceId); // fill in the new location

        // check for opportunities (for MYSELF)
        GameManager::identifyTargets(myTeam);
        if (!this->getForcedMoves().empty())
        {
            spdlog::info("OPPONENT IS IN DANGER");
        }

        this->isMyTurn = !this->isMyTurn; // toggle player turns
        this->updateMessage("Opponent moved to " + std::to_string(payload.destination().cell_index()) +
                            ". It's your turn.");
    });
}

/**
 * Listening for CapturePiece events from server, and update gameBoard
 */
inline void OnlineGameManager::startCaptureListener()
{
    this->wsClient->setOnCapturePieceCallback([this](const chk::payload::CapturePayload &payload) {
        bool isKingBefore = false;
        bool isKingNow = false;
        // clang-format off
        const chk::PlayerPtr &opponent = payload.from_team() == TeamColor::TEAM_RED ? this->playerRed : this->playerBlack;
        const chk::PlayerPtr &myTeam = opponent->getPlayerType() == PlayerType::PLAYER_RED ? this->playerBlack : this->playerRed;
        // clang-format on
        const auto destPos = sf::Vector2f{payload.destination().x(), payload.destination().y()};
        const auto hunterPieceId = static_cast<short>(payload.hunter_piece_id());

        isKingBefore = opponent->getOwnPieces().at(hunterPieceId)->getIsKing();
        if (!opponent->captureEnemyWith(hunterPieceId, destPos))
        {
            return;
        }

        this->updateMessage(opponent->getName() + " has captured your piece!");
        isKingNow = opponent->getOwnPieces().at(hunterPieceId)->getIsKing(); // update changes for hunter piece
        gameMap.erase(payload.details().hunter_src_cell());                  // set opponent's old location empty!
        gameMap.erase(payload.details().prey_cell_idx());                    // set my old location empty!
        gameMap.emplace(payload.destination().cell_index(), hunterPieceId);  // fill in hunter new location
        const int targetId = payload.details().prey_piece_id();              // get the target dead piece
        myTeam->losePiece(static_cast<short>(targetId));                     // I will lose one piece

        const int destCellIdx = payload.destination().cell_index();
        const auto it = std::find_if(blockList.begin(), blockList.end(), [&destCellIdx](const chk::Block &cell) {
            return cell->getIndex() == destCellIdx;
        });

        // Check for extra opportunities (for Enemy), only if Enemy did NOT just become King
        this->forcedMoves.clear();
        if ((isKingBefore == isKingNow) && it != this->blockList.end())
        {
            GameManager::identifyTargets(opponent, *it);
        }

        if (this->getForcedMoves().empty())
        {
            // NO MORE JUMPS AVAILABLE. SWITCH TURNS to myself.
            chk::GameManager::identifyTargets(myTeam);
            this->isMyTurn = !this->isMyTurn;
            this->updateMessage("It's now your turn!");
        }
    });
}

/**
 * Listening for killed connections & Winner notifications from server
 */
inline void OnlineGameManager::startDeathListener()
{
    this->wsClient->setOnDeathCallback([this](std::string_view notice) {
        this->updateMessage(notice);
        this->doCleanup();
        this->isMyTurn = false;
        this->gameReady = false;
    });

    this->wsClient->setOnWinLoseCallback([this](std::string_view notice) {
        this->updateMessage(notice);
        this->doCleanup();
        this->isMyTurn = false;
        this->gameReady = false;
    });
}

} // namespace chk
