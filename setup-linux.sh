#!/bin/bash
# Quick Linux setup for Tesseract

# Install dependencies
if command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y build-essential libcurl4-openssl-dev make gcc clang
elif command -v dnf &> /dev/null; then
    sudo dnf install -y gcc clang make libcurl-devel
elif command -v yum &> /dev/null; then
    sudo yum install -y gcc clang make libcurl-devel
fi

# Build
make clean && make

echo "✅ Ready! Test with: ./tesser examples/hello_world.tesseract"