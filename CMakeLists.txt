cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project("SpaceCheckers" VERSION 1.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SFML_STATIC_LIBRARIES TRUE)

# Absolute path where you installed SFML
set(CUSTOM_SFML_HOME "D:/work_mine/CPP/SFML-2.6.0")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")
set(SFML_DIR ${CUSTOM_SFML_HOME}/lib/cmake/SFML)
find_package(SFML REQUIRED COMPONENTS "graphics" "window" "system")

if (WIN32)
    add_executable(SpaceCheckers WIN32 ${SOURCES} ${CMAKE_SOURCE_DIR}/resources/win-icon.rc)
    target_link_libraries(SpaceCheckers PRIVATE sfml-main sfml-graphics sfml-window sfml-system)
else()
    add_executable(SpaceCheckers ${SOURCES})
    target_link_libraries(SpaceCheckers PRIVATE sfml-graphics sfml-window sfml-system)
endif()

# include SFML headers
include_directories(${CUSTOM_SFML_HOME}/include)

# show warnings (depending on C++ compiler)
if (MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
endif()

# COPY resources folder to destination
add_custom_command(
        TARGET SpaceCheckers POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:SpaceCheckers>/resources
)
