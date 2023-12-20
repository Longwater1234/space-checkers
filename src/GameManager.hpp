//
// Created by Davis on 10/29/2023.
//
#pragma once

#include "CapturedPiece.hpp"
#include "Cell.hpp"
#include "Piece.hpp"
#include "Player.hpp"
#include <SFML/Graphics/Text.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace chk
{
using Block = std::unique_ptr<chk::Cell>;
using PlayerPtr = std::unique_ptr<chk::Player>;
constexpr size_t NUM_ROWS = 8;
constexpr size_t NUM_COLS = 8;

/**
 * Overall game state
 */
class GameManager
{

  public:
    GameManager();
    void drawCheckerboard(const sf::Font &font);
    static void drawAllPieces(std::vector<chk::PiecePtr> &pieceList);
    void updateMessage(const std::string &msg);
    void matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList);
    [[nodiscard]] const std::unordered_map<int, int> &getForcedMoves() const;
    [[nodiscard]] const std::string &getCurrentMsg() const;

  private:
    // source cell Index of selected piece
    int sourceCell;
    // keeps details about opponent about to be captured
    std::unique_ptr<CapturedPiece> capturedPiece;
    // checkerboard cells
    std::vector<chk::Block> blockList;
    // map of cell_index --> piece_id
    std::map<int, int> gameMap;
    // flag to check if cache is already filled
    bool alreadyCached = false;
    // whether it's player Red's turn
    bool playerRedTurn = true;
    // bottom display message
    std::string currentMsg;
    // Pair(hunterPieceId -> targetCell) For keeping records of pending "forced" captures
    std::unordered_map<int, int> forcedMoves;

  private:
    bool checkDangerLHS(const chk::PlayerPtr &player, const Block &destCell);
    bool checkDangerRHS(const chk::PlayerPtr &player, const Block &destCell);
    // bool checkForCaptureLHS(const std::unique_ptr<chk::Player> &player, const Block &destCell);
    // bool checkForCaptureRHS(const std::unique_ptr<chk::Player> &player, const Block &destCell);

  public:
    [[nodiscard]] bool isPlayerRedTurn() const;
    [[nodiscard]] inline int getPieceFromCell(const int &cell_idx);
    [[nodiscard]] const std::vector<chk::Block> &getBlockList() const;
    [[nodiscard]] bool isReadyForCapture() const;
    void setSourceCell(const int &src_cell);
    void handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &destCell, const int &currentPieceId);
    void handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, const chk::Block &targetCell);
};

inline GameManager::GameManager()
{
    this->sourceCell = -1;
    this->capturedPiece = std::make_unique<CapturedPiece>();
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
}

/**
 * Get pair of pieceID's forced to JUMP to matching cellIndex
 * @return a pair of PieceId--> cellIndex
 */
inline const std::unordered_map<int, int> &GameManager::getForcedMoves() const
{
    return this->forcedMoves;
}

/**
 * Update main window message
 */
inline void GameManager::updateMessage(const std::string &msg)
{
    this->currentMsg = msg;
}

/**
 * Get current message passed from Main
 * @return string value of mesage
 */
inline const std::string &GameManager::getCurrentMsg() const
{
    return this->currentMsg;
}

/**
 * /brief Get list of all checkboard cells
 * /return vector of unique_ptr of Cells
 */
inline const std::vector<chk::Block> &GameManager::getBlockList() const
{
    return this->blockList;
}

/**
 * Create checkerboard cells, labeled with an index using given font
 * @param font used for text labels
 */
inline void GameManager::drawCheckerboard(const sf::Font &font)
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
                float x = (col % NUM_COLS) * chk::SIZE_CELL;
                lightRec.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                auto whiteBlock = std::make_unique<chk::Cell>(-1, lightRec, lightRec.getPosition(), font);
                blockList.emplace_back(std::move_if_noexcept(whiteBlock));
            }
            else
            {
                // Odd cell, SET DARKER color (USED BY PIECES)
                sf::RectangleShape darkRect(sf::Vector2f(chk::SIZE_CELL, chk::SIZE_CELL));
                darkRect.setFillColor(sf::Color{82, 55, 27});
                float x = (col % NUM_COLS) * chk::SIZE_CELL;
                darkRect.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                auto redBlock = std::make_unique<chk::Cell>(counter, darkRect, darkRect.getPosition(), font);
                redBlock->setisEvenRow(row % 2 == 0);
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
inline void GameManager::drawAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    std::random_device randomDevice;
    std::mt19937 randEngine(randomDevice());
    std::uniform_int_distribution<uint16_t> dist(1, 54059);
    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                sf::CircleShape circle(0.5 * chk::SIZE_CELL);
                const float x = (col % NUM_COLS) * chk::SIZE_CELL;
                circle.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                if (row < 3)
                {
                    // Top cells, put BLACK piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(kete));
                }
                else if (row > 4)
                {
                    // Bottom cell, put RED piece
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
inline void GameManager::handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &destCell,
                                         const int &currentPieceId)
{
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
        //FIXME check for opportunities the whole board
        this->capturedPiece->preyPieceId = currentPieceId;
        this->capturedPiece->preyCellIdx = destCell->getIndex();
        std::cout << player->getName() << " is in DANGER!" << std::endl;
    }
}

/**
 * Handle capturing of "Prey's" pieces by "Hunter", then update gameMap
 * @param hunter the offensive player
 * @param prey the defensive player
 * @param targetCell the destination of hunter
 */
