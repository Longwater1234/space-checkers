#pragma once
#include "../GameManager.hpp"

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

    // CREATE TWO unique PLAYERS
    this->playerRed = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_RED);
    this->playerBlack = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_BLACK);
    assert(!(*playerRed == *playerBlack));
}

/**
 * Create all pieces for both players (using std C++ PRNG), then place them on the board.
 */
inline void LocalGameManager::createAllPieces()
{
    std::random_device randomDevice;
    std::mt19937 randEngine{randomDevice()};
    std::uniform_int_distribution<short> dist(1, std::numeric_limits<short>::max());

    // Reserve container for pieces on board
    std::vector<chk::PiecePtr> pieceList;
    pieceList.reserve(chk::NUM_PIECES);

    for (uint16_t row = 0; row < NUM_ROWS; row++)
    {
        for (uint16_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                sf::CircleShape circle{0.5 * chk::SIZE_CELL};
                const float x = static_cast<float>(col % NUM_COLS) * chk::SIZE_CELL;
                circle.setPosition(sf::Vector2f{x, row * chk::SIZE_CELL});
                if (row < 3)
                {
                    // Half Top cells, put BLACK piece
                    auto pb = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(pb));
                }
                else if (row > 4)
                {
                    // Half Bottom cells, put RED piece
                    auto ppr = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, dist(randEngine));
                    pieceList.emplace_back(std::move_if_noexcept(ppr));
                }
            }
        }
    }
    GameManager::matchCellsToPieces(pieceList);
    // GIVE EACH PLAYER their own piece
    for (auto &pp : pieceList)
    {
        if (pp->getPieceType() == chk::PieceType::Red)
        {
            this->playerRed->receivePiece(pp);
        }
        else
        {
            this->playerBlack->receivePiece(pp);
        }
    }
    // SAFE. It's now useless.
    pieceList.clear();
}

/**
 * This will be called in the main game loop, at 60 FPS, drawing elements on screen
 */
inline void LocalGameManager::drawBoard()
{
    auto mousePos = sf::Mouse::getPosition(*window);
    // DRAW CHECKERBOARD
    for (const auto &cell : this->getBlockList())
    {
        window->draw(*cell);
    }
    // DRAW RED PIECES
    for (const auto &[id, red_piece] : this->playerRed->getOwnPieces())
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
    for (const auto &[id, black_piece] : this->playerBlack->getOwnPieces())
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
 * This will be handling all UI and mouse events
 * @param buffer stores the currently selected piece
 */
inline void LocalGameManager::handleEvents(chk::CircularBuffer<short> &buffer)
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
            // START inner loop:
            for (auto &cell : this->getBlockList())
            {
                if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
                {
                    const auto &hunter = this->isPlayerRedTurn() ? this->playerRed : this->playerBlack;
                    const auto &prey = this->isPlayerRedTurn() ? this->playerBlack : this->playerRed;
                    GameManager::handleCellTap(hunter, prey, buffer, cell);
                    break;
                }
            }
            //^ END inner loop
        }
    }
}

} // namespace chk
