#!/bin/bash

# target directory
SRC_DIR="src"
TEST_DIR="tests"

clang-format $(find $SRC_DIR $TEST_DIR -type f -name "*.cpp" -o -name "*.hpp") -i --verbose
echo "Formatting completed."