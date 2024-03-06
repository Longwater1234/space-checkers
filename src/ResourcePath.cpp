// this is for Windows and Linux only
#ifndef __APPLE__
#include "ResourcePath.hpp"

std::string chk::getResourcePath(const std::string &relativePath)
{
    return "resources/" + relativePath;
}

#endif