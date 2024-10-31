//
// Base implementation of Abstract Game Manager
// Created by Davis on 2023/12/21.
//
#include "../GameManager.hpp"

using namespace chk;

/**
 * Get hashmap of hunter pieceID's to the assigned CaptureTarget
 * @return )pair of forced captures
 */
[[nodiscard]] const std::unordered_map<short, chk::CaptureTarget> &GameManager::getForcedMoves() const
{
    return this->forcedMoves;
}

/**
 * Atomically update main UI message
 * @param msg the message content
 */
void GameManager::updateMessage(const std::string_view msg)
{
    std::scoped_lock<std::mutex> lg(my_mutex);
    this->currentMsg = msg;
}

/**
 * Get current message passed from Main
 * @return string value of message
 */
const std::string &GameManager::getCurrentMsg() const
{
    return this->currentMsg;
}

/**
 * /brief Get list of all checkerboard cells
 * /return vector of unique_ptr of Cells
 */
const std::vector<chk::Block> &GameManager::getBlockList() const
{
    return this->blockList;
}

/**
 * Create checkerboard cells, labeled with an index using given font
 * @param font used for text labels
 */
void GameManager::drawCheckerboard(const sf::Font &font)
{
    int counter = 32;
    for (uint16_t row = 0; row < NUM_ROWS; ++row)
    {
        for (uint16_t col = 0; col < NUM_COLS; ++col)
        {
            if ((row + col) % 2 == 0)
            {
                // even CELL, set LIGHTER color (unused)
                sf::RectangleShape lightRec(sf::Vector2f(chk::SIZE_CELL, chk::SIZE_CELL));
                lightRec.setFillColor(sf::Color{255, 225, 151});
                float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                lightRec.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                auto whiteBlock = std::make_unique<chk::Cell>(-1, lightRec, lightRec.getPosition(), font);
                blockList.emplace_back(std::move_if_noexcept(whiteBlock));
            }
            else
            {
                // Odd cell, SET DARKER color (USED BY PIECES)
                sf::RectangleShape darkRect(sf::Vector2f(chk::SIZE_CELL, chk::SIZE_CELL));
                darkRect.setFillColor(sf::Color{82, 55, 27});
                float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                darkRect.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                auto redBlock = std::make_unique<chk::Cell>(counter, darkRect, darkRect.getPosition(), font);
                redBlock->setEvenRow(row % 2 == 0);
                blockList.emplace_back(std::move_if_noexcept(redBlock));
                counter--;
            }
        }
    }
}

/**
 * Move the selected piece to clicked cell, and update the gameMap
 * @param player current player
 * @param opponent opposing player
 * @param destCell target cell
 * @param currentPieceId the selected PieceId
 */
void GameManager::handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                                  const short &currentPieceId)
{
    if (this->gameOver)
    {
        return;
    }
    // VERIFY if move is successful
    const bool success = player->movePiece(currentPieceId, destCell->getPos());
    if (!success)
    {
        return;
    }
    gameMap.erase(this->sourceCell.value());               // set old location empty!
    gameMap.emplace(destCell->getIndex(), currentPieceId); // fill in the new location
    this->sourceCell = std::nullopt;                       // reset source cell
    this->identifyTargets(opponent);                       // check  opportunities for Opponent

    if (!this->forcedMoves.empty())
    {
        spdlog::info(player->getName() + " IS IN DANGER ");
    }
    this->playerRedTurn = !this->playerRedTurn; // toggle player turns
    this->updateMessage(player->getName() + " has moved to " + std::to_string(destCell->getIndex()) + ". It's " +
                        opponent->getName() + "'s turn.");
}

/**
 * Perform capturing of "prey's" pieces by "hunter", then update gameMap
 * @param hunter the attacking player
 * @param prey the defensive player
 * @param targetCell the destination of hunter
 */
