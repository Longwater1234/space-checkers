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

constexpr uint16_t NUM_ROWS = 8;
constexpr uint16_t NUM_COLS = 8;

using Block = std::unique_ptr<chk::Cell>;

/**
 * Overall game state
 */
class GameState
{

  public:
    GameState();
    static void drawCheckerboard(std::vector<Block> &blockList, const sf::Font &font);
    static void drawAllPieces(std::vector<chk::PiecePtr> &pieceList);
    void matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList, const std::vector<Block> &cellList);
    [[nodiscard]] bool checkCanMove() const;

  private:
    // next destination of selected piece
    int targetCell;
    // currently clicked piece
    int currentPieceId;
    // previously selected piece
    int oldSelectedId;
    // map of cell_index --> piece_id
    std::map<int, int> gameMap;
    // flag to check if cache is filled
    bool alreadyCached;

  public:
    [[nodiscard]] int getTargetCell() const;
    void setTargetCell(const int &cell_idx);
    [[nodiscard]] inline int getCachedPieceId(const int &cell_idx);
    void setCurrentPieceId(const int &pieceId);
    void handleMovePiece(const std::unique_ptr<chk::Player> &player, const std::unique_ptr<chk::Cell> &cell);
};

inline GameState::GameState()
{
    this->currentPieceId = -1;
    this->targetCell = -1;
    this->oldSelectedId = 0;
    this->alreadyCached = false;
    //    this->gameMap = {{}};
}

/**
 * Create red and black checkerboard cells, with Position
 * @param blockList empty list of cells
 * @param font      for loaded from file cells
 */
inline void GameState::drawCheckerboard(std::vector<Block> &blockList, const sf::Font &font)
{
    int counter = 32;
    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 == 0)
            {
                // even CELL, set LIGHTER color (UNUSED)
                sf::RectangleShape lightRec(sf::Vector2f(100.f, 100.f));
                lightRec.setFillColor(sf::Color{255, 225, 151});
                float x = (col % NUM_COLS) * 100.0f;
                lightRec.setPosition(sf::Vector2f(x, row * 100.0f));
                auto whiteBlock = std::make_unique<chk::Cell>(lightRec, lightRec.getPosition(), -1);
                whiteBlock->setFont(font);
                blockList.emplace_back(std::move(whiteBlock));
            }
            else
            {
                // Odd cell, SET DARKER color (USED BY PIECES)
                sf::RectangleShape darkRect(sf::Vector2f(100.f, 100.f));
                darkRect.setFillColor(sf::Color{82, 55, 27});
                float x = (col % NUM_COLS) * 100.0f;
                darkRect.setPosition(sf::Vector2f(x, row * 100.0f));
                auto redBlock = std::make_unique<chk::Cell>(darkRect, darkRect.getPosition(), counter);
                redBlock->setFont(font);
                blockList.emplace_back(std::move(redBlock));
                counter--;
            }
        }
    }
}

/**
 * Create new checker pieces, each with own position, and add them to given vector
 * @param pieceList destination
 */
inline void GameState::drawAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    std::random_device randomDevice;
    std::mt19937 randEngine(randomDevice());
    std::uniform_int_distribution<int> dist(1, 269);
    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                sf::CircleShape circle(50.0f);
                const float x = (col % NUM_COLS) * 100.0f;
                circle.setPosition(sf::Vector2f(x, row * 100.0f));
                if (row < 3)
                {
                    // Top cells, put BLACK piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move(kete));
                }
                else if (row > 4)
                {
                    // Bottom cell, put RED piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, dist(randEngine));
                    pieceList.emplace_back(std::move(kete));
                }
            }
        }
    }
}

/**
 * Move the selected piece to clicked cell, and update the gameMap
 * @param player current player
 * @param cell target cell
 */
void GameState::handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &cell)
{

    if(gameMap.find(cell->getIndex()) != gameMap.end()) {
        // CELL HAS PIECE, SO NOW THIS IS CLICKED!!!! SAVE IT TO STATE.
        this->setCurrentPieceId(gameMap[cell->getIndex()]);
    }

    const int idx = player->getPieceVecIndex(currentPieceId);
    std::cout << "vector Piece index " << idx << std::endl;
    player->getOwnPieces()[idx]->moveCustom(cell->getCellPos());
    this->oldSelectedId = currentPieceId;
    // gameMap[cell->getIndex()] = currentPieceId;
    // this->currentPieceId = -1;
}

/**
 * check if the move is valid
 * @return TRUE or false
 */
inline bool GameState::checkCanMove() const
{
    return this->currentPieceId != -1;
}

int GameState::getTargetCell() const
{
    return targetCell;
}

/**
 * Store currently selected Cell
 * @param cell_idx cell index
 */
void GameState::setTargetCell(const int &cell_idx)
{
    GameState::targetCell = cell_idx;
}

/**
 * store currently clicked PieceId
 * @param pieceId the new piece id
 */
inline void GameState::setCurrentPieceId(const int &pieceId)
{
    this->currentPieceId = pieceId;
}

/***
 * Get from hashMap, the PieceId placed at this cell_index
 * @param cell_idx the clicked cell
 * @return pieceId or -1 if not found
 */
inline int GameState::getCachedPieceId(const int &cell_idx)
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
void GameState::matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList, const std::vector<Block> &cellList)
{
    if (this->alreadyCached)
    {
        return;
    }

    for (const auto &cell : cellList)
    {
        for (const auto &piece : pieceList)
        {
            if (cell->getIndex() != -1 && cell->containsOrigin(piece->getMyPos()))
            {
                this->gameMap[cell.get()->getIndex()] = piece.get()->getId();
            }
        }
    }
    this->alreadyCached = true;
    std::cout << "map size " << gameMap.size() << std::endl;
}

} // namespace chk