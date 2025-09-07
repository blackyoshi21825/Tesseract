#!/bin/bash

# Static analysis and linting script for Tesseract
set -e

echo "🔍 Running static analysis on Tesseract..."

# Check if cppcheck is available
if ! command -v cppcheck &> /dev/null; then
    echo "Installing cppcheck..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install cppcheck
    elif command -v apt-get &> /dev/null; then
        sudo apt-get install -y cppcheck
    else
        echo "Please install cppcheck manually"
        exit 1
    fi
fi

# Run cppcheck on source files
echo "Running cppcheck..."
cppcheck --enable=all --inconclusive --std=c99 \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    -I include/ \
    src/ packages/

# Check for common issues with grep
echo "Checking for common issues..."

# Check for TODO/FIXME comments
echo "📝 TODO/FIXME comments:"
grep -rn "TODO\|FIXME" src/ include/ packages/ || echo "None found"

# Check for potential memory leaks (malloc without free)
echo "🔍 Potential memory issues:"
grep -rn "malloc\|calloc\|realloc" src/ packages/ | while read -r line; do
    file=$(echo "$line" | cut -d: -f1)
    if ! grep -q "free" "$file"; then
        echo "⚠️  Potential memory leak in: $line"
    fi
done

echo "✅ Static analysis completed!"