void GameManager::handleCapturePiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                     const chk::Block &targetCell)
{

    assert(!(*hunter == *prey) && "cannot pass the same player");
    if (this->gameOver || this->getPieceFromCell(targetCell->getIndex()) != -1)
    {
        // STOP if game over OR there's already a Piece on target cell
        return;
    }
    bool isKingBefore = false;
    bool isKingNow = false;

    bool isCaptured = false; // outside guard to verify if Capture completed
    for (const auto &[hunterPieceId, target] : this->forcedMoves)
    {
        if (target.hunterNextCell == targetCell->getIndex())
        {
            isKingBefore = hunter->getOwnPieces().at(hunterPieceId)->getIsKing(); // before capture
            if (!hunter->captureEnemyWith(hunterPieceId, targetCell->getPos()))
            {
                return;
            }
            isCaptured = true;
            this->updateMessage(hunter->getName() + " has captured " + prey->getName() + "'s piece!");
            gameMap.erase(this->sourceCell.value());                           // set hunter's old location empty!
            gameMap.erase(target.preyCellIdx);                                 // set Prey's old location empty!
            gameMap.emplace(targetCell->getIndex(), hunterPieceId);            // fill in hunter new location
            prey->losePiece(target.preyPieceId);                               // the defending player loses 1 piece
            this->sourceCell = std::nullopt;                                   // reset source cell
            isKingNow = hunter->getOwnPieces().at(hunterPieceId)->getIsKing(); // track changes after capture
            break;
        }
    }
    if (!isCaptured)
    {
        return;
    }
    //  Check for extra opportunities (only if hunter has NOT just became KING)
    this->forcedMoves.clear();
    if (isKingBefore == isKingNow)
    {
        GameManager::identifyTargets(hunter, targetCell);
    }

    if (this->forcedMoves.empty())
    {
        // NO MORE JUMPS AVAILABLE. SWITCH TURNS to opponent
        this->identifyTargets(prey);
        this->playerRedTurn = !this->playerRedTurn;
    }
    else
    {
        spdlog::info(prey->getName() + " IS IN DANGER");
        this->updateMessage(prey->getName() + " IS IN DANGER");
    }
}

/**
 * Whether it's Red player's turn
 * @return TRUE or FALSE
 */
const bool &GameManager::isPlayerRedTurn() const
{
    return this->playerRedTurn;
}

/**
 * Store the cell idx from which the piece is LEAVING
 * @param src_cell index of the cell
 */
void GameManager::setSourceCell(int src_cell)
{
    this->sourceCell = src_cell;
}

/**
 * If match is interrupted, or game is over, reset all states
 */
void chk::GameManager::doCleanup()
{
    this->gameMap.clear();
    this->forcedMoves.clear();
    this->playerRed->emptyBasket();
    this->playerBlack->emptyBasket();
    this->gameOver = true;
    this->alreadyCached = false;
}

/**
 * Returns TRUE only if the current player is holding own hunting Piece, AND forcedMoves is NOT empty
 *
 *@return TRUE or FALSE
 */
bool GameManager::isHunterActive() const
{
    if (!this->sourceCell.has_value())
    {
        return false;
    }
    const short pieceId = this->getPieceFromCell(this->sourceCell.value()); // hunter pieceId
    return pieceId != -1 && (forcedMoves.find(pieceId) != forcedMoves.end());
}

/**
 * Using cached gameMap, get the PieceId placed at this cell_index
 *
 * @param cell_idx the clicked cell
 * @return positive number or -1 if not found
 */
short GameManager::getPieceFromCell(const int cell_idx) const
{
    if (this->gameMap.find(cell_idx) != gameMap.end())
    {
        return gameMap.at(cell_idx);
    }
    return -1;
}

/**
 * Match cells to pieces at game launch, using position, and cache it to Hashmap
 * @param pieceList vector of all pieces
 */
