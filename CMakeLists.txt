cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project("space-checkers")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SFML_STATIC_LIBRARIES TRUE)

# Absolute path where you installed SFML
set(CUSTOM_SFML_HOME "D:/work_mine/CPP/SFML-2.6.0")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")
set(SFML_DIR ${CUSTOM_SFML_HOME}/lib/cmake/SFML)
find_package(SFML REQUIRED COMPONENTS "graphics" "window" "system")

if(WIN32)
	add_executable(space-checkers WIN32 ${SOURCES})
else()
    add_executable(space-checkers ${SOURCES})
endif()

# include SFML headers and libs
target_link_libraries(space-checkers PRIVATE sfml-main sfml-graphics sfml-window sfml-system)
include_directories(${CUSTOM_SFML_HOME}/include)

# show warnings (depending on CXX compiler)
target_compile_options(space-checkers PRIVATE
		$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall>
		$<$<CXX_COMPILER_ID:MSVC>: /W4>)

# COPY resources folder to destination
add_custom_command(
	TARGET space-checkers POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:space-checkers>/resources
)
