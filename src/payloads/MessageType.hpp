#pragma once
#include <cstdint>
namespace chk::payload
{
enum class MessageType : uint16_t
{
    WELCOME = 49,
    START,
    EXIT,
    MOVE,
    CAPTURE,
    WIN,
    LOSE,
};
} // namespace chk::payload