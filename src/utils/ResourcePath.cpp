// this is for Windows and Linux only
#if defined(_WIN32) || defined(__linux__)
#include "ResourcePath.hpp"

namespace chk
{
std::string getResourcePath(const std::string &relativePath)
{
    return "resources/" + relativePath;
}

} // namespace chk

#endif