#pragma once

#include "../GameManager.hpp"
#include <SFML/Window/Event.hpp>
#include <array>
#include <limits>
#include <numeric>
#include <spdlog/fmt/fmt.h>

namespace chk
{
/**
 * This class is responsible for offline play
 * @since 2024-04-11
 */
class LocalGameManager : public chk::GameManager
{
  public:
    explicit LocalGameManager(sf::RenderWindow *windowPtr);
    LocalGameManager() = delete;
    ~LocalGameManager() override = default;

    // Inherited via GameManager
    void createAllPieces() override;
    void drawBoard() override;
    void handleEvents(chk::CircularBuffer<int> &buffer) override;

  protected:
    std::array<int, chk::NUM_PIECES> generateRandomPieceIds();
    void handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent, const Block &destCell,
                         const int currentPieceId) override;
    void updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2) override;
};

/**
 * Custom constructor
 * @param windowPtr original window from main.cpp
 */
inline LocalGameManager::LocalGameManager(sf::RenderWindow *windowPtr) : GameManager(windowPtr)
{
    // nothing here — players already created by base
}

/**
 * Create all pieces for both players (using std C++ PRNG), then place them on the board.
 */
inline void LocalGameManager::createAllPieces()
{
    auto pieceIds = this->generateRandomPieceIds();
    int idx = 0;

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
                    auto pb = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, pieceIds.at(idx++));
                    pieceList.emplace_back(std::move_if_noexcept(pb));
                }
                else if (row > 4)
                {
                    // Half Bottom cells, put RED piece
                    auto ppr = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, pieceIds.at(idx++));
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
    static sf::Clock deltaClock;
    const float deltaTime = deltaClock.restart().asSeconds();

    // DRAW CHECKERBOARD
    for (const auto &cell : this->getBlockList())
    {
        window->draw(*cell);
    }
    // DRAW RED PIECES
    for (const auto &[id, red_piece] : this->playerRed->getOwnPieces())
    {
        red_piece->updateAnimation(deltaTime);
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
        black_piece->updateAnimation(deltaTime);
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
inline void LocalGameManager::handleEvents(chk::CircularBuffer<int> &buffer)
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

/**
 * Generates 24 unique random piece IDs from the range [1, int_NAX] for both players.
 *
 * @return array of 24 unique ints in random order.
 */
inline std::array<int, chk::NUM_PIECES> LocalGameManager::generateRandomPieceIds()
{
    static std::mt19937 gen([] {
        std::random_device rd;
        std::seed_seq seed{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
        return std::mt19937{seed};
    }());

    std::uniform_int_distribution<int> dist(1, std::numeric_limits<int>::max());

    std::unordered_set<int> uniqueIds;
    while (uniqueIds.size() < chk::NUM_PIECES)
    {
        uniqueIds.insert(dist(gen));
    }

    std::array<int, chk::NUM_PIECES> pieceIds{};
    std::copy(uniqueIds.begin(), uniqueIds.end(), pieceIds.begin());
    std::shuffle(pieceIds.begin(), pieceIds.end(), gen);
    return pieceIds;
}

/**
 * Move piece and check if opponent has any possible moves remaining.
 */
inline void LocalGameManager::handleMovePiece(const chk::PlayerPtr &player, const chk::PlayerPtr &opponent,
                                              const Block &destCell, const int currentPieceId)
{
    GameManager::handleMovePiece(player, opponent, destCell, currentPieceId);
    if (!this->sourceCell.has_value())
    {
        this->updateMatchStatus(player, opponent);
    }
}

/**
 * Checks piece count for both players and verifies if current player has possible moves.
 *
 * @param p1 first player
 * @param p2 second player
 */
inline void LocalGameManager::updateMatchStatus(const chk::PlayerPtr &p1, const chk::PlayerPtr &p2)
{
    GameManager::updateMatchStatus(p1, p2);
    if (this->isGameOver())
    {
        return;
    }

    const auto &currentTurnPlayer = this->isPlayerRedTurn() ? this->playerRed : this->playerBlack;
    const auto &otherPlayer = this->isPlayerRedTurn() ? this->playerBlack : this->playerRed;
    if (this->hasNoPossibleMoves(currentTurnPlayer))
    {
        this->setGameOver(true);
        this->updateMessage(fmt::format("GAME OVER! {} wins!", otherPlayer->getName()));
    }
}

} // namespace chk
