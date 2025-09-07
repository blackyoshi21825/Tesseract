#!/bin/bash

# Code formatting script for Tesseract
set -e

echo "🎨 Formatting Tesseract source code..."

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo "clang-format not found. Installing..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install clang-format
    elif command -v apt-get &> /dev/null; then
        sudo apt-get install -y clang-format
    else
        echo "Please install clang-format manually"
        exit 1
    fi
fi

# Format C source files
find src/ include/ packages/ -name "*.c" -o -name "*.h" | while read -r file; do
    echo "Formatting: $file"
    clang-format -i "$file"
done

echo "✅ Code formatting completed!"