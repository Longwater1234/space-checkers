cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  "SpaceCheckers"
  VERSION 1.0.12
  HOMEPAGE_URL "https://github.com/Longwater1234/space-checkers")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# On macOS using Frameworks, NOT static (dylib)
if(NOT APPLE)
  set(SFML_STATIC_LIBRARIES TRUE)
endif()

# write app version into Header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/proto_schema/AppVersion.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/src/AppVersion.hpp)

# ==== UPDATE ME HERE ===========
# Absolute path where you installed SFML (Required on Windows)
set(SFML_HOME "C:/SFML/SFML-2.6.1")
set(SFML_DIR ${SFML_HOME}/lib/cmake/SFML)
# ==========================

# Collect all sources
file(GLOB_RECURSE GAME_SRC "src/*.cpp" "src/*.hpp")
find_package(SFML 2.6 REQUIRED COMPONENTS "graphics" "window" "system")

# download extra libs
add_subdirectory(dependencies)

if(WIN32)
  add_executable(SpaceCheckers WIN32 ${GAME_SRC} ${CMAKE_SOURCE_DIR}/resources/win-icon.rc)
  target_link_libraries(SpaceCheckers PRIVATE sfml-main)
elseif(APPLE)
  include(${CMAKE_SOURCE_DIR}/cmake/macbundle.cmake)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  target_link_libraries(SpaceCheckers PRIVATE ${FOUNDATION_FRAMEWORK})
else()
  add_executable(SpaceCheckers ${GAME_SRC})
endif()

# link all required libraries
target_link_libraries(
  SpaceCheckers
  PRIVATE sfml-graphics
          sfml-window
          sfml-system
          ImGui-SFML
          ixwebsocket
          spdlog::spdlog
          cpr::cpr
          simdjson::simdjson
          protobuf::libprotobuf)

# include SFML headers
target_include_directories(SpaceCheckers PUBLIC ${SFML_HOME}/include)

# show warnings (depending on C++ compiler)
if(MSVC)
  target_compile_options(SpaceCheckers PRIVATE /W4 /sdl)
else()
  target_compile_options(SpaceCheckers PRIVATE -Wall -Werror=constant-conversion -Werror=fortify-source)
endif()

# COPY resources folder to destination
if(NOT APPLE)
  add_custom_command(
    TARGET SpaceCheckers
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:SpaceCheckers>/resources)
endif()
