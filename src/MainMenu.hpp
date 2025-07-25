//
// Created by Davis 2024-04-09
//
#pragma once
#include "AppVersion.hpp"
#include "Piece.hpp"
#include "utils/ResourcePath.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <memory>
#include <string>

namespace chk
{
constexpr auto ICON_PATH = "win-icon-16.png";
constexpr auto FONT_PATH = "notosans-regular.ttf";
constexpr unsigned FONT_SIZE{16};

enum class UserChoice
{
    LOCAL_PLAY = 38483, // playing offline
    ONLINE_PLAY,        // playing online
};

/**
 * Shown first when game is launched
 */
class MainMenu final
{
  public:
    explicit MainMenu(sf::RenderWindow *windowPtr);
    MainMenu() = delete;
    MainMenu(const MainMenu &) = delete;
    MainMenu &operator=(const MainMenu &) = delete;
    chk::UserChoice runMainLoop();

  private:
    void init();
    sf::RenderWindow *window;
    sf::Texture bgroundImage;
    sf::RectangleShape mainFrame;
    sf::RectangleShape localBtn;
    sf::RectangleShape onlineBtn;
    sf::Font font;
    sf::Text versionTxt;
    sf::Color DARK_BROWN = sf::Color{82, 55, 27};
    void handleEvents(chk::UserChoice &result);
};

inline MainMenu::MainMenu(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->mainFrame = sf::RectangleShape(sf::Vector2f(600, 700));
    if (!this->bgroundImage.loadFromFile(chk::getResourcePath("main_menu_en.png")))
    {
        perror("cannot find background image");
        exit(EXIT_FAILURE);
    }
    mainFrame.setTexture(&this->bgroundImage);
    mainFrame.setPosition(0, 0);
    sf::Image appIcon;
    if (appIcon.loadFromFile(chk::getResourcePath(ICON_PATH)))
    {
        auto dims = appIcon.getSize();
        window->setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }
    this->init();
}

/**
 * Initialize the menu buttons
 */
inline void MainMenu::init()
{
    // draw two rectangles
    sf::Vector2f sizeRec{277.0, 55.0};
    this->localBtn = sf::RectangleShape{sizeRec};
    this->onlineBtn = sf::RectangleShape{sizeRec};
    this->localBtn.setFillColor(sf::Color::Transparent);
    this->onlineBtn.setFillColor(sf::Color::Transparent);
    // position them over menu text
    this->onlineBtn.setPosition(sf::Vector2f{154.0, 476.0});
    this->localBtn.setPosition(sf::Vector2f{154.0, 558.0});
    // create version text
    if (this->font.loadFromFile(chk::getResourcePath(chk::FONT_PATH)))
    {
        this->versionTxt.setFont(this->font);
        this->versionTxt.setCharacterSize(20);
        this->versionTxt.setFillColor(this->DARK_BROWN);
        this->versionTxt.setString(chk::APP_VERSION);
        this->versionTxt.setPosition(sf::Vector2f{420.0, 410.0});
    }
}

/**
 * Listen for GUI events, and store the selected choice to `result`
 * @param result Output will be written into this
 */
inline void MainMenu::handleEvents(chk::UserChoice &result)
{
    for (auto event = sf::Event{}; window->pollEvent(event);)
    {
        if (event.type == sf::Event::Closed)
        {
            window->close();
            exit(EXIT_SUCCESS);
        }
        if (event.type == sf::Event::MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            const auto clickedPos = sf::Mouse::getPosition(*window);
            /* Check window bounds */
            if (clickedPos.y > chk::SIZE_CELL * 8)
            {
                continue;
            }
            if (this->localBtn.getGlobalBounds().contains(sf::Vector2f(clickedPos)))
            {
                result = chk::UserChoice::LOCAL_PLAY;
            }
            else if (this->onlineBtn.getGlobalBounds().contains(sf::Vector2f(clickedPos)))
            {
                result = chk::UserChoice::ONLINE_PLAY;
            }
        }
    }
}

/**
 * The main loop, renders the main menu screen at 60FPS
 * @return user choice for game Mode
 */
inline chk::UserChoice MainMenu::runMainLoop()
{
    chk::UserChoice result{};
    while (this->window->isOpen())
    {
        // HANDLE EVENTS
        this->handleEvents(result);
        if (result == chk::UserChoice::LOCAL_PLAY || result == chk::UserChoice::ONLINE_PLAY)
        {
            break;
        }
        window->clear();
        auto mousePos = sf::Mouse::getPosition(*window);
        if (this->localBtn.getGlobalBounds().contains(sf::Vector2f(mousePos)))
        {
            this->localBtn.setOutlineColor(DARK_BROWN);
            this->localBtn.setOutlineThickness(5.0f);
        }
        else
        {
            this->localBtn.setOutlineThickness(0);
        }
        if (this->onlineBtn.getGlobalBounds().contains(sf::Vector2f{mousePos}))
        {
            this->onlineBtn.setOutlineColor(DARK_BROWN);
            this->onlineBtn.setOutlineThickness(5.0f);
        }
        else
        {
            this->onlineBtn.setOutlineThickness(0);
        }

        window->draw(mainFrame);
        window->draw(localBtn);
        window->draw(onlineBtn);
        window->draw(versionTxt);
        window->display();
    }

    return result;
}

} // namespace chk