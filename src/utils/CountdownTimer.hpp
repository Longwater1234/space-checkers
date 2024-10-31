// created by davis on 2024-10-31
#pragma once
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
namespace chk

{
class CountdownTimer
{
  public:
    explicit CountdownTimer(const sf::Font &font);
    CountdownTimer() = delete;
    CountdownTimer &operator=(const CountdownTimer &) = delete;
    CountdownTimer(const CountdownTimer &) = delete;

  private:
    sf::Text sfText;
    int32_t currentVal{0};
};

inline CountdownTimer::CountdownTimer(const sf::Font &font)
{
}

} // namespace chk
