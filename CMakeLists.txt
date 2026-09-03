cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  "SpaceCheckers"
  VERSION 1.0.17
  HOMEPAGE_URL "https://github.com/Longwater1234/space-checkers")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Enable unit tests
option(ENABLE_GAME_TESTS "Enable unit tests" OFF)

# Link SFML statically
set(SFML_STATIC_LIBRARIES TRUE)

# write app version into Header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/protos/AppVersion.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/src/AppVersion.hpp @ONLY) 

# download extra libs
add_subdirectory(dependencies) 

# Collect all sources
file(GLOB_RECURSE GAME_SRC "src/*.cpp" "src/*.hpp")

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

# link all common libraries
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


# compiler flags (depending on C++ compiler)
if(MSVC)
  target_compile_options(SpaceCheckers PRIVATE /W4 /sdl /utf-8 $<$<CONFIG:Release>:/GL>)
  target_link_options(SpaceCheckers PRIVATE $<$<CONFIG:Release>:/LTCG>)
else()
  target_compile_options(SpaceCheckers PRIVATE -Wall -Werror=constant-conversion 
                          -Werror=fortify-source $<$<CONFIG:Release>:-flto=thin>)
endif()
 
# COPY resources folder to destination
if(NOT APPLE)
  add_custom_command(
    TARGET SpaceCheckers
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:SpaceCheckers>/resources)
endif()

if(ENABLE_GAME_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()