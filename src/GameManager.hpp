//
// Created by Davis on 10/29/2023.
//
#pragma once

#include "Cell.hpp"
#include "Piece.hpp"
#include "Player.hpp"
#include <iostream>
#include <memory>
#include <random>
namespace chk
{
using Block = std::unique_ptr<chk::Cell>;
constexpr size_t NUM_ROWS = 8;
constexpr size_t NUM_COLS = 8;

using Block = std::unique_ptr<chk::Cell>;

/**
 * Overall game state
 */
class GameManager
{

  public:
    GameManager();
    static void drawCheckerboard(std::vector<Block> &blockList, const sf::Font &font);
    static void drawAllPieces(std::vector<chk::PiecePtr> &pieceList);
    void matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList, const std::vector<Block> &cellList);

  private:
    // source cell Index of selected piece
    int sourceCell;
    // map of cell_index --> piece_id
    std::map<int, int> gameMap;
    // flag to check if cache is filled
    bool alreadyCached;
    // whether it's player Red's turn
    bool playerRedTurn = true;

  private:
    bool checkNorthWest(const std::unique_ptr<chk::Player> &player, const Block &destCell);
    bool checkNorthEast(const std::unique_ptr<chk::Player> &player, const Block &destCell);

  public:
    [[nodiscard]] bool isPlayerRedTurn() const;
    [[nodiscard]] inline int getPieceFromCell(const int &cell_idx);
    void setSourceCell(const int &src_cell);
    // bool checkDangerRight(const std::unique_ptr<chk::Player> &player, const Block &destCell);
    void handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &destCell, const int &currentPieceId);
};

inline GameManager::GameManager()
{
    this->sourceCell = -1;
    this->alreadyCached = false;
}

/**
 * Create red and black checkerboard cells, with Position
 * @param blockList empty list of cells
 * @param font      font loaded from file
 */
