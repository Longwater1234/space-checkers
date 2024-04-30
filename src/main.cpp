#include "CircularBuffer.hpp"
#include "MainMenu.hpp"
#include "ResourcePath.hpp"
#include "WsClient.hpp"
#include "managers/LocalGameManager.hpp"
#include "managers/OnlineGameManager.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cassert>
#include <set>
#include <vector>

#include "imgui-SFML.h"
#include "imgui.h"

constexpr uint16_t NUM_PIECES{24};

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
 * When current player taps any playable cell.
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

    // CHECK IF this cell has a Piece
    const short pieceId = manager->getPieceFromCell(cell->getIndex());
    if (pieceId != -1)
    {
        // YES, it has one! CHECK IF THERE IS ANY PENDING "forced jumps"
        if (!manager->getForcedMoves().empty())
        {
            showForcedMoves(manager, player, cell);
            return;
        }
        // OTHERWISE, store it in buffer (for a simple move next)!
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
            {
                return;
            }
            manager->handleMovePiece(player, opponent, cell, movablePieceId);
            buffer.clean();
        }
    }
}

int main()
{

    auto window = sf::RenderWindow{sf::VideoMode{600, 700}, "SpaceCheckers", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window, false);

    std::unique_ptr<chk::GameManager> manager = nullptr;
    chk::MainMenu homeMenu(&window);
    homeMenu.init();
    const auto userChoice = homeMenu.runLoop();
    if (userChoice == chk::UserChoice::ONLINE_PLAY)
    {
        manager = std::make_unique<chk::OnlineGameManager>(&window);
    }
    else
    {
        manager = std::make_unique<chk::LocalGameManager>(&window);
    }

    // LOAD FONT for IMGUI
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImFont *imfont = io.Fonts->AddFontFromFileTTF(chk::getResourcePath(chk::FONT_PATH).c_str(), 16);
    IM_ASSERT(imfont != nullptr);
    ImGui::SFML::UpdateFontTexture();
    // ImGui::StyleColorsLight(); //LIGHT THEME

    sf::Image appIcon;
    if (appIcon.loadFromFile(chk::getResourcePath(chk::ICON_PATH)))
    {
        auto dims = appIcon.getSize();
        window.setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }

    // LOAD FONT for SFML
    sf::Font font;
    if (!font.loadFromFile(chk::getResourcePath(chk::FONT_PATH)))
    {
        perror("cannot find font file");
        exit(EXIT_FAILURE);
    }

    manager->drawCheckerboard(font);

    // CREATE TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);
    assert(!(*p1 == *p2));

    // NOW create all PIECES ON BOARD
    std::vector<chk::PiecePtr> pieceVector;
    pieceVector.reserve(NUM_PIECES);

    // wait for manager to create pieces
    manager->setOnReadyCreatePiecesCallback([&manager, &p1, &p2, &pieceVector](const bool &isReady) {
        if (!isReady)
        {
            return;
        }
        manager->matchCellsToPieces(pieceVector);
        // GIVE EACH PLAYER their own piece
        for (auto &kete : pieceVector)
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
    });

    manager->createAllPieces(pieceVector);

    /* we don't need this anymore */
    pieceVector.clear();

    // for storing currently clicked Piece
    chk::CircularBuffer<short> circularBuffer{1};

    // THE STATUS TEXT
    sf::Text txtPanel{"Welcome to Checkers", font, 16};
    txtPanel.setFillColor(sf::Color::White);
    txtPanel.setPosition(sf::Vector2f{0, 8.5 * chk::SIZE_CELL});
    manager->updateMessage("Now playing! RED starts");

    auto wsClient = std::make_unique<chk::WsClient>(manager.get(), p1.get(), p2.get());

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
                if (clickedPos.y > chk::SIZE_CELL * 8)
                {
                    continue;
                }
                for (auto &cell : manager->getBlockList())
                {
                    // inner loop
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
                        break; // END inner loop
                    }
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        auto mousePos = sf::Mouse::getPosition(window);
        window.clear();
        // START IMGUI
        if (wsClient->doneConnectWindow())
        {
            wsClient->tryConnect();
        }

        // RENDER CHECKERBOARD
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
