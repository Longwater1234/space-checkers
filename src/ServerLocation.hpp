#pragma once
// Created by Davis on 2024-08-04
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
};

} // namespace chk