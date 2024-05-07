#include "CircularBuffer.hpp"
#include "MainMenu.hpp"
#include "ResourcePath.hpp"
#include "WsClient.hpp"
#include "managers/LocalGameManager.hpp"
#include "managers/OnlineGameManager.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cassert>
#include <vector>

#include "imgui-SFML.h"
#include "imgui.h"

int main()
{
    auto window = sf::RenderWindow{sf::VideoMode{600, 700}, "SpaceCheckers", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window, false);
    std::unique_ptr<chk::GameManager> manager = nullptr;

    // SHOW MAIN MENU
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

    // NOW create all PIECES ON BOARD
    std::vector<chk::PiecePtr> pieceVector;
    pieceVector.reserve(chk::NUM_PIECES);

    // CREATE TWO unique PLAYERS
    auto p1 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_1);
    auto p2 = std::make_unique<chk::Player>(chk::PlayerType::PLAYER_2);
    assert(!(*p1 == *p2));

    // wait for manager to create pieces
    manager->setOnReadyPiecesCallback([&manager, &p1, &p2, &pieceVector](const bool &isReady) {
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

    // initialize Websocket client (only ONLINE PLAY)
    std::unique_ptr<chk::WsClient> wsClient = nullptr;
    if (userChoice == chk::UserChoice::ONLINE_PLAY)
    {
        wsClient = std::make_unique<chk::WsClient>(manager.get(), p1.get(), p2.get());
    }

    // THE MAIN GAME LOOP
    sf::Clock deltaClock;
    while (window.isOpen())
    {
        manager->handleEvents(p1, p2, circularBuffer);
        ImGui::SFML::Update(window, deltaClock.restart());

        window.clear();
        manager->drawScreen(p1, p2);

        // draw IMGUI elements (online only)
        if (wsClient != nullptr && wsClient->doneConnectWindow())
        {
            wsClient->tryConnect();
        }
        txtPanel.setString(manager->getCurrentMsg());
        window.draw(txtPanel);
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return EXIT_SUCCESS;
}
