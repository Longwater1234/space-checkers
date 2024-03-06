# include relative dependencies
include(CPM.cmake)
include(zlib.cmake)
include(mbedtls.cmake)
include(ixwebsocket.cmake)

# GET ImGui 1.90.4
CPMAddPackage(
  NAME imgui
  URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.90.4.tar.gz"
  VERSION		1.90.4
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE 
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


# GET ImGui-SFML 2.6
CPMAddPackage(
  NAME imgui-sfml
  URL "https://github.com/SFML/imgui-sfml/archive/refs/tags/v2.6.tar.gz"
  VERSION 2.6
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  OPTIONS        "IMGUI_SFML_FIND_SFML OFF"
)

# GET spdlog v1.13
CPMAddPackage(
  NAME spdlog
  URL "https://github.com/gabime/spdlog/archive/refs/tags/v1.13.0.tar.gz"
  VERSION 1.13.0
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
) 