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

using Block = std::unique_ptr<chk::Cell>;
using Kete = std::unique_ptr<chk::Piece>;


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
    std::vector<Block> blockList;
    blockList.reserve(NUM_CELLS);
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH))
    {
        perror("cannot find file");
        exit(EXIT_FAILURE);
    }
    chk::GameState gameState;
    gameState.drawCheckerboard(blockList, font);

    // CREATE YOUR TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);

    // NOW DRAW all PIECES ON BOARD
    std::vector<Kete> keteList;
    keteList.reserve(NUM_PIECES);
    gameState.drawAllPieces(keteList);

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
                    statusText = "Clicked Cell " + std::to_string(cell->getIndex());
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
                    // TODO handle move here
                    red_piece->getIndex();
                    statusText = "Clicked Piece " + std::to_string(red_piece->getIndex());
                }
            }
            else
            {
                red_piece->removeOutline();
                // statusText.setString("");
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
