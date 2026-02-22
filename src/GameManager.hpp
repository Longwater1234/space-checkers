//
// Created by Davis on 2023-10-29.
//
#pragma once

#include "CaptureTarget.hpp"
#include "Cell.hpp"
#include "CircularBuffer.hpp"
#include "Player.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace chk
{
// cell ptr
using Block = std::unique_ptr<chk::Cell>;
// player ptr
using PlayerPtr = std::unique_ptr<chk::Player>;

constexpr uint16_t NUM_ROWS{8};
constexpr uint16_t NUM_COLS{8};

/**
 * Abstract game manager (Base Class)
 */
class GameManager
{

  public:
    virtual ~GameManager() = default;
    virtual void createAllPieces() = 0;
    virtual void handleEvents(chk::CircularBuffer<short> &buffer) = 0;
    virtual void drawBoard() = 0;
    void drawCheckerboard(const sf::Font &font);
    void updateMessage(std::string_view msg);
    void matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList);
    [[nodiscard]] const std::unordered_map<short, chk::CaptureTarget> &getForcedMoves() const;
    [[nodiscard]] const std::string &getCurrentMsg() const;

  private:
    // flag to check if cache is already filled
    bool alreadyCached = false;
    // whether it's player Red's turn
    bool playerRedTurn = true;
    // bottom display message
    mutable std::string currentMsg;
    // whether match is over (for offline play only)
    bool gameOver = false;
    // used for atomic updates
    std::mutex my_mutex;

    [[nodiscard]] bool boardContainsCell(const int cell_idx) const;
    [[nodiscard]] bool awayFromEdge(const int cell_idx) const;
    void collectFrontRHS(const chk::PlayerPtr &hunter, const chk::Block &cell_ptr);
    void collectFrontLHS(const chk::PlayerPtr &hunter, const chk::Block &cell_ptr);
    void collectBehindRHS(const chk::PlayerPtr &hunter, const chk::Block &cell_ptr);
    void collectBehindLHS(const chk::PlayerPtr &hunter, const chk::Block &cell_ptr);

  protected:
    explicit GameManager(sf::RenderWindow *windowPtr);
    // gameBoard: map of cell_index -> piece_id
    std::unordered_map<int, short> gameMap;
    // main window
    sf::RenderWindow *window = nullptr;
    // source cell Index of selected piece
    std::optional<int> sourceCell{};
    // all checkerboard cells
    std::vector<chk::Block> blockList{};
    // first player (p1)
    chk::PlayerPtr playerRed = nullptr;
    // second player (p2)
    chk::PlayerPtr playerBlack = nullptr;
    // collection of Player's next targets (Map<HunterPieceID, CaptureTarget>)
    std::unordered_map<short, chk::CaptureTarget> forcedMoves{};

    [[nodiscard]] bool isPlayerRedTurn() const;
    [[nodiscard]] short getPieceFromCell(const int cell_idx) const;
    [[nodiscard]] const std::vector<chk::Block> &getBlockList() const;
    [[nodiscard]] bool isHunterActive() const;
    [[nodiscard]] bool isGameOver() const;
    void setSourceCell(const int src_cell);
    void doCleanup();
    void identifyTargets(const chk::PlayerPtr &hunter, const chk::Block &singleCell = nullptr);
    virtual void handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                                 const short currentPieceId);
    virtual void handleCapturePiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                                    const chk::Block &targetCell);
    virtual void handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                               chk::CircularBuffer<short> &buffer, const chk::Block &cell);
    void updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2);
    void showForcedMoves(const chk::PlayerPtr &player, const chk::Block &cell);
};

} // namespace chk
