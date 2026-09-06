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
    NONE = 0,           // nothing picked yet
    LOCAL_PLAY = 38483, // offline, two humans
    ONLINE_PLAY,        // online, versus a remote player
    BOT_PLAY,           // offline, versus the "Ibox Offline" engine
};

/**
 * Shown first when game is launched
 */
class StartMenu final
{
  public:
    explicit StartMenu(sf::RenderWindow *windowPtr);
    StartMenu() = delete;
    StartMenu(const StartMenu &) = delete;
    StartMenu &operator=(const StartMenu &) = delete;
    chk::UserChoice runMainLoop();

  private:
    void init();
    sf::RenderWindow *window;
    sf::Texture bgroundImage;
    sf::RectangleShape mainFrame;
    sf::RectangleShape localBtn;
    sf::RectangleShape onlineBtn;
    // The bot entry is drawn by us rather than baked into the background art,
    // so it gets its own filled "pill" plus a label.
    sf::RectangleShape botBtn;
    sf::Text botTxt;
    sf::Font font;
    sf::Text versionTxt;
    inline static const sf::Color DARK_BROWN{82, 55, 27};
    inline static const sf::Color LIGHT_BROWN{124, 88, 46};
    inline static const sf::Color CREAM{255, 225, 151};
    void handleEvents(chk::UserChoice &result);
};

} // namespace chk