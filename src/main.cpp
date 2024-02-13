#include "CircularBuffer.hpp"
#include "GameManager.hpp"
#include "ResourcePath.hpp"
#include "WsClient.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include "imgui-SFML.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

constexpr uint16_t NUM_PIECES = 24u;
constexpr auto ICON_PATH = "win-icon-16.png";
constexpr auto FONT_PATH = "open-sans.regular.ttf";

/**
 * When player is forced to capture opponent's piece, highlight their pieces.
 * @param manager game manager
 * @param player current player
 * @param cell selected cell
 */
static void showForcedMoves(const std::unique_ptr<chk::GameManager> &manager, const chk::PlayerPtr &player,
                            const chk::Block &cell)
{
    const auto &forcedMoves = manager->getForcedMoves();
    const short pieceId = manager->getPieceFromCell(cell->getIndex());
    if (forcedMoves.find(pieceId) == forcedMoves.end())
    {
        // FORCE PLAYER TO DO JUMP, don't proceed until done!
        std::set<short> pieceSet;
        for (const auto &[hunter_piece, captureTarget] : forcedMoves)
        {
            pieceSet.insert(hunter_piece);
        }
        player->showForcedPieces(pieceSet);
        manager->updateMessage(player->getName() + " must capture piece!");
    }
    else
    {
        manager->setSourceCell(cell->getIndex());
    }
}

/**
 * When current player taps a cell.
 * @param manager game manager
 * @param player currentPlayer
 * @param buffer Temporary store for clicked Pieces
 * @param cell Tapped cell
 */
static void handleCellTap(const std::unique_ptr<chk::GameManager> &manager, const chk::PlayerPtr &player,
                          const chk::PlayerPtr &opponent, chk::CircularBuffer<short> &buffer, const chk::Block &cell)
{
    if (manager->isGameOver())
        return;

    // CHECK IF cell has a Piece
    const short pieceId = manager->getPieceFromCell(cell->getIndex());
    if (pieceId != -1)
    {
        // YES, it has one! CHECK IF THERE IS ANY PENDING "forced jumps"
        if (!manager->getForcedMoves().empty())
        {
            showForcedMoves(manager, player, cell);
            return;
        }
        // OTHERWISE, store it in buffer!
        buffer.addItem(pieceId);
        manager->setSourceCell(cell->getIndex());
    }
    else
    {
        // Cell is Empty! Let's move a piece (from buffer) here!
        if (!buffer.isEmpty())
        {
            const short movablePieceId = buffer.getTop();
            if (!player->hasThisPiece(movablePieceId))
                return;
            manager->handleMovePiece(player, opponent, cell, movablePieceId);
            buffer.clean();
        }
    }
}

int main()
{
    auto window = sf::RenderWindow{sf::VideoMode{600, 700}, "SpaceCheckers", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60u);
    ImGui::SFML::Init(window);

    // LOAD FONT FOR IMGUI
    ImGuiIO &io = ImGui::GetIO();
    ImFont *imfont = io.Fonts->AddFontFromFileTTF(chk::getResourcePath(FONT_PATH).c_str(), 16.0f);
    IM_ASSERT(imfont != nullptr);
    ImGui::SFML::UpdateFontTexture();

    sf::Image appIcon;
    if (appIcon.loadFromFile(chk::getResourcePath(ICON_PATH)))
    {
        auto dims = appIcon.getSize();
        window.setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }

    // LOAD FONT
    sf::Font font;
    if (!font.loadFromFile(chk::getResourcePath(FONT_PATH)))
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

    // WsClient wsClient{"wss://echo.websocket.org"};
    // std::thread t1(wsClient);

    // Give each player their own pieces
    for (auto &kete : keteList)
    {
        if (kete->getPieceType() == chk::PieceType::Red)
        {
            p1->receivePiece(kete);
        }
        else
        {
            p2->receivePiece(kete);
        }
    }

    // we don't need this anymore
    keteList.clear();

    // Our temp store with maxCap of 1
    chk::CircularBuffer<short> circularBuffer{1};

    // THE STATUS TEXT
    sf::Text txtPanel{"Welcome to Checkers", font, 16};
    txtPanel.setFillColor(sf::Color::White);
    txtPanel.setPosition(sf::Vector2f{0, 8.5 * chk::SIZE_CELL});
    manager->updateMessage("Now playing! RED starts");

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            ImGui::SFML::ProcessEvent(window, event);
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
                            const auto &hunter = manager->isPlayerRedTurn() ? p1 : p2;
                            const auto &prey = manager->isPlayerRedTurn() ? p2 : p1;

                            if (manager->hasPendingCaptures())
                            {
                                manager->handleJumpPiece(hunter, prey, cell);
                                manager->updateMatchStatus(hunter, prey);
                                circularBuffer.clean();
                            }
                            else
                            {
                                handleCellTap(manager, hunter, prey, circularBuffer, cell);
                            }
                            break;
                        }
                    }
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
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
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return EXIT_SUCCESS;
}