inline void GameManager::drawCheckerboard(std::vector<Block> &blockList, const sf::Font &font)
{
    int counter = 32;
    for (size_t row = 0; row < NUM_ROWS; row++)
    {
        for (size_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 == 0)
            {
                // even CELL, set LIGHTER color (UNUSED)
                sf::RectangleShape lightRec(sf::Vector2f(chk::SIZE_CELL, chk::SIZE_CELL));
                lightRec.setFillColor(sf::Color{255, 225, 151});
                float x = (col % NUM_COLS) * chk::SIZE_CELL;
                lightRec.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                auto whiteBlock = std::make_unique<chk::Cell>(lightRec, lightRec.getPosition(), -1);
                whiteBlock->setFont(font);
                blockList.emplace_back(std::move_if_noexcept(whiteBlock));
            }
            else
            {
                // Odd cell, SET DARKER color (USED BY PIECES)
                sf::RectangleShape darkRect(sf::Vector2f(chk::SIZE_CELL, chk::SIZE_CELL));
                darkRect.setFillColor(sf::Color{82, 55, 27});
                float x = (col % NUM_COLS) * chk::SIZE_CELL;
                darkRect.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                auto redBlock = std::make_unique<chk::Cell>(darkRect, darkRect.getPosition(), counter);
                redBlock->setFont(font);
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
    std::uniform_int_distribution<uint16_t> dist(1, 269);
    for (size_t row = 0; row < NUM_ROWS; row++)
    {
        for (size_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                sf::CircleShape circle(37.5f);
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
void GameManager::handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &destCell,
                                  const int &currentPieceId)
{
    bool success = player->movePiece(currentPieceId, destCell->getPos());
    // VERIFY IF move IS SUCCESSFUL
    if (!success)
    {
        return;
    }
    gameMap.erase(this->sourceCell);                // set old location empty!
    gameMap[destCell->getIndex()] = currentPieceId; // fill in the new location
    this->playerRedTurn = !this->playerRedTurn;
    if (this->checkNorthEast(player, destCell) || this->checkNorthWest(player, destCell))
    {
        std::cout << player->getName() << " is in danger" << std::endl;
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
 * check if player who has just moved is in danger (North west)
 * @param player unique ptr of player
 * @param destCell the just moved-in cell
 */
inline bool GameManager::checkNorthWest(const std::unique_ptr<chk::Player> &player, const Block &destCell)
{
    bool hasEmptyCellBehind = false;
    bool hasEnemyAhead = false;
    int deltaForward = destCell->getIsEvenRow() ? 4 : 5;
    int deltaBehind = destCell->getIsEvenRow() ? 5 : 4;
    int mSign = 1;

    if (destCell->getPos().x == 0 || destCell->getPos().x == 8 * chk::SIZE_CELL)
    {
        return false;
    }

    // if is player Black (PLAYER 2)
    if (player->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    /* red player */
    int cellAheadIdx = destCell->getIndex() + (deltaForward * mSign);
    int pieceId_NW = this->getPieceFromCell(cellAheadIdx);
    hasEnemyAhead = (pieceId_NW == -1) ? false : !player->hasThisPiece(pieceId_NW);

    int cellBelowRight = destCell->getIndex() - (deltaBehind * mSign);
    int pieceId_SE = this->getPieceFromCell(cellBelowRight);
    hasEmptyCellBehind = pieceId_SE == -1;

    if (hasEnemyAhead && hasEmptyCellBehind)
    {
        return true;
    }
    return false;
}

/**
 * check if player who has just moved is in danger (North East)
 * @param player unique ptr of player
 * @param destCell the just moved-in cell
 */
bool GameManager::checkNorthEast(const std::unique_ptr<chk::Player> &player, const Block &destCell)
{
    bool hasEmptyCellBehind = false;
    bool hasEnemyAhead = false;
    int deltaForward = destCell->getIsEvenRow() ? 3 : 4;
    int deltaBehind = destCell->getIsEvenRow() ? 4 : 3;
    int mSign = 1;

    if (destCell->getPos().x == 0 || destCell->getPos().x == 8 * chk::SIZE_CELL)
    {
        return false;
    }

    // if is player Black (PLAYER 2)
    if (player->getPlayerType() == PlayerType::PLAYER_2)
    {
        mSign = -1;
        std::swap(deltaForward, deltaBehind);
    }

    /* red player */
    int cellAheadIdx = destCell->getIndex() + (deltaForward * mSign);
    int pieceId_NE = this->getPieceFromCell(cellAheadIdx);
    hasEnemyAhead = (pieceId_NE == -1) ? false : !player->hasThisPiece(pieceId_NE);

    int cellBelowRight = destCell->getIndex() - (deltaBehind * mSign);
    int pieceId_SW = this->getPieceFromCell(cellBelowRight);
    hasEmptyCellBehind = pieceId_SW == -1;

    if (hasEnemyAhead && hasEmptyCellBehind)
    {
        return true;
    }
    return false;
}

/**
 * Using hashMap, get the PieceId placed at this cell_index
 *
 * @param cell_idx the clicked cell
 * @return positive int  or -1 if not found
 */
inline int GameManager::getPieceFromCell(const int &cell_idx)
{
    if (this->gameMap.find(cell_idx) != gameMap.end())
    {
        return gameMap[cell_idx];
    };
    return -1;
}

/**
 * Match the 2 lists, by position, at the beginning of the game, and cache it to Hashmap
 * @param pieceList vector of all pieces
 * @param cellList vector of all cells
 */
void GameManager::matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList, const std::vector<Block> &cells)
{
    if (this->alreadyCached)
    {
        return;
    }

    for (const auto &piece : pieceList)
    {
        for (const auto &cell : cells)
        {
            if (cell->getIndex() != -1 && cell->containsOrigin(piece->getPosition()))
            {
                this->gameMap[cell->getIndex()] = piece->getId();
            }
        }
    }
    this->alreadyCached = true;
    std::cout << "hashmap size " << gameMap.size() << std::endl;
}

} // namespace chk
