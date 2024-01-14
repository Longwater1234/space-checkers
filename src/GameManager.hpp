//
// Created by Davis on 10/29/2023.
//
#pragma once

#include "CaptureTarget.hpp"
#include "Cell.hpp"
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
    [[nodiscard]] const std::unordered_map<uint16_t, int> &getForcedMoves() const;
    [[nodiscard]] const std::string &getCurrentMsg() const;

  private:
    // source cell Index of selected piece
    int sourceCell;
    // details about opponent to be captured
    std::unique_ptr<CaptureTarget> cTarget;
    // checkerboard cells
    std::vector<chk::Block> blockList;
    // map of cell_index --> piece_id
    std::map<int, short> gameMap;
    // flag to check if cache is already filled
    bool alreadyCached = false;
    // whether it's player Red's turn
    bool playerRedTurn = true;
    // bottom display message
    std::string currentMsg;
    // Hashmap(hunterPieceId -> targetCell) For keeping records of pending "forced" captures
    std::unordered_map<uint16_t, int> forcedMoves;
    // whether match is over
    bool gameOver = false;

  private:
    [[nodiscard]] bool boardContainsCell(const int &cell_idx) const;
    bool checkDangerLHS(const chk::PlayerPtr &player, const Block &destCell);
    bool checkDangerRHS(const chk::PlayerPtr &player, const Block &destCell);
    void identifyTargets(const chk::PlayerPtr &hunter);
    void collectFrontRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectFrontLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectBehindRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectBehindLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);

  public:
    [[nodiscard]] const bool &isPlayerRedTurn() const;
    [[nodiscard]] short getPieceFromCell(const int &cell_idx);
    [[nodiscard]] const std::vector<chk::Block> &getBlockList() const;
    [[nodiscard]] bool isReadyForCapture() const;
    [[nodiscard]] const bool &isGameOver() const;
    void setSourceCell(const int &src_cell);
    void handleMovePiece(const std::unique_ptr<chk::Player> &player, const Block &destCell,
                         const short &currentPieceId);
    void handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, const chk::Block &targetCell);
    void updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2);
};

} // namespace chk
