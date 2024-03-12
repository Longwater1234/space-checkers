//
// Created by Davis on 10/29/2023.
//
#pragma once

#include "CaptureTarget.hpp"
#include "Cell.hpp"
#include "Player.hpp"
#include "spdlog/spdlog.h"
#include <SFML/Graphics/Text.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace chk
{
using Block = std::unique_ptr<chk::Cell>;
using PlayerPtr = std::unique_ptr<chk::Player>;
using onMoveSuccessCallback = std::function<void(short, int)>; // callback after moving
using onJumpSuccess = std::function<void(short, int)>;         // callback after capturing
constexpr uint16_t NUM_ROWS = 8;
constexpr uint16_t NUM_COLS = 8;

/**
 * Overall game manager
 */
class GameManager
{

  public:
    GameManager();
    void drawCheckerboard(const sf::Font &font);
    static void createAllPieces(std::vector<chk::PiecePtr> &pieceList);
    void updateMessage(std::string_view msg);
    void matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList);
    [[nodiscard]] const std::unordered_map<short, chk::CaptureTarget> &getForcedMoves() const;
    [[nodiscard]] const std::string &getCurrentMsg() const;

  private:
    // source cell Index of selected piece
    int sourceCell;
    // all checkerboard cells
    std::vector<chk::Block> blockList;
    // map of cell_index --> piece_id
    std::map<int, short> gameMap;
    // flag to check if cache is already filled
    bool alreadyCached = false;
    // whether it's player Red's turn
    bool playerRedTurn = true;
    // bottom display message
    std::string currentMsg;
    // whether match is over
    bool gameOver = false;
    // collection of my next targets (Map<HunterPieceID, CaptureTarget>)
    std::unordered_map<short, chk::CaptureTarget> forcedMoves;
    // mutex for atomic updates
    std::mutex my_mutex;
    // callback after successfully moved piece
    onMoveSuccessCallback onMoveSuccess_;

  private:
    [[nodiscard]] bool boardContainsCell(const int &cell_idx) const;
    [[nodiscard]] bool awayFromEdge(const int &cell_idx) const;
    void identifyTargets(const chk::PlayerPtr &hunter);
    void collectFrontRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectFrontLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectBehindRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectBehindLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);

  public:
    [[nodiscard]] const bool &isPlayerRedTurn() const;
    [[nodiscard]] short getPieceFromCell(const int &cell_idx);
    [[nodiscard]] const std::vector<chk::Block> &getBlockList() const;
    [[nodiscard]] bool hasPendingCaptures() const;
    [[nodiscard]] const bool &isGameOver() const;
    void setSourceCell(const int &src_cell);
    void handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                         const short &currentPieceId);
    void handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, const chk::Block &targetCell);
    void updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2);
    void setOnMoveSuccessCallback(const onMoveSuccessCallback &callback);
};

} // namespace chk
