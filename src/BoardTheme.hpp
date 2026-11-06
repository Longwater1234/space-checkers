//
// Created by Davis on 2026-11-04
//
#pragma once

#include <SFML/Graphics/Color.hpp>
#include <array>
#include <string_view>

namespace chk
{

struct BoardTheme
{
    std::string_view name;
    sf::Color darkCell;       // Playable cells (odd cells, counter 1~32)
    sf::Color lightCell;      // Unused cells (even cells, index -1)
    sf::Color highlightColor; // Active/selected cell
    sf::Color indexTextColor; // Number label on dark cells
};

// 4 Board Themes:
// 1. Deep Space (Default Sci-Fi)
// 2. Cyberpunk
// 3. Greenland (Chess.com default)
// 4. Classic Wood (Original)
inline const std::array BOARD_THEMES = {
    // clang-format off
    chk::BoardTheme{
        "Deep Space", sf::Color{24, 28, 52}, // Deep void indigo
        sf::Color{130, 145, 185},            // Soft starlight
        sf::Color{0, 240, 255},              // Neon cyan
        sf::Color{255, 255, 255, 120}        // white numbers
    },
    chk::BoardTheme{
        "Cyberpunk", sf::Color{20, 22, 28}, // dark charcoal
        sf::Color{45, 52, 68},              // grey blue
        sf::Color{255, 42, 109},            // Hot pink
        sf::Color{255, 255, 255, 150}       // White numbers
    },
    chk::BoardTheme{
        "Greenland", sf::Color{118, 150, 86}, // Forest green
        sf::Color{238, 238, 210},             // Warm parchment/buff
        sf::Color{247, 247, 105},             // Solar amber/yellow
        sf::Color{255, 255, 255, 120}         // white numbers
    },
    chk::BoardTheme{
        "Classic Wood", sf::Color{82, 55, 27}, // Walnut brown
        sf::Color{255, 225, 151},              // Cream
        sf::Color{98, 174, 239},               // Baby blue
        sf::Color{255, 255, 255, 100}          // Original text color
    }
    // clang-format on
};

} // namespace chk
