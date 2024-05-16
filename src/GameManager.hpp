//
// Created by Davis on 10/29/2023.
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
using Block = std::unique_ptr<chk::Cell>;
using PlayerPtr = std::unique_ptr<chk::Player>;
using onMoveSuccessCallback = std::function<void(short, int)>; // callback after moving
using onJumpSuccess = std::function<void(short, int)>;         // callback after capturing
using onReadyCreatePieces = std::function<void(bool)>;         // callback after creating pieces
constexpr uint16_t NUM_ROWS{8};
constexpr uint16_t NUM_COLS{8};

/**
 * Abstract game manager (Base Class)
 */
class GameManager
{

  public:
    GameManager() = default;
    virtual ~GameManager() = default;
    virtual void createAllPieces(std::vector<chk::PiecePtr> &pieceList) = 0;
    virtual void handleEvents(chk::CircularBuffer<short> &buffer) = 0;
    virtual void drawScreen() = 0;
    virtual void setOnReadyPiecesCallback(const onReadyCreatePieces &callback) = 0;
    void drawCheckerboard(const sf::Font &font);
    void updateMessage(std::string_view msg);
    void matchCellsToPieces(const std::vector<chk::PiecePtr> &pieceList);
    void setOnMoveSuccessCallback(const onMoveSuccessCallback &callback);
    [[nodiscard]] const std::unordered_map<short, chk::CaptureTarget> &getForcedMoves() const;
    [[nodiscard]] const std::string &getCurrentMsg() const;

  private:
    // map of cell_index -> piece_id
    std::map<int, short> gameMap;
    // flag to check if cache is already filled
    bool alreadyCached = false;
    // whether it's player Red's turn
    bool playerRedTurn = true;
    // bottom display message
    std::string currentMsg;
    // whether match is over
    bool gameOver = false;
    // mutex for atomic updates
    std::mutex my_mutex;
    // collection of my next targets (Map<HunterPieceID, CaptureTarget>)
    std::unordered_map<short, chk::CaptureTarget> forcedMoves{};
    // callback after successfully moved piece
    onMoveSuccessCallback _onMoveSuccess;

    [[nodiscard]] bool boardContainsCell(const int &cell_idx) const;
    [[nodiscard]] bool awayFromEdge(const int &cell_idx) const;
    void identifyTargets(const chk::PlayerPtr &hunter);
    void collectFrontRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectFrontLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectBehindRHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);
    void collectBehindLHS(const chk::PlayerPtr &hunter, const Block &cell_ptr);

  protected:
    // callback after creating pieces for both players
    onReadyCreatePieces _onReadyCreatePieces;
    // main window
    sf::RenderWindow *window = nullptr;
    // source cell Index of selected piece
    std::optional<int> sourceCell;
    // all checkerboard cells
    std::vector<chk::Block> blockList{};
    // first player (RED)
    chk::PlayerPtr player1 = nullptr;
    // second player (BLACK)
    chk::PlayerPtr player2 = nullptr;

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
    void showForcedMoves(const chk::PlayerPtr &player, const chk::Block &cell);
    void handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, chk::CircularBuffer<short> &buffer,
                       const chk::Block &cell);
};

} // namespace chk
