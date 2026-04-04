#include "StartMenu.hpp"

namespace chk
{
chk::MainMenu::MainMenu(sf::RenderWindow *windowPtr)
{
    this->window = windowPtr;
    this->mainFrame = sf::RectangleShape(sf::Vector2f{600.f, 700.f});
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
void MainMenu::init()
{
    // draw two rectangles
    sf::Vector2f sizeRec{277.0f, 55.0f};
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
void MainMenu::handleEvents(chk::UserChoice &result)
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
chk::UserChoice MainMenu::runMainLoop()
{
    chk::UserChoice result{};
    constexpr float HOVER_THICKNESS = 5.0f;
    constexpr float NORMAL_THICKNESS = 0.0f;

    while (this->window->isOpen())
    {
        // HANDLE EVENTS
        this->handleEvents(result);
        if (result == chk::UserChoice::LOCAL_PLAY || result == chk::UserChoice::ONLINE_PLAY)
        {
            break;
        }

        const sf::Vector2f mousePos{sf::Mouse::getPosition(*window)};

        // hover state
        const bool isLocal = this->localBtn.getGlobalBounds().contains(mousePos);
        const bool isOnline = this->onlineBtn.getGlobalBounds().contains(mousePos);

        // Apply outline style based on hover
        auto applyHover = [&](sf::RectangleShape &btn, bool hover) {
            btn.setOutlineThickness(hover ? HOVER_THICKNESS : NORMAL_THICKNESS);
            if (hover)
            {
                btn.setOutlineColor(DARK_BROWN);
            }
        };

        applyHover(this->localBtn, isLocal);
        applyHover(this->onlineBtn, isOnline);

        window->clear();
        window->draw(mainFrame);
        window->draw(localBtn);
        window->draw(onlineBtn);
        window->draw(versionTxt);
        window->display();
    }
    return result;
}

} // namespace chk