#!/bin/bash

# Script to convert C++ files to PNG images for presentations using carbon-now-cli
#
# Prerequisites:
#   1. Node.js and npm must be installed
#   2. Install carbon-now-cli: npm install -g carbon-now-cli
#   3. Install Playwright browsers: npx playwright install
#
# Usage:
#   ./generate_slides.sh

set -e

# Delete existing images in slides folder
echo "Cleaning slides directory..."
rm -rf slides/*.png

# Convert all .cpp files in adc-examples to PNG images
echo "Converting .cpp files to images..."
for file in adc-examples/*.cpp; do
    carbon-now "$file" --save-to slides --settings '{"windowControls":false,"dropShadow":false,"paddingVertical":"0px","paddingHorizontal":"0px","theme":"blackboard"}'
done

echo "Done! Images saved to slides/"
