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
    void drawScreen(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2) override;
    void handleEvents(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2, chk::CircularBuffer<short> &buffer) override;
    void setOnReadyPiecesCallback(const onReadyCreatePieces &callback) override;
};

/**
 * Custom constructor
 * @param windowPtr original window from main.cpp
 */
inline LocalGameManager::LocalGameManager(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->sourceCell = std::nullopt;
    this->blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
}

/**
 * Create all pieces for both players and add them to pieceList, using stdlib random generator
 * @param pieceList destination of created pieces
 */
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

/**
 * This will be called in the main game loop, every 60 FPS, drawing elements on screen
 * @param p1 Red Player
 * @param p2 Black Player
 */
void LocalGameManager::drawScreen(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2)
{
    auto mousePos = sf::Mouse::getPosition(*window);
    // DRAW CHECKERBOARD
    for (const auto &cell : this->getBlockList())
    {
        window->draw(*cell);
    }
    // DRAW RED PIECES
    for (const auto &[id, red_piece] : p1->getOwnPieces())
    {
        if (this->isPlayerRedTurn() && red_piece->containsPoint(mousePos))
        {
            red_piece->addOutline();
        }
        else
        {
            red_piece->removeOutline();
        }
        window->draw(*red_piece);
    }
    // DRAW BLACK PIECES
    for (const auto &[id, black_piece] : p2->getOwnPieces())
    {
        if (!this->isPlayerRedTurn() && black_piece->containsPoint(mousePos))
        {
            black_piece->addOutline();
        }
        else
        {
            black_piece->removeOutline();
        }
        window->draw(*black_piece);
    }
}

/**
 * This will be called in the main game loop, every 60 FPS, drawing elements on screen
 * @param p1 Red Player
 * @param p2 Black Player
 * @param circularBuffer stores the currently selected piece
 */
void LocalGameManager::handleEvents(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2,
                                    chk::CircularBuffer<short> &circularBuffer)
{
    for (auto event = sf::Event{}; window->pollEvent(event);)
    {
        if (event.type == sf::Event::Closed)
        {
            window->close();
        }
        if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            const auto clickedPos = sf::Mouse::getPosition(*window);
            /* Check window bounds */
            if (clickedPos.y > chk::SIZE_CELL * 8)
            {
                continue;
            }
            for (auto &cell : this->getBlockList())
            {
                // inner loop
                if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
                {
                    const auto &hunter = this->isPlayerRedTurn() ? p1 : p2;
                    const auto &prey = this->isPlayerRedTurn() ? p2 : p1;

                    if (this->hasPendingCaptures())
                    {
                        this->handleJumpPiece(hunter, prey, cell);
                        this->updateMatchStatus(hunter, prey);
                        circularBuffer.clean();
                    }
                    else
                    {
                        this->handleCellTap(hunter, prey, circularBuffer, cell);
                    }
                    break;
                    // END inner loop
                }
            }
        }
    }
}

/**
 * Set the callback for handling event after creating all pieces
 */
void LocalGameManager::setOnReadyPiecesCallback(const onReadyCreatePieces &callback)
{
    this->_onReadyCreatePieces = callback;
}

} // namespace chk
