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
    sf::Font font;
    sf::Text versionTxt;
    static inline const sf::Color DARK_BROWN{82, 55, 27};
    void handleEvents(chk::UserChoice &result);
};

} // namespace chk