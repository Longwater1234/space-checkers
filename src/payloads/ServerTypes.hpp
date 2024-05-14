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
    int64_t messageType;
    int64_t myTeam;
    std::vector<int16_t> piecesRed;
    std::vector<int16_t> piecesBlack;
};

} // namespace chk::payload