void GameManager::matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList)
{
    if (this->alreadyCached)
    {
        return;
    }
    for (const auto &piece : pieceList)
    {
        for (const auto &cell : this->blockList)
        {
            if (cell->getIndex() != -1 && cell->isAtPosition(piece->getPosition()))
            {
                this->gameMap.emplace(cell->getIndex(), piece->getId());
            }
        }
    }
    this->alreadyCached = true;
    spdlog::info("gamMap size {}", std::to_string(gameMap.size()));
}

/**
 * Checks piece count for both players (in any order), then updates match status
 * @param p1 first player
 * @param p2 second player
 */
void GameManager::updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2)
{
    assert(!(*p1 == *p2) && "cannot pass same Player");
    const auto p1Count = p1->getPieceCount();
    const auto p2Count = p2->getPieceCount();
    if (p1Count == 0 || p2Count == 0)
    {
        this->gameOver = true;
        const std::string &winnerName = p1Count > p2Count ? p1->getName() : p2->getName();
        this->updateMessage("GAME OVER! " + winnerName + " wins!");
    }
}

/**
 * When current player taps any playable cell.
 * @param hunter currentPlayer
 * @param prey the opposing player
 * @param buffer Temporary store for clicked Pieces
 * @param cell Tapped cell
 */
void chk::GameManager::handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                     chk::CircularBuffer<short> &buffer, const chk::Block &cell)
{
    if (this->isGameOver())
    {
        return;
    }
    // CHECK IF this cell has a Piece
    const short pieceId = this->getPieceFromCell(cell->getIndex());
    if (pieceId != -1)
    {
        // YES, it has one! VERIFY IF THERE IS ANY PENDING "forced captures", if yes, verify hunter SELECTED
        if (!this->getForcedMoves().empty() && this->forcedMoves.find(pieceId) == forcedMoves.end())
        {
            this->showForcedMoves(hunter, cell);
            return;
        }
        // OTHERWISE, store it in buffer (for a SIMPLE/CAPTURE move next)!
        buffer.addItem(pieceId);
        this->setSourceCell(cell->getIndex());
    }
    else
    {
        // Cell is Empty! Let's judge if this is SIMPLE move or ATTACK move
        if (!buffer.isEmpty())
        {
            const short movablePieceId = buffer.getTop();
            if (!hunter->hasThisPiece(movablePieceId))
            {
                return;
            }
            else if (isHunterActive())
            {
                // it's an ATTACK move
                this->handleCapturePiece(hunter, prey, cell);
                this->updateMatchStatus(hunter, prey);
                buffer.clean();
            }
            else
            {
                // it's a SIMPLE MOVE
                this->handleMovePiece(hunter, prey, cell, movablePieceId);
                buffer.clean();
            }
        }
    }
}

/**
 * When player is forced to capture opponent's piece, highlight their hunter pieces with GREEN.
 * @param player current player
 * @param cell selected destination cell
 */
void chk::GameManager::showForcedMoves(const chk::PlayerPtr &player, const chk::Block &cell)
{
    const auto &moves = this->getForcedMoves();
    const short pieceId = this->getPieceFromCell(cell->getIndex());
    if (moves.find(pieceId) == moves.end())
    {
        // FORCE PLAYER TO CAPTURE opponent, don't proceed until done!
        std::set<short> pieceSet;
        for (const auto &[hunter_piece, captureTarget] : moves)
        {
            pieceSet.emplace(hunter_piece);
        }
        player->showForcedPieces(pieceSet);
        this->updateMessage(player->getName() + " must capture piece!");
    }
    else
    {
        // GOOD! user has selected own Hunter Piece
        this->setSourceCell(cell->getIndex());
    }
}

/**
 * Whether game is over
 * @return TRUE or FALSE
 */
const bool GameManager::isGameOver() const
{
    return this->gameOver;
}

/**
 * Whether the game board contains this cell, AND is within playable range
 * @param cell_idx Cell index
 * @return TRUE if cell on board, else FALSE
 */
