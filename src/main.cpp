
#include "CircularBuffer.hpp"
#include "GameManager.hpp"
#include "ResourcePath.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <memory>
#include <vector>

constexpr uint16_t NUM_PIECES = 24;
constexpr auto ICON_PATH = "win-icon-16.png";
constexpr auto FONT_PATH = "open-sans.regular.ttf";

/**
 * When player taps a cell
 * @param manager game manager
 * @param player currentPlayer
 * @param buffer Temporary store for clicked Pieces
 * @param cell Tapped cell
 */
void handleCellTap(std::shared_ptr<chk::GameManager> &manager, const std::unique_ptr<chk::Player> &player,
                   chk::CircularBuffer<int> &buffer, const std::unique_ptr<chk::Cell> &cell)
{
    // CHECK IF cell has Piece
    int pieceId = manager->getPieceFromCell(cell->getIndex());
    if (pieceId != -1)
    {
        // YES HAS Piece, keep source in buffer!
        manager->setSourceCell(cell->getIndex());
        buffer.addItem(pieceId);
    }
    else
    {
        // Cell is Empty! Let's move a piece (from buffer) here!
        //  First, verify if Buffer not empty
        if (!buffer.isEmpty())
        {
            const int movablePieceId = buffer.getTop();
            manager->handleMovePiece(player, cell, movablePieceId);
        }
    }
}

int main()
{
    auto window = sf::RenderWindow{sf::VideoMode(600u, 700u), "SpaceCheckers", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60u);

    sf::Image appIcon;
    if (appIcon.loadFromFile(getResourcePath(ICON_PATH)))
    {
        auto dims = appIcon.getSize();
        window.setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }

    // CREATE CHECKERBOARD
    std::vector<chk::Block> blockList;
    blockList.reserve(chk::NUM_COLS * chk::NUM_COLS);
    sf::Font font;
    if (!font.loadFromFile(getResourcePath(FONT_PATH)))
    {
        perror("cannot find file");
        exit(EXIT_FAILURE);
    }

    auto manager = std::make_shared<chk::GameManager>();
    manager->drawCheckerboard(blockList, font);

    // CREATE YOUR TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);

    // NOW DRAW all PIECES ON BOARD
    std::vector<chk::PiecePtr> keteList;
    keteList.reserve(NUM_PIECES);
    manager->drawAllPieces(keteList);
    manager->matchCellsToPieces(keteList, blockList);

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
    txtPanel.setPosition(sf::Vector2f(0, 650));

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
                if (clickedPos.y <= 600u)
                {
                    for (auto &cell : blockList)
                    {
                        if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
                        {
                            statusText = "Tapped cell index " + std::to_string(cell->getIndex());
                            if (manager->isPlayerRedTurn())
                            {
                                handleCellTap(manager, p1, circularBuffer, cell);
                            }
                            else
                            {
                                handleCellTap(manager, p2, circularBuffer, cell);
                            }
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

        for (const auto &red_pair : p1->getOwnPieces())
        {
            if (manager->isPlayerRedTurn() && red_pair.second->containsPoint(mousePos))
            {
                red_pair.second->addOutline();
            }
            else
            {
                red_pair.second->removeOutline();
            }
            window.draw(*red_pair.second);
        }
        for (const auto &black_pair : p2->getOwnPieces())
        {
            if (!manager->isPlayerRedTurn() && black_pair.second->containsPoint(mousePos))
            {
                black_pair.second->addOutline();
            }
            else
            {
                black_pair.second->removeOutline();
            }
            window.draw(*black_pair.second);
        }

        txtPanel.setString(statusText);
        window.draw(txtPanel);
        window.display();
    }
    return EXIT_SUCCESS;
}
