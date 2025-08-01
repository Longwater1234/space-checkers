﻿#include "CircularBuffer.hpp"
#include "MainMenu.hpp"
#include "managers/LocalGameManager.hpp"
#include "managers/OnlineGameManager.hpp"
#include "utils/ResourcePath.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cassert>
#include <google/protobuf/stubs/common.h>
#include <vector>

#include "imgui-SFML.h"
#include "imgui.h"

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    auto window = sf::RenderWindow{sf::VideoMode{600, 700}, "SpaceCheckers", sf::Style::Titlebar | sf::Style::Close};
    window.setFramerateLimit(60);
    (void)ImGui::SFML::Init(window, false);
    // ImGui::StyleColorsLight(); //<-- light color theme
    std::unique_ptr<chk::GameManager> manager = nullptr;

    // SHOW MAIN MENU
    chk::MainMenu homeMenu{&window};
    const auto userChoice = homeMenu.runMainLoop();
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
    ImFont *imfont = io.Fonts->AddFontFromFileTTF(chk::getResourcePath(chk::FONT_PATH).c_str(), chk::FONT_SIZE);
    IM_ASSERT(imfont != nullptr);
    (void)ImGui::SFML::UpdateFontTexture();

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
        std::perror("cannot find font file");
        exit(EXIT_FAILURE);
    }

    // create all cells
    manager->drawCheckerboard(font);

    // create pieces and give each player their own
    manager->createAllPieces();

    // Storing currently clicked Piece. (NOTE: using curly braces for constructor)
    chk::CircularBuffer<short> circularBuffer{1};

    // THE STATUS TEXT
    sf::Text txtPanel{"Space Checkers", font, chk::FONT_SIZE};
    txtPanel.setFillColor(sf::Color::White);
    txtPanel.setPosition(sf::Vector2f{10.0f, 8.5 * chk::SIZE_CELL});
    manager->updateMessage("Welcome to Space Checkers");

    if (userChoice == chk::UserChoice::LOCAL_PLAY)
    {
        manager->updateMessage("Now playing! It's RED's turn");
    }

    // THE MAIN GAME LOOP
    sf::Clock deltaClock;
    while (window.isOpen())
    {
        manager->handleEvents(circularBuffer);
        ImGui::SFML::Update(window, deltaClock.restart());

        window.clear();
        manager->drawBoard();

        txtPanel.setString(manager->getCurrentMsg());
        window.draw(txtPanel);
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    google::protobuf::ShutdownProtobufLibrary();
    return EXIT_SUCCESS;
}
