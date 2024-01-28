include(FetchContent)

set(IMGUI_VERSION 1.87)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY "https://gitee.com/msnh/imgui.git"
  GIT_TAG		3733b5064e1820dc8517fedba2b34bfbdef569ab
  GIT_SHALLOW
)

# Or by tag...
FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY "https://gitee.com/i4xin/imgui-sfml.git"
  GIT_TAG        2a4b9dd9b34df61d91e33667dbaf5902c4923867
  GIT_SHALLOW
  # GIT_COMMIT 5f54b69b6aee18db846c81633f86f78c2586dded
  # ^ or like this - sometimes it's better because FetchContent won't look
  # into remote to see if branch head was updated or not - good for stable
  # tags like 'vX.X' corresponding to releases
)

## finally, let's build

# Dear ImGui
FetchContent_MakeAvailable(imgui)

# ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)

FetchContent_MakeAvailable(imgui-sfml)
