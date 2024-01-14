//
// Created by Davis on 2023/12/21.
//
#include "GameManager.hpp"
using namespace chk;

/**
 * Default constructor
 */
GameManager::GameManager()
{
    this->sourceCell = -1;
    this->cTarget = std::make_unique<CaptureTarget>();
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
}

/**
 * Get pair of pieceID's forced to JUMP to matching cellIndex
 * @return pair of PieceId--> cellIndex
 */
[[nodiscard]] const std::unordered_map<uint16_t, int> &GameManager::getForcedMoves() const
{
    return this->forcedMoves;
}

/**
 * Update main window message
 */
void GameManager::updateMessage(const std::string &msg)
{
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
 * Create new checker pieces, each with own position, and add them to given vector
 * @param pieceList destination
 */
void GameManager::drawAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    std::random_device randomDevice;
    std::mt19937 randEngine(randomDevice());
    std::uniform_int_distribution<uint16_t> dist(1, std::numeric_limits<short>::max());
    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                sf::CircleShape circle(0.5 * chk::SIZE_CELL);
                const float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                circle.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                if (row < 3)
                {
                    // Half Top cells, put BLACK piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(kete));
                }
                else if (row > 4)
                {
                    // Half Bottom cells, put RED piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(kete));
                }
            }
        }
    }
}

/**
 * Move the selected piece to clicked cell, and update the gameMap
 * @param player current player
 * @param destCell target cell
 * @param currentPieceId the selected PieceId
 */
void GameManager::handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &destCell,
                                  const short &currentPieceId)
{
    if (this->gameOver)
        return;
    // VERIFY if move is completed
    const bool success = player->movePiece(currentPieceId, destCell->getPos());
    if (!success)
    {
        return;
    }
    gameMap.erase(this->sourceCell);                       // set old location empty!
    gameMap.emplace(destCell->getIndex(), currentPieceId); // fill in the new location
    this->playerRedTurn = !this->playerRedTurn;            // toggle player turns
    this->sourceCell = -1;
    const auto dangerRight = this->checkDangerRHS(player, destCell);
    const auto dangerLeft = this->checkDangerLHS(player, destCell);
    if (dangerRight || dangerLeft)
    {
        // FIXME check for opportunities the whole board
        this->cTarget->preyPieceId = currentPieceId;
        this->cTarget->preyCellIdx = destCell->getIndex();
        std::cout << player->getName() << " is in DANGER!" << std::endl;
        this->updateMessage(player->getName() + " is in DANGER!");
    }
    // player->identifyTarget(this->gameMap, cell_ptr);
}

/**
 * Handle capturing of "Prey's" pieces by "Hunter", then update gameMap
 * @param hunter the offensive player
 * @param prey the defensive player
 * @param targetCell the destination of hunter
 */
