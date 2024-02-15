# include relative dependencies
include(CPM.cmake)
include(zlib.cmake)
include(mbedtls.cmake)
include(ixwebsocket.cmake)

# GET ImGui
CPMAddPackage(
  NAME imgui
  GIT_REPOSITORY "https://gitee.com/msnh/imgui.git"
  GIT_TAG		8491cf36adb2663f16975bc4ebcd871daf5ff8b7
  GIT_SHALLOW TRUE
)


if(imgui_ADDED)
  set(IMGUI_DIR ${imgui_SOURCE_DIR})
  set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR})
  set(IMGUI_SOURCES
	${IMGUI_INCLUDE_DIR}/imgui.cpp 
	${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
	${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
	${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
	${IMGUI_INCLUDE_DIR}/misc/cpp/imgui_stdlib.cpp
  )
  include_directories(${imgui_SOURCE_DIR})
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
endif()
