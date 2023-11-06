
#include "CircularBuffer.hpp"
#include "GameState.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <memory>
#include <vector>

constexpr uint16_t NUM_PIECES = 24;
constexpr uint16_t NUM_CELLS = 64;
constexpr auto ICON_PATH = "resources/icons8-checkers-16.png";
constexpr auto FONT_PATH = "resources/open-sans.regular.ttf";

/**
 * When player taps a cell
 * @param gameState game state
 * @param player currentPlayer
 * @param buffer Temporary store for clicked Pieces
 * @param cell Tapped cell
 */
inline void handleCellTap(std::shared_ptr<chk::GameState> &gameState, const std::unique_ptr<chk::Player> &player,
                          chk::CircularBuffer<int> &buffer, const std::unique_ptr<chk::Cell> &cell)
{
    // CHECK IF cell has Piece
    int pieceId = gameState->getCachedPieceId(cell->getIndex());
    if (pieceId != -1)
    {
        // YES HAS Piece, keep it in buffer!
        std::cout << "piece id " << pieceId << std::endl;
        gameState->setSourceCell(cell->getIndex());
        buffer.addItem(pieceId);
    }
    else
    {
        // It's Empty! Let's move a piece (from buffer) here!
        //  First, verify if Buffer has data
        if (!buffer.isEmpty())
        {
            const int movablePieceId = buffer.getTop();
            gameState->handleMovePiece(player, cell, movablePieceId);
        }
        // dont forget to clean up!
        buffer.clean();
    }
}

int main()
{
    auto window = sf::RenderWindow{sf::VideoMode(800u, 900u), "Checkers CPP", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(30u);

    sf::Image appIcon;
    if (appIcon.loadFromFile(ICON_PATH))
    {
        auto dims = appIcon.getSize();
        window.setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }

    // CREATE CHECKERBOARD
    std::vector<chk::Block> blockList;
    blockList.reserve(NUM_CELLS);
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH))
    {
        perror("cannot find file");
        exit(EXIT_FAILURE);
    }
    auto gameState = std::make_shared<chk::GameState>();
    gameState->drawCheckerboard(blockList, font);

    // CREATE YOUR TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);

    // NOW DRAW all PIECES ON BOARD
    std::vector<chk::PiecePtr> keteList;
    keteList.reserve(NUM_PIECES);
    gameState->drawAllPieces(keteList);

    gameState->matchCellsToPieces(keteList, blockList);

    // Give each player their own pieces
    for (auto &kete : keteList)
    {
        if (kete->getPieceType() == chk::PieceType::Red)
        {
            p1->givePiece(kete);
        }
        else
        {
            p2->givePiece(kete);
        }
    }

    // we don't need this anymore
    keteList.clear();

    // Our temp store with maxCap of 1
    chk::CircularBuffer<int> circularBuffer(1);

    // THE STATUS TEXT
    sf::Text txtPanel;
    txtPanel.setFont(font);
    std::string statusText;
    txtPanel.setString("Now playing!");
    txtPanel.setCharacterSize(16u);
    txtPanel.setFillColor(sf::Color::White);
    txtPanel.setPosition(sf::Vector2f(0, 825));

    while (window.isOpen())
    {

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                const auto clickedPos = sf::Mouse::getPosition(window);
                /* Check window bounds */
                if (clickedPos.y <= 800u)
                {
                    for (auto &cell : blockList)
                    {
                        if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
                        {
                            statusText = "Tapped cell index " + std::to_string(cell->getIndex());
                            handleCellTap(gameState, p1, circularBuffer, cell);
                            break;
                        }
                    }
                }
            }
        }

        auto mousePos = sf::Mouse::getPosition(window);
        window.clear();

        for (auto &cell : blockList)
        {
            window.draw(*cell);
        }

        for (const auto &red_piece : p1->getOwnPieces())
        {
            if (red_piece->containsPoint(mousePos))
            {
                red_piece->addOutline();
            }
            else
            {
                red_piece->removeOutline();
            }
            window.draw(*red_piece);
        }
        for (const auto &black_piece : p2->getOwnPieces())
        {
            window.draw(*black_piece);
        }

        txtPanel.setString(statusText);
        window.draw(txtPanel);
        window.display();
    }
    return EXIT_SUCCESS;
}