bool GameManager::boardContainsCell(const int cell_idx) const
{
    const auto it = std::find_if(blockList.begin(), blockList.end(), [&cell_idx](const chk::Block &cell) {
        return cell->getIndex() == cell_idx && cell->getPos().x >= 0 && cell->getPos().x <= 7 * chk::SIZE_CELL &&
               cell->getPos().y >= 0 && cell->getPos().y <= 7 * chk::SIZE_CELL;
    });
    return it != blockList.end();
}

/**
 * Whether the given cell index is NOT on any edge of board
 * @param cell_idx cell index
 * @return TRUE if NOT on edges, else FALSE
 */
bool GameManager::awayFromEdge(const int cell_idx) const
{
    const auto it = std::find_if(blockList.begin(), blockList.end(), [&cell_idx](const chk::Block &cell) {
        return cell->getIndex() == cell_idx && cell->getPos().x > 0 && cell->getPos().x < 7 * chk::SIZE_CELL &&
               cell->getPos().y > 0 && cell->getPos().y < 7 * chk::SIZE_CELL;
    });
    return it != blockList.end();
}

/**
 * Collect all possible next "forced captures" for this hunter.
 * @param hunter Current player
 * @param singleCell if not NULL, only check around this cell. Otherwise, loop ENTIRE board
 */
void GameManager::identifyTargets(const PlayerPtr &hunter, const chk::Block &singleCell)
{
    this->forcedMoves.clear();
    if (singleCell != nullptr)
    {
        // JUST CHECK SINGLE CELL
        const short pieceId = this->getPieceFromCell(singleCell->getIndex());
        if (gameMap.find(singleCell->getIndex()) == gameMap.end() || !hunter->hasThisPiece(pieceId))
        {
            // this CELL is not usable, OR piece not OWNED by hunter
            return;
        }
        this->collectFrontLHS(hunter, singleCell);
        this->collectFrontRHS(hunter, singleCell);
        const auto &piecePtr = hunter->getOwnPieces().at(pieceId);
        if (piecePtr->getIsKing())
        {
            this->collectBehindLHS(hunter, singleCell);
            this->collectBehindRHS(hunter, singleCell);
        }
    }
    else
    {
        // LOOP ENTIRE BOARD
        for (const auto &cell_ptr : this->blockList)
        {
            const short pieceId = this->getPieceFromCell(cell_ptr->getIndex());
            if (gameMap.find(cell_ptr->getIndex()) == gameMap.end() || !hunter->hasThisPiece(pieceId))
            {
                // same reason as previous code-block
                continue;
            }
            this->collectFrontLHS(hunter, cell_ptr);
            this->collectFrontRHS(hunter, cell_ptr);
            const auto &piecePtr = hunter->getOwnPieces().at(pieceId);
            if (piecePtr->getIsKing())
            {
                this->collectBehindLHS(hunter, cell_ptr);
                this->collectBehindRHS(hunter, cell_ptr);
            }
        }
    }
}

/**
 * Collect nearby enemies of Hunter for next "forced" captures (NORTH WEST)
 * @param hunter  player whose turn is next
 * @param cell_ptr current cell of hunter
 */
void GameManager::collectFrontLHS(const chk::PlayerPtr &hunter, const chk::Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_RED && cell_ptr->getPos().x == 0)
    {
        // IMPOSSIBLE to have enemies on my Left.
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // same as above, based on my current "X" position.
        return;
    }
    bool enemyOpenBehind = false; // does enemy piece have EMPTY cell behind it?
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 4 : 5;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 5 : 4;
    int mSign = +1; // direction. up +1, down -1

    // if player piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    int cellAheadIdx = cell_ptr->getIndex() + (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }

    const short pieceId_NW = this->getPieceFromCell(cellAheadIdx); // North West (of hunter)
    hasEnemyAhead = pieceId_NW != -1 && !hunter->hasThisPiece(pieceId_NW);

    const int cellBehindEnemy = cell_ptr->getIndex() + (deltaBehindEnemy * mSign) + (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }

    const short pieceId_SE = this->getPieceFromCell(cellBehindEnemy); // South East (of enemy)
    enemyOpenBehind = pieceId_SE == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        chk::CaptureTarget cf;
        cf.preyPieceId = pieceId_NW;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        this->forcedMoves.emplace(myPieceId, cf);
    }
}

