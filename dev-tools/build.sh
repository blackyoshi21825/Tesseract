#!/bin/bash
set -e

echo "🔨 Building Tesseract..."

# Check if ccache is available
if command -v ccache &> /dev/null; then
    export CC="ccache gcc"
    echo "Using ccache for faster builds"
fi

# Detect number of cores
if [[ "$OSTYPE" == "darwin"* ]]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=$(nproc)
fi

echo "Building with $CORES parallel jobs..."

# Generate precompiled header first
make pch

# Build with parallel jobs
make -j$CORES

echo "✅ Build completed successfully!"