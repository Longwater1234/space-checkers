#include "ResourcePath.hpp"

std::string getResourcePath(const std::string &relativePath)
{
    return "resources/" +  relativePath;
}