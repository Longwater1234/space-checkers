#include "ResourcePath.hpp"

#ifdef _WIN32
std::string getResourcePath(const std::string &relativePath)
{
    return "resources/" +  relativePath;
}

#endif // _WIN32
