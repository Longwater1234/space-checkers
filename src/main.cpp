#include "Cell.hpp"
#include "Piece.hpp"
#include "Player.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <memory>
#include <random>
#include <vector>

constexpr uint16_t NUM_ROWS = 8;
constexpr uint16_t NUM_COLS = 8;
constexpr uint16_t NUM_PIECES = 24;
constexpr auto ICON_PATH = "resources/icons8-checkers-16.png";
constexpr auto FONT_PATH = "resources/open-sans.regular.ttf";

using Block = std::unique_ptr<chk::Cell>;
using Kete = std::unique_ptr<chk::Piece>;

/**
 * draw red and white checkboard cells
 * @param blockList empty list of cells
 * @param font      for text inside cells
 */
void drawCheckerboard(std::vector<Block> &blockList, const sf::Font &font)
{
    int counter = 32;
    for (size_t row = 0; row < NUM_ROWS; row++)
    {
        for (size_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 == 0)
            {
                // even CELL, set LIGHTER color
                sf::RectangleShape lightRec(sf::Vector2f(100.f, 100.f));
                lightRec.setFillColor(sf::Color{255, 225, 151});
                float x = (col % NUM_COLS) * 100.0f;
                lightRec.setPosition(sf::Vector2f(x, row * 100.0f));
                auto whiteBlock = std::make_unique<chk::Cell>(lightRec, lightRec.getPosition(), 0);
                whiteBlock->setFont(font);
                blockList.emplace_back(std::move(whiteBlock));
            }
            else
            {
                // Odd cell, SET DARKER RED
                sf::RectangleShape darkRect(sf::Vector2f(100.f, 100.f));
                darkRect.setFillColor(sf::Color{82, 55, 27});
                float x = (col % NUM_COLS) * 100.0f;
                darkRect.setPosition(sf::Vector2f(x, row * 100.0f));
                auto redBlock = std::make_unique<chk::Cell>(darkRect, darkRect.getPosition(), counter);
                redBlock->setFont(font);
                blockList.emplace_back(std::move(redBlock));
                counter--;
            }
        }
    }
}

/**
 * Create new checker pieces, each with own position, and add them to vector<Pieces>
 * @param pieceList destination
 */
void drawAllPieces(std::vector<Kete> &pieceList)
{
    std::random_device randomDevice;
    std::mt19937 randEngine(randomDevice());
    std::uniform_int_distribution<int> dist(1, 100);
    for (size_t row = 0; row < NUM_ROWS; row++)
    {
        for (size_t col = 0; col < NUM_COLS; col++)
        {
            if ((row + col) % 2 != 0)
            {
                // Put piece on Odd cells only
                sf::CircleShape circle(50.0f);
                const float x = (col % NUM_COLS) * 100.0f;
                circle.setPosition(sf::Vector2f(x, row * 100.0f));
                if (row < 3)
                {
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Black, dist(randEngine));
                    pieceList.emplace_back(std::move(kete));
                }
                else if (row > 4)
                {
                    auto kete = std::make_unique<chk::Piece>(circle, chk::PieceType::Red, dist(randEngine));
                    pieceList.emplace_back(std::move(kete));
                }
            }
        }
    }
}

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
    blockList.reserve(NUM_COLS * NUM_ROWS);
    sf::Font font;
    if (!font.loadFromFile(FONT_PATH))
    {
        perror("cannot find file");
        exit(EXIT_FAILURE);
    }

    drawCheckerboard(blockList, font);

    // CREATE YOUR TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);

    // NOW DRAW all PIECES ON BOARD
    std::vector<Kete> keteList;
    drawAllPieces(keteList);

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
                    statusText = "clicked " + std::to_string(red_piece->getIndex());
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
