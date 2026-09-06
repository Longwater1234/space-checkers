#include "StartMenu.hpp"

namespace chk
{

chk::StartMenu::StartMenu(sf::RenderWindow *windowPtr)
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
void StartMenu::init()
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
    // The third entry sits in the gap between "offline play" and the copyright
    // line. It is drawn as a filled pill so it reads as a deliberate new option
    // rather than a mismatched imitation of the background artwork.
    this->botBtn = sf::RectangleShape{sf::Vector2f{340.0f, 44.0f}};
    this->botBtn.setPosition(sf::Vector2f{130.0f, 606.0f});
    this->botBtn.setFillColor(this->DARK_BROWN);

    // create version text
    if (this->font.loadFromFile(chk::getResourcePath(chk::FONT_PATH)))
    {
        this->versionTxt.setFont(this->font);
        this->versionTxt.setCharacterSize(20);
        this->versionTxt.setFillColor(this->DARK_BROWN);
        this->versionTxt.setString(chk::APP_VERSION);
        this->versionTxt.setPosition(sf::Vector2f{420.0, 410.0});

        this->botTxt.setFont(this->font);
        this->botTxt.setCharacterSize(20);
        this->botTxt.setFillColor(this->CREAM);
        this->botTxt.setString("play vs IBOX OFFLINE");
        // centre the label inside the pill
        const sf::FloatRect tb = this->botTxt.getLocalBounds();
        const sf::Vector2f bp = this->botBtn.getPosition();
        const sf::Vector2f bs = this->botBtn.getSize();
        this->botTxt.setPosition(
            sf::Vector2f{bp.x + (bs.x - tb.width) * 0.5f - tb.left, bp.y + (bs.y - tb.height) * 0.5f - tb.top});
    }
}

/**
 * Listen for GUI events, and store the selected choice to `result`
 * @param result Output will be written into this
 */
void StartMenu::handleEvents(chk::UserChoice &result)
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
            /* Check window bounds.
             * NOTE: this used to clamp at SIZE_CELL * 8 (600 px), which is the
             * height of the *board*, not of the menu. That silently swallowed
             * clicks on the bottom 13 px of "offline play" and would have made
             * the new bot button unclickable entirely. */
            if (clickedPos.y < 0 || clickedPos.y > static_cast<int>(window->getSize().y))
            {
                continue;
            }
            const sf::Vector2f clickF{clickedPos};
            if (this->localBtn.getGlobalBounds().contains(clickF))
            {
                result = chk::UserChoice::LOCAL_PLAY;
            }
            else if (this->onlineBtn.getGlobalBounds().contains(clickF))
            {
                result = chk::UserChoice::ONLINE_PLAY;
            }
            else if (this->botBtn.getGlobalBounds().contains(clickF))
            {
                result = chk::UserChoice::BOT_PLAY;
            }
        }
    }
}

/**
 * The main loop, renders the main menu screen at 60FPS
 * @return user choice for game Mode
 */
chk::UserChoice StartMenu::runMainLoop()
{
    chk::UserChoice result{chk::UserChoice::NONE};
    constexpr float HOVER_THICKNESS = 5.0f;
    constexpr float NORMAL_THICKNESS = 0.0f;

    while (this->window->isOpen())
    {
        // HANDLE EVENTS
        this->handleEvents(result);
        if (result != chk::UserChoice::NONE)
        {
            break;
        }

        const sf::Vector2f mousePos{sf::Mouse::getPosition(*window)};

        // hover state
        const bool isLocal = this->localBtn.getGlobalBounds().contains(mousePos);
        const bool isOnline = this->onlineBtn.getGlobalBounds().contains(mousePos);
        const bool isBot = this->botBtn.getGlobalBounds().contains(mousePos);

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
        // The bot pill is filled, so it brightens instead of gaining an outline.
        this->botBtn.setFillColor(isBot ? LIGHT_BROWN : DARK_BROWN);

        window->clear();
        window->draw(mainFrame);
        window->draw(localBtn);
        window->draw(onlineBtn);
        window->draw(botBtn);
        window->draw(botTxt);
        window->draw(versionTxt);
        window->display();
    }
    return result;
}

} // namespace chk