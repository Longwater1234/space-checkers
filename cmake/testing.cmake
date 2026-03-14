# Add the test executable
add_executable(SpaceCheckersTests
    tests/PlayerTests.cpp
    tests/PieceTests.cpp
    # Include more test files as needed
)

# Add src directory so test files can find headers
target_include_directories(SpaceCheckersTests PRIVATE ${CMAKE_SOURCE_DIR}/src ${SFML_HOME}/include)

# Include ResourcePath implementation (platform-specific)
if(APPLE)
    target_sources(SpaceCheckersTests PRIVATE ${CMAKE_SOURCE_DIR}/src/utils/ResourcePath.mm)
else()
    target_sources(SpaceCheckersTests PRIVATE ${CMAKE_SOURCE_DIR}/src/utils/ResourcePath.cpp)
endif()

# Link Google Test and SFML to the test executable
target_link_libraries(SpaceCheckersTests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    sfml-graphics
    sfml-window
    sfml-system
)

# Automatically discover and register tests
include(GoogleTest)  # Ensure you include the GoogleTest module
gtest_discover_tests(SpaceCheckersTests)
