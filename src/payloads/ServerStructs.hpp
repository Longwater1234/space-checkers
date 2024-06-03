#pragma once
#include "../PlayerType.hpp"
#include "MessageType.hpp"
#include <cstdint>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace chk::payload
{
struct BasePayload
{
    MessageType messageType{};
};

/* Received when first connected to server */
struct Welcome : public BasePayload
{
    chk::PlayerType myTeam{};
    std::string_view notice{};
};

/* Received when both players have joined game, kicks off match */
struct StartGame : public BasePayload
{
    std::string_view notice{};
    std::vector<int16_t> piecesRed{};
    std::vector<int16_t> piecesBlack{};

    StartGame()
    {
        piecesRed.reserve(12);
        piecesBlack.reserve(12);
    }
};

} // namespace chk::payload