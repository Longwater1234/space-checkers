#pragma once
#include "../GameManager.hpp"

namespace chk
{
/**
 * This class is responsible for offline play
 * @since 2024-11-04
 */
class LocalGameManager : public chk::GameManager
{
  public:
    explicit LocalGameManager(sf::RenderWindow *windowPtr);
    LocalGameManager() = delete;
    void createAllPieces(std::vector<chk::PiecePtr> &pieceList) override;

    // Inherited via GameManager
    void handleEvents() override;
    void drawScreen() override;
    void setOnReadyCreatePiecesCallback(const onReadyCreatePieces &callback) override;
};

inline void LocalGameManager::createAllPieces(std::vector<chk::PiecePtr> &pieceList)
{
    std::random_device randomDevice;
    std::mt19937 randEngine(randomDevice());
    std::uniform_int_distribution<short> dist(1, std::numeric_limits<short>::max());
    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                sf::CircleShape circle(0.5 * chk::SIZE_CELL);
                const float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                circle.setPosition(sf::Vector2f(x, row * chk::SIZE_CELL));
                if (row < 3)
                {
                    // Half Top cells, put BLACK piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(kete));
                }
                else if (row > 4)
                {
                    // Half Bottom cells, put RED piece
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(kete));
                }
            }
        }
    }
    this->_onReadyCreatePieces(true);
}

inline LocalGameManager::LocalGameManager(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->sourceCell = -1;
    this->forcedMoves.clear();
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
}

void LocalGameManager::handleEvents()
{
}

void LocalGameManager::drawScreen()
{
}

void LocalGameManager::setOnReadyCreatePiecesCallback(const onReadyCreatePieces &callback)
{
    this->_onReadyCreatePieces = callback;
}
} // namespace chk
