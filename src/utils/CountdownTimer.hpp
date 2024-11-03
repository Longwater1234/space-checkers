// created by davis on 2024-10-31
#pragma once
#include <cstdint>
namespace chk

{
class CountdownTimer
{
  public:
    explicit CountdownTimer();
    CountdownTimer() = delete;
    CountdownTimer &operator=(const CountdownTimer &) = delete;
    CountdownTimer(const CountdownTimer &) = delete;

  private:
    int32_t currentVal{0};
};

inline CountdownTimer::CountdownTimer()
{
}

} // namespace chk
