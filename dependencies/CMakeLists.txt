# include(FetchContent)

include(CPM.cmake)
include(zlib.cmake)
include(mbedtls.cmake)
include(ixwebsocket.cmake)

# GET ImGui
CPMAddPackage(
  NAME imgui
  GIT_REPOSITORY "https://gitee.com/msnh/imgui.git"
  GIT_TAG		3733b5064e1820dc8517fedba2b34bfbdef569ab
  GIT_SHALLOW TRUE
)


if(imgui_ADDED)
  set(IMGUI_DIR ${imgui_SOURCE_DIR})
  set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR})
endif()


# GET ImGui-SFML
CPMAddPackage(
  NAME imgui-sfml
  GIT_REPOSITORY "https://gitee.com/i4xin/imgui-sfml.git"
  GIT_TAG        2a4b9dd9b34df61d91e33667dbaf5902c4923867
  GIT_SHALLOW TRUE
)

if(imgui-sfml_ADDED)
  option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
  option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)
  set(IMGUI_SFML_IMGUI_DEMO TRUE)
endif()

## finally, let's build

# Dear ImGui
# FetchContent_MakeAvailable(imgui)


# FetchContent_MakeAvailable(imgui-sfml)