void GameManager::handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                  const chk::Block &targetCell)
{
    // If There's a PIECE on target cell, Cannot Jump to it.
    if (this->gameOver || this->getPieceFromCell(targetCell->getIndex()) != -1)
        return;

    for (const auto &[hunterPieceId, cell_idx] : this->forcedMoves)
    {
        if (cell_idx == targetCell->getIndex())
        {
            if (!hunter->captureEnemyWith(hunterPieceId, targetCell->getPos()))
            {
                return;
            }
            gameMap.erase(this->sourceCell);                        // set hunter's old location empty!
            gameMap.erase(this->cTarget->preyCellIdx);              // set Prey's old location empty!
            gameMap.emplace(targetCell->getIndex(), hunterPieceId); // fill in hunter new location
            prey->losePiece(this->cTarget->preyPieceId);            // the defending player loses 1 piece
            // FIXME check for more opportunities NOW before toggle Turns
            this->playerRedTurn = !this->playerRedTurn; // toggle player turns
            this->sourceCell = -1;
            this->cTarget->preyCellIdx = -1;
            this->cTarget->preyPieceId = -1;
            this->forcedMoves.clear();
            // check for other dangers ahead
            const auto dangerRight = this->checkDangerRHS(hunter, targetCell);
            const auto dangerLeft = this->checkDangerLHS(hunter, targetCell);
            if (dangerRight || dangerLeft)
            {
                this->cTarget->preyCellIdx = targetCell->getIndex();
                this->cTarget->preyPieceId = gameMap.at(targetCell->getIndex());
                std::cout << hunter->getName() << " is in DANGER!" << std::endl;
                this->updateMessage(hunter->getName() + " is in DANGER!");
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
 * /brief Whether both src_cell is NOT NULL & forcedMoves NOT empty
 * /return TRUE or FALSE
 */
bool GameManager::isReadyForCapture() const
{
    return this->sourceCell != -1 && !forcedMoves.empty();
}
/**
 * check if player who has just moved is in danger (North west)
 * @param player recent pLayer
 * @param destCell the just moved-in cell
 */
bool GameManager::checkDangerLHS(const chk::PlayerPtr &player, const Block &destCell)
{
    bool hasEmptyCellBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = destCell->getIsEvenRow() ? 4 : 5;
    short deltaBehind = destCell->getIsEvenRow() ? 5 : 4;
    int mSign = 1;

    if (destCell->getPos().x == 0 || destCell->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // SAFE at either  edges
        return false;
    }
    if (destCell->getPos().y == 0 || destCell->getPos().y == 7 * chk::SIZE_CELL)
    {
        return false;
    }

    // if its player Black (PLAYER 2)
    if (player->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    int cellAheadIdx = destCell->getIndex() + (deltaForward * mSign);
    short pieceId_NW = this->getPieceFromCell(cellAheadIdx);
    hasEnemyAhead = pieceId_NW != -1 && !player->hasThisPiece(pieceId_NW);

    int cellBelowRight = destCell->getIndex() - (deltaBehind * mSign);
    short pieceId_SE = this->getPieceFromCell(cellBelowRight);
    hasEmptyCellBehind = pieceId_SE == -1;

    bool inDanger = false;
    if (hasEnemyAhead && hasEmptyCellBehind)
    {
        inDanger = true;
        this->forcedMoves.emplace(pieceId_NW, cellBelowRight);
    }
    return inDanger;
}

/**
 * check if player who has just moved is in danger (North East)
 * @param player recent pLayer
 * @param destCell the just moved-in cell
 */
bool GameManager::checkDangerRHS(const chk::PlayerPtr &player, const Block &destCell)
{
    bool hasEmptyCellBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = destCell->getIsEvenRow() ? 3 : 4;
    short deltaBehind = destCell->getIsEvenRow() ? 4 : 3;
    int mSign = 1;

    if (destCell->getPos().x == 0 || destCell->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // SAFE at either edges
        return false;
    }
    if (destCell->getPos().y == 0 || destCell->getPos().y >= 7 * chk::SIZE_CELL)
    {
        return false;
    }

    // if is player Black (PLAYER 2)
    if (player->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    int cellAheadIdx = destCell->getIndex() + (deltaForward * mSign);
    short pieceId_NE = this->getPieceFromCell(cellAheadIdx); // north East
    hasEnemyAhead = pieceId_NE != -1 && !player->hasThisPiece(pieceId_NE);

    int cellBelowLeft = destCell->getIndex() - (deltaBehind * mSign);
    int pieceId_SW = this->getPieceFromCell(cellBelowLeft); // south West
    hasEmptyCellBehind = pieceId_SW == -1;

    bool inDanger = false;
    if (hasEnemyAhead && hasEmptyCellBehind)
    {
        inDanger = true;
        this->forcedMoves.emplace(pieceId_NE, cellBelowLeft);
    }
    return inDanger;
}

/**
 * Using cached hashmap, get the PieceId placed at this cell_index
 *
 * @param cell_idx the clicked cell
 * @return positive int or -1 if not found
 */
inline short GameManager::getPieceFromCell(const int &cell_idx)
{
    if (this->gameMap.find(cell_idx) != gameMap.end())
    {
        return gameMap.at(cell_idx);
    }
    return -1;
}

/**
 * Match cells to pieces at game launch, using location, and cache it to Hashmap
 * @param pieceList vector of all pieces
 */
void GameManager::matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList)
{
    if (this->alreadyCached)
    {
        return;
    }
    std::ofstream fout{"D:/work_mine/CPP/space-checkers/game_map.txt"};
    if (!fout.good())
    {
        perror("cannot create file");
        exit(EXIT_FAILURE);
    }
    for (const auto &piece : pieceList)
    {
        for (const auto &cell : this->blockList)
        {
            if (cell->getIndex() != -1 && cell->containsOrigin(piece->getPosition()))
            {
                this->gameMap.emplace(cell->getIndex(), piece->getId());
                const std::string playerName = piece->getPieceType() == PieceType::Red ? "RED" : "BLACK";
                fout << cell->getIndex() << "->" << piece->getId() << "\t " + playerName << "\n";
            }
        }
    }
    this->alreadyCached = true;
    fout << std::endl;
    fout.close();
    std::cout << "hashmap size " << gameMap.size() << std::endl;
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
        throw std::exception("cannot pass same Player");
    }
    const auto p1Count = p1->getPieceCount();
    const auto p2Count = p2->getPieceCount();
    if (p1Count == 0 || p2Count == 0)
    {
        this->gameOver = true;
        const std::string winnerName = p1Count > p2Count ? p1->getName() : p2->getName();
        this->updateMessage("GAME OVER! " + winnerName + " won");
    }
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
 * Whether the game board contains this cell
 * @param cell_idx Cell index
 * @return TRUE if cell on board, else FALSE
 */
bool GameManager::boardContainsCell(const int &cell_idx) const
{
    return this->gameMap.find(cell_idx) != gameMap.end();
}

/**
 * Collect all possible next "forced captures" which must be taken
 * @param hunter current player
 */
inline void GameManager::identifyTargets(const PlayerPtr &hunter)
{
    for (const auto &cell_ptr : this->blockList)
    {
        short pieceId = this->getPieceFromCell(cell_ptr->getIndex());
        if (gameMap.find(cell_ptr->getIndex()) == gameMap.end() || !hunter->hasThisPiece(pieceId))
        {
            // this CELL is not usable, or piece not OWNED by this Player!
            continue;
        }
        // FIXME for each piece, only 1 captureTarget struct used.
        this->collectFrontLHS(hunter, cell_ptr);
        this->collectFrontRHS(hunter, cell_ptr);
        const auto &piecePtr = hunter->getOwnPieces().at(pieceId);
        //        if(piecePtr->getIsKing()) {
        //            this->collectBehindLHS(hunter, cell_ptr);
        //            this->collectBehindRHS(hunter, cell_ptr);
        //        }
    }
}

/**
 * Collect nearby enemies for next "forced" captures (Front RIGHT)
 * @param hunter player whose turn is next
 * @param cell_ptr current cell
 */
void GameManager::collectFrontRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr)
{
    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 3 : 4;
    short deltaBehind = cell_ptr->getIsEvenRow() ? 4 : 3;
    int mSign = 1;

    // if is player Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    int cellAheadIdx = cell_ptr->getIndex() + (deltaForward * mSign);
    if (!this->boardContainsCell(cellAheadIdx))
    {
        return;
    }
    short pieceId_NE = this->getPieceFromCell(cellAheadIdx); // North East
    hasEnemyAhead = pieceId_NE != -1 && !hunter->hasThisPiece(pieceId_NE);

    int cellBehindEnemy = cell_ptr->getIndex() - (deltaBehind * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }
    short pieceId_SW = this->getPieceFromCell(cellBehindEnemy); // south West of enemy
    enemyOpenBehind = pieceId_SW == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        CaptureTarget cf;
        cf.preyCellIdx = pieceId_NE;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        hunter->appendNextTarget(std::make_pair(myPieceId, cf));
    }
}

/**
 *  Collect nearby enemies for next "forced" captures (Front LEFT)
 * @param hunter  player whose turn is next
 * @param cell_ptr current cell
 */
void GameManager::collectFrontLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr)
{
    bool enemyOpenBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = cell_ptr->getIsEvenRow() ? 4 : 5;
    short deltaBehind = cell_ptr->getIsEvenRow() ? 5 : 4;
    int mSign = 1;

    // if its hunter Black (PLAYER 2)
    if (hunter->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    int cellAheadIdx = cell_ptr->getIndex() + (deltaForward * mSign);
    if (!this->boardContainsCell(cellAheadIdx))
    {
        return;
    }

    short pieceId_NW = this->getPieceFromCell(cellAheadIdx); // North West
    hasEnemyAhead = pieceId_NW != -1 && !hunter->hasThisPiece(pieceId_NW);

    int cellBehindEnemy = cell_ptr->getIndex() - (deltaBehind * mSign);
    if (!this->boardContainsCell(cellBehindEnemy))
    {
        return;
    }

    short pieceId_SE = this->getPieceFromCell(cellBehindEnemy); // South East of enemy
    enemyOpenBehind = pieceId_SE == -1;

    if (hasEnemyAhead && enemyOpenBehind)
    {
        CaptureTarget cf;
        cf.preyCellIdx = pieceId_NW;
        cf.preyCellIdx = cellAheadIdx;
        cf.hunterNextCell = cellBehindEnemy;
        const auto myPieceId = this->getPieceFromCell(cell_ptr->getIndex());
        hunter->appendNextTarget(std::make_pair(myPieceId, cf));
    }
}
