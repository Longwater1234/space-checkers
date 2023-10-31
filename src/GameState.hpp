//
// Created by Davis on 10/29/2023.
//
#pragma once

#include "Cell.hpp"
#include "Piece.hpp"
#include "Player.hpp"
#include <memory>
#include <random>
#include <unordered_map>
namespace chk
{
using Block = std::unique_ptr<chk::Cell>;
using Kete = std::unique_ptr<chk::Piece>;

constexpr uint16_t NUM_ROWS = 8;
constexpr uint16_t NUM_COLS = 8;

using Block = std::unique_ptr<chk::Cell>;
using Kete = std::unique_ptr<chk::Piece>;

/**
 * Overall game state
 */
class GameState
{

  public:
    GameState();
    static void drawCheckerboard(std::vector<Block> &blockList, const sf::Font &font);
    static void drawAllPieces(std::vector<Kete> &pieceList);
    [[nodiscard]] bool checkCanMove() const;

  private:
    // next destination of selected piece
    int targetCell;
    // currently clicked piece
    uint16_t selectedPieceId;

  public:
    [[nodiscard]] uint16_t getTargetCell() const;
    void setTargetCell(const int &cell_idx);
    [[nodiscard]] uint16_t getSelectedPieceId() const;
    void setSelectedPieceId(const uint16_t &pieceId);
    void handleMovePiece(const std::unique_ptr<chk::Player> &player, const std::unique_ptr<chk::Cell> &cell);
};

inline GameState::GameState()
{
    this->selectedPieceId = 0;
    this->targetCell = -1;
}

/**
 * Create red and black checkerboard cells, with Position
 * @param blockList empty list of cells
 * @param font      for text inside cells
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
                // even CELL, set LIGHTER color
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
                // Odd cell, SET DARKER color (USED by Pieces)
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
inline void GameState::drawAllPieces(std::vector<Kete> &pieceList)
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
                // Put piece on Odd cells only
                sf::CircleShape circle(50.0f);
                const float x = (col % NUM_COLS) * 100.0f;
                circle.setPosition(sf::Vector2f(x, row * 100.0f));
                if (row < 3)
                {
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move(kete));
                }
                else if (row > 4)
                {
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, dist(randEngine));
                    pieceList.emplace_back(std::move(kete));
                }
            }
        }
    }
}

/**
 * Place the piece on clicked cell
 * @param player current player
 * @param cell target cell
 */
void GameState::handleMovePiece(const std::unique_ptr<chk::Player> &player, const std::unique_ptr<chk::Cell> &cell)
{
    this->setTargetCell(cell->getIndex());
    if (cell->getIndex() == 0)
    {
        this->setSelectedPieceId(0);
    }
    if (this->checkCanMove())
    {
        // TODO HANDLE MOVE HERE
        int pieceId = this->getSelectedPieceId();

        for (auto &pp : player->getOwnPieces())
        {
            if (pieceId == pp->getId())
            {
                pp->moveCustom(cell->getCellPos());
                this->setSelectedPieceId(0);
                break;
            }
        }
    }
}

/**
 * check if the move is valid
 * @return TRUE or false
 */
inline bool GameState::checkCanMove() const
{
    return this->selectedPieceId != 0 && this->targetCell != -1;
}

uint16_t GameState::getTargetCell() const
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

uint16_t GameState::getSelectedPieceId() const
{
    return selectedPieceId;
}

/**
 * store current clicked PieceId
 * @param pieceId the piece id
 */
inline void GameState::setSelectedPieceId(const uint16_t &pieceId)
{
    this->selectedPieceId = pieceId;
}
} // namespace chk
