#pragma once
#include "../PlayerType.hpp"
#include "MessageType.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace chk::payload
{
struct Welcome
{
    /* data */
    MessageType messageType;
    chk::PlayerType myTeam;
    std::vector<int16_t> piecesRed;
    std::vector<int16_t> piecesBlack;
};

} // namespace chk::payload