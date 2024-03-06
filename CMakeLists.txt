cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project("SpaceCheckers" VERSION 1.0 HOMEPAGE_URL "https://github.com/Longwater1234/space-checkers")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(IMGUI_SFML_FIND_SFML OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(WIN32)
  set(SFML_STATIC_LIBRARIES TRUE)
endif()

# Absolute path where you installed SFML
set(SFML_HOME "D:/work_mine/CPP/SFML-2.6.1")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp" "src/*.mm")
set(SFML_DIR ${SFML_HOME}/lib/cmake/SFML)

find_package(SFML REQUIRED COMPONENTS "graphics" "window" "system")
find_package(OpenGL REQUIRED)

# download extra deps
add_subdirectory(dependencies)

if(WIN32)
  add_executable(SpaceCheckers WIN32 ${SOURCES} ${CMAKE_SOURCE_DIR}/resources/win-icon.rc)
  target_link_libraries(SpaceCheckers PRIVATE sfml-main)
elseif(APPLE)
  include(${CMAKE_SOURCE_DIR}/apple_cmake/MacBundle.cmake)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  target_link_libraries(SpaceCheckers PRIVATE ${FOUNDATION_FRAMEWORK})
else()
  add_executable(SpaceCheckers ${SOURCES} OpenGL:GL)
endif()

# link common libs
target_link_libraries(SpaceCheckers PRIVATE sfml-graphics sfml-window sfml-system ImGui-SFML
                        ixwebsocket spdlog::spdlog)

# include SFML headers
include_directories(${SFML_HOME}/include)

# show warnings (depending on C++ compiler)
if(MSVC)
  target_compile_options(SpaceCheckers PRIVATE /W4)
else()
  target_compile_options(SpaceCheckers PRIVATE -Wall)
endif()

# COPY resources folder to destination
if(WIN32)
  add_custom_command(
    TARGET SpaceCheckers
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:SpaceCheckers>/resources)
endif()