inline void GameManager::handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                         const chk::Block &targetCell)
{
    // Check if There's a PIECE in this cell. Cannot Jump to here.
    if (this->getPieceFromCell(targetCell->getIndex()) != -1)
        return;

    for (const auto &[piece_id, cell_idx] : this->forcedMoves)
    {
        if (cell_idx == targetCell->getIndex())
        {
            if (!hunter->captureEnemyWith(piece_id, targetCell->getPos()))
            {
                return;
            }
            gameMap.erase(this->sourceCell);                   // set hunter's old location empty!
            gameMap.emplace(targetCell->getIndex(), piece_id); // fill in hunter new location
            gameMap.erase(this->capturedPiece->preyCellIdx);   // set Prey's old location empty!
            prey->losePiece(this->capturedPiece->preyPieceId); // the defending player loses 1 piece
            this->playerRedTurn = !this->playerRedTurn;        // toggle player turns
            this->sourceCell = -1;
            this->capturedPiece->preyCellIdx = -1;
            this->capturedPiece->preyPieceId = -1;
            this->forcedMoves.clear();
            // TODO check for opportunities to capture ANOTHER NOW!
            const auto dangerRight = this->checkDangerRHS(hunter, targetCell);
            const auto dangerLeft = this->checkDangerLHS(hunter, targetCell);
            if (dangerRight || dangerLeft)
            {
                this->capturedPiece->preyCellIdx = targetCell->getIndex();
                this->capturedPiece->preyPieceId = gameMap.at(targetCell->getIndex());
                std::cout << hunter->getName() << " is in DANGER!" << std::endl;
            }
            break;
        }
    }
}

/**
 * Whether it's Red player's turn
 * @return TRUE or FALSE
 */
inline bool GameManager::isPlayerRedTurn() const
{
    return this->playerRedTurn;
}

/**
 * Store the cell idx from which the piece is LEAVING
 * @param src_cell index of the cell
 */
inline void GameManager::setSourceCell(const int &src_cell)
{
    this->sourceCell = src_cell;
}

/**
 * /brief Whether both src_cell is NOT NULL & forcedMoves NOT empty
 * /return TRUE or FALSE
 */
inline bool GameManager::isReadyForCapture() const
{
    return this->sourceCell != -1 && !forcedMoves.empty();
}
/**
 * check if player who has just moved is in danger (North west)
 * @param player unique ptr of player
 * @param destCell the just moved-in cell
 */
inline bool GameManager::checkDangerLHS(const chk::PlayerPtr &player, const Block &destCell)
{
    bool hasEmptyCellBehind = false;
    bool hasEnemyAhead = false;
    short deltaForward = destCell->getIsEvenRow() ? 4 : 5;
    short deltaBehind = destCell->getIsEvenRow() ? 5 : 4;
    int mSign = 1;

    if (destCell->getPos().x == 0 || destCell->getPos().x >= 7 * chk::SIZE_CELL)
    {
        // SAFE at either edges
        return false;
    }

    // if is player Black (PLAYER 2)
    if (player->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    int cellAheadIdx = destCell->getIndex() + (deltaForward * mSign);
    int pieceId_NW = this->getPieceFromCell(cellAheadIdx);
    hasEnemyAhead = pieceId_NW != -1 && !player->hasThisPiece(pieceId_NW);

    int cellBelowRight = destCell->getIndex() - (deltaBehind * mSign);
    int pieceId_SE = this->getPieceFromCell(cellBelowRight);
    hasEmptyCellBehind = pieceId_SE == -1;

    bool inDanger = false;
    if (hasEnemyAhead && hasEmptyCellBehind)
    {
        inDanger = true;
        this->forcedMoves.insert(std::make_pair(pieceId_NW, cellBelowRight));
    }
    return inDanger;
}

/**
 * check if player who has just moved is in danger (North East)
 * @param player unique ptr of player
 * @param destCell the just moved-in cell
 */
inline bool GameManager::checkDangerRHS(const chk::PlayerPtr &player, const Block &destCell)
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

    // if is player Black (PLAYER 2)
    if (player->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    int cellAheadIdx = destCell->getIndex() + (deltaForward * mSign);
    int pieceId_NE = this->getPieceFromCell(cellAheadIdx); // north East
    hasEnemyAhead = pieceId_NE != -1 && !player->hasThisPiece(pieceId_NE);

    int cellBelowLeft = destCell->getIndex() - (deltaBehind * mSign);
    int pieceId_SW = this->getPieceFromCell(cellBelowLeft); // south West
    hasEmptyCellBehind = pieceId_SW == -1;

    bool inDanger = false;
    if (hasEnemyAhead && hasEmptyCellBehind)
    {
        inDanger = true;
        this->forcedMoves.insert(std::make_pair(pieceId_NE, cellBelowLeft));
    }
    return inDanger;
}

/**
 * Using cached hashmap, get the PieceId placed at this cell_index
 *
 * @param cell_idx the clicked cell
 * @return positive int  or -1 if not found
 */
inline int GameManager::getPieceFromCell(const int &cell_idx)
{
    if (this->gameMap.find(cell_idx) != gameMap.end())
    {
        return gameMap.at(cell_idx);
    }
    return -1;
}

/**
 * Match cells to pieces, using location, at the beginning of the game, and cache it to Hashmap
 * @param pieceList vector of all pieces
 */
inline void GameManager::matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList)
{
    if (this->alreadyCached)
    {
        return;
    }
    std::ofstream fout("D:/work_mine/CPP/space-checkers/game_map.txt");
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
    fout << std::endl;
    fout.close();
    this->alreadyCached = true;
    std::cout << "hashmap size " << gameMap.size() << std::endl;
}

} // namespace chk
