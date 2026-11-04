# AUTO-download extra dependencies.
# DO NOT change the order below:
include(CPM.cmake)
include(gtest.cmake)
include(zlib.cmake)
include(mbedtls.cmake)
include(ixwebsocket.cmake)
include(libcpr.cmake)
include(spdlog.cmake)
include(simdjson.cmake)
include(protobuf.cmake)

if (NOT CHECKERS_USE_SYSTEM_SFML)
    include(sfml.cmake) ## <-- uncomment to download and build SFML from source
endif(CHECKERS_USE_SYSTEM_SFML)

include(imgui.cmake)
