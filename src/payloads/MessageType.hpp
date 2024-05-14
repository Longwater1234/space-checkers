#pragma once

namespace chk::payload
{
enum class MessageType
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