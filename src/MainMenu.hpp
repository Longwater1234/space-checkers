//
// Created by Davis 2024-04-09
//
#pragma once
#include "ResourcePath.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <utility>
namespace chk
{
constexpr auto ICON_PATH = "win-icon-16.png";
class MainMenu final
{
  public:
    MainMenu();
    void handleEvents();
    int runLoop();

  private:
    std::unique_ptr<sf::RenderWindow> window;
    sf::Texture mainImage;
    sf::RectangleShape rectangle;
    bool imageOk = false;
};

inline MainMenu::MainMenu()
{
    this->window = std::make_unique<sf::RenderWindow>(sf::VideoMode{600, 700}, "SpaceCheckers",
                                                      sf::Style::Titlebar | sf::Style::Close);
    this->window->setFramerateLimit(60);
    this->rectangle = sf::RectangleShape(sf::Vector2f(600, 700));
    if (this->mainImage.loadFromFile(chk::getResourcePath("main_menu.png")))
    {
        this->imageOk = true;
    }
    sf::Image appIcon;
    if (appIcon.loadFromFile(chk::getResourcePath(ICON_PATH)))
    {
        auto dims = appIcon.getSize();
        window->setIcon(dims.x, dims.y, appIcon.getPixelsPtr());
    }
}

inline void MainMenu::handleEvents()
{
}

inline int MainMenu::runLoop()
{
    if (imageOk)
    {
        rectangle.setTexture(&this->mainImage);
        rectangle.setPosition(0, 0);
    }
    while (this->window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
        }
        window->clear();
        window->draw(rectangle);
        window->display();
    }
    return 1;
}

} // namespace chk