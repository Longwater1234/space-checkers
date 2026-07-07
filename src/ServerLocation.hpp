// Created by Davis on 2024-08-04

#pragma once

#include <string>

namespace chk
{
/**
 * For storing public servers
 */
struct ServerLocation
{
    std::string name{};
    std::string address{};
    int64_t playerCount{-1};
};

} // namespace chk