/**
 * Collect nearby enemies of Hunter for next "forced" captures (NORTH EAST)
 * @param hunter player whose turn is next
 * @param cell_ptr current cell of hunter
 */
void GameManager::collectFrontRHS(const chk::PlayerPtr &hunter, const chk::Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_RED && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // IMPOSSIBLE to have enemies on my Right.
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK && cell_ptr->getPos().x == 0)
    {
        // same as above, based on my current "X" position.
        return;
    }
    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 3 : 4;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 4 : 3;
    int mSign = +1; // direction. up +1, down -1

    // if piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    const int cellAheadIdx = cell_ptr->getIndex() + (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }
    const short pieceId_NE = this->getPieceFromCell(cellAheadIdx); // North East of hunter
    hasEnemyAhead = pieceId_NE != -1 && !hunter->hasThisPiece(pieceId_NE);

    const int cellBehindEnemy = cell_ptr->getIndex() + (deltaBehindEnemy * mSign) + (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    const short pieceId_SW = this->getPieceFromCell(cellBehindEnemy); // South West (of enemy)
    enemyOpenBehind = pieceId_SW == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        chk::CaptureTarget cf;
        cf.preyPieceId = pieceId_NE;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        this->forcedMoves.emplace(myPieceId, cf);
    }
}

/**
 * Collect nearby enemies of Hunter for next "forced" captures (SOUTH EAST). Only for KING pieces
 * @param hunter  player whose turn is next (MUST be King)
 * @param cell_ptr current cell of hunter
 */
void GameManager::collectBehindRHS(const PlayerPtr &hunter, const chk::Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_RED && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK && cell_ptr->getPos().x == 0)
    {
        return;
    }

    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 5 : 4;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 4 : 5;
    int mSign = +1; // direction. up +1, down -1

    // if piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    const int cellAheadIdx = cell_ptr->getIndex() - (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }

    const short pieceId_NW = this->getPieceFromCell(cellAheadIdx); // North west (of hunter, reverse dir)
    hasEnemyAhead = pieceId_NW != -1 && !hunter->hasThisPiece(pieceId_NW);

    int cellBehindEnemy = cell_ptr->getIndex() - (deltaBehindEnemy * mSign) - (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    const short pieceId_SE = this->getPieceFromCell(cellBehindEnemy); // South East of enemy
    enemyOpenBehind = pieceId_SE == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        chk::CaptureTarget cf;
        cf.preyPieceId = pieceId_NW;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        this->forcedMoves.emplace(myPieceId, cf);
    }
}

/**
 * Collect nearby enemies for next "forced" captures (SOUTH WEST). Only for KING pieces
 * @param hunter  player whose turn is next
 * @param cell_ptr current cell of hunter
 */
void GameManager::collectBehindLHS(const PlayerPtr &hunter, const chk::Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_RED && cell_ptr->getPos().x == 0)
    {
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        return;
    }
    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 4 : 3;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 3 : 4;
    int mSign = +1; // direction. up +1, down -1

    // if piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_BLACK)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    const int cellAheadIdx = cell_ptr->getIndex() - (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }
    const short pieceId_NE = this->getPieceFromCell(cellAheadIdx); // North east (of hunter, reverse dir)
    hasEnemyAhead = pieceId_NE != -1 && !hunter->hasThisPiece(pieceId_NE);

    int cellBehindEnemy = cell_ptr->getIndex() - (deltaBehindEnemy * mSign) - (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    const short pieceId_SW = this->getPieceFromCell(cellBehindEnemy); // south west of enemy
    enemyOpenBehind = pieceId_SW == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        chk::CaptureTarget cf;
        cf.preyPieceId = pieceId_NE;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        this->forcedMoves.emplace(myPieceId, cf);
    }
}
