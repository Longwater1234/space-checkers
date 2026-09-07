# Add the test executable
add_executable(SpaceCheckersTests
    ${CMAKE_SOURCE_DIR}/tests/PlayerTests.cpp
    ${CMAKE_SOURCE_DIR}/tests/PieceTests.cpp
    ${CMAKE_SOURCE_DIR}/tests/GameManagerTests.cpp
    # Include more test files as needed
)

# include headers directories
target_include_directories(SpaceCheckersTests PRIVATE ${CMAKE_SOURCE_DIR}/src ${SFML_HOME}/include)

# add required src files
SET(TEST_SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/Player.cpp"
    "${CMAKE_SOURCE_DIR}/src/Piece.cpp"
    "${CMAKE_SOURCE_DIR}/src/Cell.cpp"
    "${CMAKE_SOURCE_DIR}/src/managers/GameManager.cpp"
)

if(APPLE)
    target_sources(SpaceCheckersTests PRIVATE ${TEST_SRC_FILES} ${CMAKE_SOURCE_DIR}/src/utils/ResourcePath.mm)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    target_link_libraries(SpaceCheckersTests PRIVATE ${FOUNDATION_FRAMEWORK})
else()
    target_sources(SpaceCheckersTests PRIVATE ${TEST_SRC_FILES} ${CMAKE_SOURCE_DIR}/src/utils/ResourcePath.cpp)
endif()

# Link Google Test and SFML to the test executable
target_link_libraries(SpaceCheckersTests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    sfml-graphics
    sfml-window
    sfml-system
    spdlog::spdlog
)

# Automatically discover and register tests
include(GoogleTest)
gtest_discover_tests(SpaceCheckersTests)
