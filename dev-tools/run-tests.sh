#!/bin/bash
set -e

echo "🧪 Running Tesseract tests..."

# Ensure tesser is built
if [ ! -f "./tesser" ]; then
    echo "Building Tesseract first..."
    ./dev-tools/build.sh
fi

# Run unit tests
echo "Running unit tests..."
for test_file in tests/unit/*.tesseract; do
    if [ -f "$test_file" ]; then
        echo "Testing: $test_file"
        ./tesser "$test_file"
    fi
done

# Run integration tests
echo "Running integration tests..."
for test_file in tests/integration/*.tesseract; do
    if [ -f "$test_file" ]; then
        echo "Testing: $test_file"
        ./tesser "$test_file"
    fi
done

echo "✅ All tests completed!"