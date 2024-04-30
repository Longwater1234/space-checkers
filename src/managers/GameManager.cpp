//
// Created by Davis on 2023/12/21.
//
#include "../GameManager.hpp"
using namespace chk;

/**
 * Custom constructor
 */
GameManager::GameManager(sf::RenderWindow *window)
{
    this->window = window;
    this->sourceCell = -1;
    this->forcedMoves.clear();
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
}

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
void GameManager::updateMessage(std::string_view msg)
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
    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
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
 * @param opponent oppponent
 * @param destCell target cell
 * @param currentPieceId the selected PieceId
 */
void GameManager::handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                                  const short &currentPieceId)
{
    if (this->gameOver)
        return;
    // VERIFY if move is successful
    const bool success = player->movePiece(currentPieceId, destCell->getPos());
    if (!success)
    {
        return;
    }
    gameMap.erase(this->sourceCell.value());               // set old location empty!
    gameMap.emplace(destCell->getIndex(), currentPieceId); // fill in the new location
    this->sourceCell = std::nullopt;                       // reset source cell
    this->identifyTargets(opponent);
    if (!this->forcedMoves.empty())
    {
        spdlog::info(player->getName() + " IS IN DANGER ");
    }
    if (this->_onMoveSuccess != nullptr)
    {
        // notify server
        _onMoveSuccess(currentPieceId, destCell->getIndex());
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
void GameManager::handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                  const chk::Block &targetCell)
{

    assert(!(hunter == prey) && "cannot be same player");
    if (this->gameOver || this->getPieceFromCell(targetCell->getIndex()) != -1)
    {
        // STOP if there's already a Piece on target cell
        return;
    }

    for (const auto &[hunterPieceId, target] : this->forcedMoves)
    {
        if (target.hunterNextCell == targetCell->getIndex())
        {
            if (!hunter->captureEnemyWith(hunterPieceId, targetCell->getPos()))
            {
                return;
            }
            this->updateMessage(hunter->getName() + " has captured " + prey->getName() + "'s piece!");
            gameMap.erase(this->sourceCell.value());                // set hunter's old location empty!
            gameMap.erase(target.preyCellIdx);                      // set Prey's old location empty!
            gameMap.emplace(targetCell->getIndex(), hunterPieceId); // fill in hunter new location
            prey->losePiece(target.preyPieceId);                    // the defending player loses 1 piece
            this->sourceCell = std::nullopt;                        // reset source cell
            this->forcedMoves.clear();                              // reset forced jumps

            // FIXME do not RUN this next line if just became KING!
            this->identifyTargets(hunter); // Check for extra opportunities NOW!
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
            break;
        }
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
void GameManager::setSourceCell(const int &src_cell)
{
    this->sourceCell = src_cell;
}

/**
 * Whether the current player is holding own hunting Piece, AND
 * is about to complete capturing opponent
 *
 *@return TRUE or FALSE
 */
bool GameManager::hasPendingCaptures() const
{
    return this->sourceCell.has_value() && !forcedMoves.empty();
}

/**
 * Using cached hashmap, get the PieceId placed at this cell_index
 *
 * @param cell_idx the clicked cell
 * @return positive int or -1 if not found
 */
short GameManager::getPieceFromCell(const int &cell_idx)
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
            if (cell->getIndex() != -1 && cell->containsOrigin(piece->getPosition()))
            {
                this->gameMap.emplace(cell->getIndex(), piece->getId());
            }
        }
    }
    this->alreadyCached = true;
    spdlog::info("hashmap size " + std::to_string(gameMap.size()));
}

/**
 * Checks piece count for both players (in any order), then updates match status
 * @param p1 first player
 * @param p2 second player
 */
void GameManager::updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2)
{
    if (*p1 == *p2)
    {
        throw std::invalid_argument("cannot pass same Player");
    }
    const auto p1Count = p1->getPieceCount();
    const auto p2Count = p2->getPieceCount();
    if (p1Count == 0 || p2Count == 0)
    {
        this->gameOver = true;
        const std::string &winnerName = p1Count > p2Count ? p1->getName() : p2->getName();
        this->updateMessage("GAME OVER! " + winnerName + " won");
    }
}

/**
 * Set callback for when piece is moved succesfully
 */
void chk::GameManager::setOnMoveSuccessCallback(const onMoveSuccessCallback &callback)
{
    this->_onMoveSuccess = callback;
}

/**
 * Whether game is over
 * @return TRUE or FALSE
 */
const bool &GameManager::isGameOver() const
{
    return this->gameOver;
}

/**
 * Whether the game board contains this cell, and is within playable range
 * @param cell_idx Cell index
 * @return TRUE if cell on board, else FALSE
 */
bool GameManager::boardContainsCell(const int &cell_idx) const
{
    const auto it = std::find_if(blockList.begin(), blockList.end(), [&cell_idx](const chk::Block &cell) {
        return cell->getIndex() == cell_idx && cell->getPos().x >= 0 && cell->getPos().x <= 7 * chk::SIZE_CELL &&
               cell->getPos().y >= 0 && cell->getPos().y <= 7 * chk::SIZE_CELL;
    });
    return it != blockList.end();
}

/**
 * \brief Whether the given cell index is NOT on any edge of board
 * \param cell_idx cell index
 * \return TRUE if NOT on edges, else FALSE
 */
