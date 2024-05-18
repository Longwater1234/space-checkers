#pragma once
#include "../PlayerType.hpp"
#include "MessageType.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace chk::payload
{
struct Welcome
{
    MessageType messageType;
    chk::PlayerType myTeam;
    std::vector<int16_t> piecesRed{};
    std::vector<int16_t> piecesBlack{};

    Welcome()
    {
        piecesRed.reserve(12);
        piecesBlack.reserve(12);
    }
};

} // namespace chk::payload