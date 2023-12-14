#include "CircularBuffer.hpp"
#include "GameManager.hpp"
#include "ResourcePath.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>
#include <set>
#include <vector>

constexpr uint16_t NUM_PIECES = 24;
constexpr auto ICON_PATH = "win-icon-16.png";
constexpr auto FONT_PATH = "open-sans.regular.ttf";

/**
 * When player is forced to capture opponent's piece
 * @param manager game manager
 * @param player current player
 * @param cell selected cell
 */
void showForcedMoves(const std::unique_ptr<chk::GameManager> &manager, const chk::PlayerPtr &player,
                       const chk::Block &cell)
{
    const auto &forcedMoves = manager->getForcedJumps();
    const int pieceId = manager->getPieceFromCell(cell->getIndex());
    if (pieceId > 1 && forcedMoves.find(pieceId) == forcedMoves.end())
    {
        // FORCE PLAYER TO DO JUMP, dont proceed until complete JUMP!
        std::set<int> pieceSet;
        for (const auto &pair : forcedMoves)
        {
            pieceSet.insert(pair.first);
        }
        player->showForcedMoves(pieceSet);
        manager->updateMessage(player->getName() + " must capture piece!");
    }
}

/**
 * When player taps a cell
 * @param manager game manager
 * @param player currentPlayer
 * @param buffer Temporary store for clicked Pieces
 * @param cell Tapped cell
 */
void handleCellTap(const std::unique_ptr<chk::GameManager> &manager, const chk::PlayerPtr &player,
                   chk::CircularBuffer<int> &buffer, const chk::Block &cell)
{
    // CHECK IF cell has a Piece
    const int pieceId = manager->getPieceFromCell(cell->getIndex());
    if (pieceId != -1)
    {
        // YES, it has one! CHECK IF THERE IS ANY PENDING "NECESSARY jumps"
        if (!manager->getForcedJumps().empty())
        {
            showForcedMoves(manager, player, cell);
            return;
        }
        // OTHERWISE, store it in buffer!
        manager->setSourceCell(cell->getIndex());
        buffer.addItem(pieceId);
    }
    else
    {
        // Cell is Empty! Let's move a piece (from buffer) here!
        if (!buffer.isEmpty())
        {
            const int movablePieceId = buffer.getTop();
            manager->handleMovePiece(player, cell, movablePieceId);
            buffer.clean();
        }
    }
}

void handleJumpPiece(const chk::PlayerPtr &hunter, const chk::PlayerPtr &prey, const chk::Block &cell);

int main()
{
    auto window = sf::RenderWindow{sf::VideoMode{600u, 700u}, "Checkers CPP", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60u);

    sf::Image appIcon;
    if (appIcon.loadFromFile(getResourcePath(ICON_PATH)))
    {
        auto dims = appIcon.getSize();
        window.setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }

    // LOAD FONT
    sf::Font font;
    if (!font.loadFromFile(getResourcePath(FONT_PATH )))
    {
        perror("cannot find file");
        exit(EXIT_FAILURE);
    }

    const auto manager = std::make_unique<chk::GameManager>();
    manager->drawCheckerboard(font);

    // CREATE TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);

    // NOW DRAW all PIECES ON BOARD
    std::vector<chk::PiecePtr> keteList;
    keteList.reserve(NUM_PIECES);
    manager->drawAllPieces(keteList);
    manager->matchCellsToPieces(keteList);

    // Give each player their own pieces
    for (auto &kete : keteList)
    {
        if (kete->getPieceType() == chk::PieceType::Red)
        {
            p1->recievePiece(kete);
        }
        else
        {
            p2->recievePiece(kete);
        }
    }

    // we don't need this anymore
    keteList.clear();

    // Our temp store with maxCap of 1
    chk::CircularBuffer<int> circularBuffer(1);

    // THE STATUS TEXT
    sf::Text txtPanel;
    txtPanel.setFont(font);
    txtPanel.setCharacterSize(16u);
    txtPanel.setFillColor(sf::Color::White);
    txtPanel.setPosition(sf::Vector2f(0, 650));

    manager->updateMessage("Now playing!");
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
                if (clickedPos.y <= chk::SIZE_CELL * 8)
                {
                    for (auto &cell : manager->getBlockList())
                    {
                        if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
                        {
                            const auto &currentPlayer = manager->isPlayerRedTurn() ? p1 : p2;
                            handleCellTap(manager, currentPlayer, circularBuffer, cell);
                            break;
                        }
                    }
                }
            }
        }

        auto mousePos = sf::Mouse::getPosition(window);
        window.clear();

        for (const auto &cell : manager->getBlockList())
        {
            window.draw(*cell);
        }
        // DRAW RED PIECES
        for (const auto &[id, red_piece] : p1->getOwnPieces())
        {
            if (manager->isPlayerRedTurn() && red_piece->containsPoint(mousePos))
            {
                red_piece->addOutline();
            }
            else
            {
                red_piece->removeOutline();
            }
            window.draw(*red_piece);
        }
        // DRAW BLACK PIECES
        for (const auto &[id, black_piece] : p2->getOwnPieces())
        {
            if (!manager->isPlayerRedTurn() && black_piece->containsPoint(mousePos))
            {
                black_piece->addOutline();
            }
            else
            {
                black_piece->removeOutline();
            }
            window.draw(*black_piece);
        }
        txtPanel.setString(manager->getCurrentMsg());
        window.draw(txtPanel);
        window.display();
    }
    return EXIT_SUCCESS;
}
