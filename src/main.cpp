#include "Cell.hpp"
#include "GameState.hpp"
#include "Piece.hpp"
#include "Player.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <memory>
#include <vector>

constexpr uint16_t NUM_PIECES = 24;
constexpr uint16_t NUM_CELLS = 64;
constexpr auto ICON_PATH = "resources/icons8-checkers-16.png";
constexpr auto FONT_PATH = "resources/open-sans.regular.ttf";

int main()
{
    auto window = sf::RenderWindow{sf::VideoMode(800u, 900u), "Checkers CPP", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60u);

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
    std::vector<chk::Kete> keteList;
    keteList.reserve(NUM_PIECES);
    gameState->drawAllPieces(keteList);

    // Give each player their own pieces
    for (auto &kete : keteList)
    {
        if (kete->getPieceType() == chk::PieceType::Red)
        {
            p1->givePiece(std::move(kete));
        }
        else
        {
            p2->givePiece(std::move(kete));
        }
    }

    // we don't need this anymore
    keteList.clear();

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
        }
        auto mousePos = sf::Mouse::getPosition(window);
        window.clear();

        for (auto &cell : blockList)
        {
            if (cell->containsPoint(mousePos))
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    gameState->setTargetCell(cell->getIndex());
                    if (cell->getIndex() == 0)
                    {
                        gameState->setSelectedPieceId(0);
                    }
                    if (gameState->checkCanMove())
                    {
                        // TODO HANDLE MOVE HERE
                        int pieceId = gameState->getSelectedPieceId();

                        for (auto &pp : p1->getOwnPieces())
                        {
                            if (pieceId == pp->getId())
                            {
                                pp->moveCustom(cell->getCellPos());
                                gameState->setSelectedPieceId(0);
                                break;
                            }
                        }
                    }
                }
            }
            window.draw(*cell);
        }

        for (const auto &red_piece : p1->getOwnPieces())
        {
            if (red_piece->containsPoint(mousePos))
            {
                red_piece->addOutline();
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    gameState->setSelectedPieceId(red_piece->getId());
                    statusText = "Clicked piece " + std::to_string(red_piece->getId());
                }
            }
            else
            {
                red_piece->removeOutline();
                // gameState->setSelectedPieceId(0);
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
