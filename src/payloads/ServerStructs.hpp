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
    std::string_view notice{};
};

/* Received when first connected to server */
struct Welcome : public BasePayload
{
    chk::PlayerType myTeam{};
};

/* Received when both players have joined game */
struct StartGame : public BasePayload
{
    std::vector<int16_t> piecesRed{};
    std::vector<int16_t> piecesBlack{};

    StartGame()
    {
        piecesRed.reserve(12);
        piecesBlack.reserve(12);
    }
};

struct DestCell
{
    float x{0};
    float y{0};
};

struct MovePayload : public BasePayload
{

    int64_t fromTeam;
    int64_t pieceId;
    DestCell destCell;
    int srcCell;
};

} // namespace chk::payload