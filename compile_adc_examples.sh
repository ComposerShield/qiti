#!/usr/bin/env bash

# Clear and recreate build directory
rm -rf adc-examples/build
mkdir -p adc-examples/build

# GTest include path (if it exists from CMake build)
GTEST_INCLUDE=""
if [ -d "build/_deps/googletest-src/googletest/include" ]; then
    GTEST_INCLUDE="-I build/_deps/googletest-src/googletest/include"
fi

# Compile each cpp file in adc-examples
for file in adc-examples/*.cpp; do
    if [ -f "$file" ]; then
        filename=$(basename "$file" .cpp)
        echo "Compiling $filename..."
        clang++ -std=c++20 -I adc-examples $GTEST_INCLUDE -c "$file" -o "adc-examples/build/${filename}.o"
    fi
done

echo "Done! Object files in adc-examples/build/"