bool GameManager::awayFromEdge(const int &cell_idx) const
{
    const auto it = std::find_if(blockList.begin(), blockList.end(), [&cell_idx](const chk::Block &cell) {
        return cell->getIndex() == cell_idx && cell->getPos().x > 0 && cell->getPos().x < 7 * chk::SIZE_CELL &&
               cell->getPos().y > 0 && cell->getPos().y < 7 * chk::SIZE_CELL;
    });
    return it != blockList.end();
}

/**
 * Collect all possible next "forced captures" for this hunter. Loop entire board once.
 * @param hunter Current player
 */
void GameManager::identifyTargets(const PlayerPtr &hunter)
{
    this->forcedMoves.clear();
    for (const auto &cell_ptr : this->blockList)
    {
        const short pieceId = this->getPieceFromCell(cell_ptr->getIndex());

        if (gameMap.find(cell_ptr->getIndex()) == gameMap.end() || !hunter->hasThisPiece(pieceId))
        {
            // this CELL is not usable, OR piece not OWNED by hunter
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

/**
 * Collect nearby enemies of Hunter for next "forced" captures (NORTH WEST)
 * @param hunter  player whose turn is next
 * @param cell_ptr current cell of hunter
 */
void GameManager::collectFrontLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_1 && cell_ptr->getPos().x == 0)
    {
        // IMPOSSIBLE to have enemies on my Left.
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_2 && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // same as above, based on my current "X" position.
        return;
    }
    bool enemyOpenBehind = false; // does enemy piece have EMPTY cell behind it?
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 4 : 5;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 5 : 4;
    int mSign = 1;

    // if player piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    int cellAheadIdx = cell_ptr->getIndex() + (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }

    short pieceId_NW = this->getPieceFromCell(cellAheadIdx); // North West
    hasEnemyAhead = pieceId_NW != -1 && !hunter->hasThisPiece(pieceId_NW);

    int cellBehindEnemy = cell_ptr->getIndex() + (deltaBehindEnemy * mSign) + (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }

    short pieceId_SE = this->getPieceFromCell(cellBehindEnemy); // South East
    enemyOpenBehind = pieceId_SE == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        CaptureTarget cf;
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
void GameManager::collectFrontRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_1 && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // IMPOSSIBLE to have enemies on my Right.
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_2 && cell_ptr->getPos().x == 0)
    {
        // same as above, based on my current "X" position.
        return;
    }
    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 3 : 4;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 4 : 3;
    int mSign = 1;

    // if piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    int cellAheadIdx = cell_ptr->getIndex() + (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }
    short pieceId_NE = this->getPieceFromCell(cellAheadIdx); // North East
    hasEnemyAhead = pieceId_NE != -1 && !hunter->hasThisPiece(pieceId_NE);

    int cellBehindEnemy = cell_ptr->getIndex() + (deltaBehindEnemy * mSign) + (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    short pieceId_SW = this->getPieceFromCell(cellBehindEnemy); // south West
    enemyOpenBehind = pieceId_SW == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        CaptureTarget cf;
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
void GameManager::collectBehindRHS(const PlayerPtr &hunter, const Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_1 && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_2 && cell_ptr->getPos().x == 0)
    {
        return;
    }

    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 5 : 4;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 4 : 5;
    int mSign = +1;

    // if piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    int cellAheadIdx = cell_ptr->getIndex() - (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }

    short pieceId_NW = this->getPieceFromCell(cellAheadIdx);
    hasEnemyAhead = pieceId_NW != -1 && !hunter->hasThisPiece(pieceId_NW);

    int cellBehindEnemy = cell_ptr->getIndex() - (deltaBehindEnemy * mSign) - (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    short pieceId_SE = this->getPieceFromCell(cellBehindEnemy);
    enemyOpenBehind = pieceId_SE == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        CaptureTarget cf;
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
void GameManager::collectBehindLHS(const PlayerPtr &hunter, const Block &cell_ptr)
{
    if (hunter->getPlayerType() == PlayerType::PLAYER_1 && cell_ptr->getPos().x == 0)
    {
        return;
    }
    if (hunter->getPlayerType() == PlayerType::PLAYER_2 && cell_ptr->getPos().x >= 7 * chk::SIZE_CELL)
    {
        return;
    }
    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 4 : 3;
    short deltaBehindEnemy = cell_ptr->getIsEvenRow() ? 3 : 4;
    int mSign = 1;

    // if piece is Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehindEnemy);
    }

    int cellAheadIdx = cell_ptr->getIndex() - (deltaForward * mSign);
    if (!this->awayFromEdge(cellAheadIdx))
    {
        return;
    }
    short pieceId_NE = this->getPieceFromCell(cellAheadIdx);
    hasEnemyAhead = pieceId_NE != -1 && !hunter->hasThisPiece(pieceId_NE);

    int cellBehindEnemy = cell_ptr->getIndex() - (deltaBehindEnemy * mSign) - (deltaForward * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    short pieceId_SW = this->getPieceFromCell(cellBehindEnemy);
    enemyOpenBehind = pieceId_SW == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        CaptureTarget cf;
        cf.preyPieceId = pieceId_NE;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        this->forcedMoves.emplace(myPieceId, cf);
    }
}
