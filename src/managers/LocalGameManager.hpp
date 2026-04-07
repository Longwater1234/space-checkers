#pragma once

#include "../GameManager.hpp"
#include <array>
#include <limits>
#include <numeric>

namespace chk
{
/**
 * This class is responsible for offline play
 * @since 2024-04-11
 */
class LocalGameManager final : public chk::GameManager
{
  public:
    explicit LocalGameManager(sf::RenderWindow *windowPtr);
    LocalGameManager() = delete;

    // Inherited via GameManager
    void createAllPieces() override;
    void drawBoard() override;
    void handleEvents(chk::CircularBuffer<short> &buffer) override;

  private:
    std::array<short, chk::NUM_PIECES> generateRandomPieceIds();
};
} // namespace chk