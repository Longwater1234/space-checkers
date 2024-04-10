//
// Created by Davis 2024-04-09
//
#pragma once
#include "ResourcePath.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
namespace chk
{

class SplashScreen final
{
  public:
    SplashScreen();
    void handleEvents();
    void show();

  private:
    sf::RenderWindow window;
    std::string bkgImagePath = "";
    sf::Image image;
};

inline SplashScreen::SplashScreen()
{
    this->window = sf::RenderWindow(sf::VideoMode{600, 700}, "SpaceCheckers", sf::Style::Titlebar | sf::Style::Close);
    this->window.setFramerateLimit(60);
}

inline void SplashScreen::handleEvents()
{
}

inline void SplashScreen::show()
{
}

} // namespace chk