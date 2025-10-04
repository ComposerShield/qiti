#!/usr/bin/env bash

# Clear and recreate build directory
rm -rf adc-examples/build
mkdir -p adc-examples/build

# Compile each cpp file in adc-examples
for file in adc-examples/*.cpp; do
    if [ -f "$file" ]; then
        filename=$(basename "$file" .cpp)
        echo "Compiling $filename..."
        clang++ -std=c++20 -c "$file" -o "adc-examples/build/${filename}.o"
    fi
done

echo "Done! Object files in adc-examples/build/"
