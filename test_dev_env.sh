#!/bin/bash

echo "🧪 Testing Tesseract Developer Environment"

# Test 1: Check if directories exist
echo "✓ Testing directory structure..."
for dir in "dev-tools" "tests/unit" "tests/integration" "benchmarks" "examples" "docker"; do
    if [ -d "$dir" ]; then
        echo "  ✓ $dir exists"
    else
        echo "  ✗ $dir missing"
    fi
done

# Test 2: Check if scripts exist and are executable
echo "✓ Testing development scripts..."
for script in "dev-tools/build.sh" "dev-tools/run-tests.sh" "dev-tools/format.sh" "dev-tools/lint.sh" "dev-tools/profile.sh" "dev-tools/memcheck.sh"; do
    if [ -x "$script" ]; then
        echo "  ✓ $script is executable"
    else
        echo "  ✗ $script not executable or missing"
    fi
done

# Test 3: Check if configuration files exist
echo "✓ Testing configuration files..."
for file in ".editorconfig" ".clang-format" "CONTRIBUTING.md" "docker/Dockerfile.dev" "docker/docker-compose.dev.yml"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file exists"
    else
        echo "  ✗ $file missing"
    fi
done

# Test 4: Check if example files exist
echo "✓ Testing example files..."
for file in "examples/hello_world.tesseract" "examples/data_structures.tesseract"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file exists"
    else
        echo "  ✗ $file missing"
    fi
done

# Test 5: Test basic make functionality
echo "✓ Testing build system..."
if make --version > /dev/null 2>&1; then
    echo "  ✓ make is available"
else
    echo "  ✗ make not found"
fi

# Test 6: Check compiler availability
echo "✓ Testing compilers..."
if command -v gcc > /dev/null 2>&1; then
    echo "  ✓ gcc available: $(gcc --version | head -1)"
else
    echo "  ✗ gcc not found"
fi

if command -v clang > /dev/null 2>&1; then
    echo "  ✓ clang available: $(clang --version | head -1)"
else
    echo "  ✗ clang not found"
fi

# Test 7: Test Docker setup
echo "✓ Testing Docker configuration..."
if command -v docker > /dev/null 2>&1; then
    echo "  ✓ Docker is available"
    if docker info > /dev/null 2>&1; then
        echo "  ✓ Docker daemon is running"
    else
        echo "  ⚠ Docker daemon not running"
    fi
else
    echo "  ⚠ Docker not installed (optional)"
fi

echo ""
echo "🎉 Developer environment test completed!"
echo "📝 To get started:"
echo "   1. Fix any compilation issues in packages/stdlib/"
echo "   2. Run: make clean && make"
echo "   3. Test with: ./tesser examples/hello_world.tesseract"