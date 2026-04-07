#pragma once

#include "../GameManager.hpp"
#include "../WsClient.hpp"
#include "../payloads/base_payload.pb.hpp"
#include "imgui-SFML.h"

namespace chk
{
using chk::payload::TeamColor;

/**
 * This class is responsible for online gameplay
 * @since 2024-04-11
 */
class OnlineGameManager final : public chk::GameManager
{
  public:
    explicit OnlineGameManager(sf::RenderWindow *windowPtr);
    OnlineGameManager() = delete;

    // Inherited via GameManager
    void createAllPieces() override;
    void handleEvents(chk::CircularBuffer<short> &circularBuffer) override;
    void drawBoard() override;

  protected:
    // Inherited via GameManager
    void handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                         const short currentPieceId) override;
    void handleCapturePiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey,
                            const chk::Block &targetCell) override;
    void handleCellTap(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, chk::CircularBuffer<short> &buffer,
                       const chk::Block &cell) override;

  private:
    mutable chk::PlayerType myTeam{};
    std::unique_ptr<chk::WsClient> wsClient = nullptr;
    std::atomic_bool isMyTurn = false;
    std::atomic_bool gameReady = false;
    void startMoveListener();
    void startCaptureListener();
    void startDeathListener();
};
} // namespace chk