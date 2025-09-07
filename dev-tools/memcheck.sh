#!/bin/bash
set -e

echo "🔍 Running memory checks..."

# Build debug version
make clean
make debug

# Check if valgrind is available (Linux only)
if command -v valgrind &> /dev/null; then
    echo "Running valgrind memory check..."
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./tesser test.tesseract
else
    echo "Valgrind not available. Running with debug build..."
    ./tesser test.tesseract
fi

echo "✅ Memory check completed!"