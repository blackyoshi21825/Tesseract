#!/bin/bash

# Tesseract Developer Environment Setup Script
# This script sets up a complete development environment for Tesseract

set -e

echo "🚀 Setting up Tesseract Developer Environment..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running on macOS or Linux
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
    PACKAGE_MANAGER="brew"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    if command -v apt-get &> /dev/null; then
        PACKAGE_MANAGER="apt"
    elif command -v dnf &> /dev/null; then
        PACKAGE_MANAGER="dnf"
    elif command -v yum &> /dev/null; then
        PACKAGE_MANAGER="yum"
    else
        print_error "Unsupported Linux distribution"
        exit 1
    fi
else
    print_error "Unsupported operating system: $OSTYPE"
    exit 1
fi

print_status "Detected OS: $OS with package manager: $PACKAGE_MANAGER"

# Install dependencies
install_dependencies() {
    print_status "Installing development dependencies..."
    
    case $PACKAGE_MANAGER in
        "brew")
            brew update
            brew install curl gcc clang make cmake ninja ccache gdb lldb
            ;;
        "apt")
            sudo apt-get update
            sudo apt-get install -y libcurl4-openssl-dev build-essential gcc clang make cmake ninja-build ccache gdb valgrind
            ;;
        "dnf")
            sudo dnf install -y libcurl-devel gcc clang make cmake ninja-build ccache gdb valgrind
            ;;
        "yum")
            sudo yum install -y libcurl-devel gcc clang make cmake ninja-build ccache gdb valgrind
            ;;
    esac
    
    print_success "Dependencies installed successfully"
}

# Setup development tools
setup_dev_tools() {
    print_status "Setting up development tools..."
    
    # Create development directories
    mkdir -p dev-tools
    mkdir -p tests/unit
    mkdir -p tests/integration
    mkdir -p benchmarks
    mkdir -p examples
    
    # Setup ccache if available
    if command -v ccache &> /dev/null; then
        ccache --set-config=max_size=2G
        ccache --set-config=compression=true
        print_success "ccache configured with 2GB cache size"
    fi
    
    print_success "Development directories created"
}

# Setup VS Code configuration
setup_vscode() {
    print_status "Setting up VS Code configuration..."
    
    mkdir -p .vscode
    
    # Create tasks.json for build tasks
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Tesseract",
            "type": "shell",
            "command": "make",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Build Debug",
            "type": "shell",
            "command": "make",
            "args": ["debug"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "./dev-tools/run-tests.sh",
            "group": "test",
            "dependsOn": "Build Tesseract"
        }
    ]
}
EOF
    
    # Create launch.json for debugging
    cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Tesseract",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/tesser",
            "args": ["test.tesseract"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build Debug"
        }
    ]
}
EOF
    
    # Create settings.json
    cat > .vscode/settings.json << 'EOF'
{
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/include"
    ],
    "C_Cpp.default.defines": [],
    "C_Cpp.default.compilerPath": "/usr/bin/gcc",
    "C_Cpp.default.cStandard": "c99",
    "C_Cpp.default.intelliSenseMode": "gcc-x64",
    "files.associations": {
        "*.tesseract": "plaintext",
        "*.h": "c",
        "*.c": "c"
    },
    "editor.tabSize": 4,
    "editor.insertSpaces": true
}
EOF
    
    print_success "VS Code configuration created"
}

# Create development scripts
create_dev_scripts() {
    print_status "Creating development scripts..."
    
    # Build script
    cat > dev-tools/build.sh << 'EOF'
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
EOF
    
    # Test runner script
    cat > dev-tools/run-tests.sh << 'EOF'
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
EOF
    
    # Development server script
    cat > dev-tools/dev-server.sh << 'EOF'
#!/bin/bash

echo "🔄 Starting Tesseract development server..."
echo "Watching for file changes..."

# Install fswatch if not available
if ! command -v fswatch &> /dev/null; then
    echo "Installing fswatch for file watching..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install fswatch
    else
        echo "Please install fswatch manually"
        exit 1
    fi
fi

# Watch for changes and rebuild
fswatch -o src/ include/ | while read f; do
    echo "Files changed, rebuilding..."
    ./dev-tools/build.sh
    echo "Rebuild complete. Watching for changes..."
done
EOF
    
    # Benchmark script
    cat > dev-tools/benchmark.sh << 'EOF'
#!/bin/bash
set -e

echo "📊 Running Tesseract benchmarks..."

# Ensure tesser is built in release mode
make clean
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Run benchmarks
for benchmark in benchmarks/*.tesseract; do
    if [ -f "$benchmark" ]; then
        echo "Benchmarking: $benchmark"
        time ./tesser "$benchmark"
        echo "---"
    fi
done

echo "✅ Benchmarks completed!"
EOF
    
    # Memory check script
    cat > dev-tools/memcheck.sh << 'EOF'
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
EOF
    
    # Make scripts executable
    chmod +x dev-tools/*.sh
    
    print_success "Development scripts created"
}

# Create sample test files
create_sample_tests() {
    print_status "Creating sample test files..."
    
    # Unit test example
    cat > tests/unit/basic_operations.tesseract << 'EOF'
# Basic operations test
let$a := 10;
let$b := 5;

# Test arithmetic
let$sum := a + b;
let$diff := a - b;
let$prod := a * b;
let$quot := a / b;

::print "Sum: @s" (sum);
::print "Difference: @s" (diff);
::print "Product: @s" (prod);
::print "Quotient: @s" (quot);

# Test comparisons
::print "a > b: @s" (a > b);
::print "a == b: @s" (a == b);
EOF
    
    # Integration test example
    cat > tests/integration/data_structures.tesseract << 'EOF'
# Data structures integration test
let$list := [1, 2, 3, 4, 5];
let$stack := <stack>;
let$queue := <queue>;

# Test list operations
::print "List length: @s" (::len(list));
::append(list, 6);
::print "After append: @s" (::len(list));

# Test stack operations
::push(stack, 10);
::push(stack, 20);
::print "Stack size: @s" (::size(stack));
::print "Popped: @s" (::pop(stack));

# Test queue operations
::enqueue(queue, 100);
::enqueue(queue, 200);
::print "Queue size: @s" (::qsize(queue));
::print "Dequeued: @s" (::dequeue(queue));
EOF
    
    # Benchmark example
    cat > benchmarks/fibonacci.tesseract << 'EOF'
# Fibonacci benchmark
func$fib(n) => {
    if$ n <= 1 {
        n
    } else {
        fib(n - 1) + fib(n - 2)
    }
}

let$result := fib(30);
::print "Fibonacci(30) = @s" (result);
EOF
    
    print_success "Sample test files created"
}

# Create documentation for developers
create_dev_docs() {
    print_status "Creating developer documentation..."
    
    cat > CONTRIBUTING.md << 'EOF'
# Contributing to Tesseract

## Development Setup

1. Run the setup script:
   ```bash
   ./dev-setup.sh
   ```

2. Build the project:
   ```bash
   ./dev-tools/build.sh
   ```

3. Run tests:
   ```bash
   ./dev-tools/run-tests.sh
   ```

## Development Workflow

### Building
- `./dev-tools/build.sh` - Build with optimizations
- `make debug` - Build with debug symbols
- `make clean` - Clean build artifacts

### Testing
- `./dev-tools/run-tests.sh` - Run all tests
- `./dev-tools/memcheck.sh` - Run memory checks
- `./dev-tools/benchmark.sh` - Run performance benchmarks

### Development Server
- `./dev-tools/dev-server.sh` - Auto-rebuild on file changes

## Code Style

- Use 4 spaces for indentation
- Follow existing naming conventions
- Add comments for complex logic
- Update tests when adding features

## Adding New Features

1. Create feature branch: `git checkout -b feature/new-feature`
2. Implement feature with tests
3. Run full test suite
4. Update documentation
5. Submit pull request

## Debugging

Use VS Code with the provided configuration for debugging:
- Set breakpoints in source files
- Use "Debug Tesseract" launch configuration
- Inspect variables and call stack

## Performance Testing

Run benchmarks to ensure performance:
```bash
./dev-tools/benchmark.sh
```

Monitor memory usage:
```bash
./dev-tools/memcheck.sh
```
EOF
    
    print_success "Developer documentation created"
}

# Main setup function
main() {
    print_status "Starting Tesseract developer environment setup..."
    
    install_dependencies
    setup_dev_tools
    setup_vscode
    create_dev_scripts
    create_sample_tests
    create_dev_docs
    
    print_success "🎉 Developer environment setup completed!"
    print_status "Next steps:"
    echo "  1. Build the project: ./dev-tools/build.sh"
    echo "  2. Run tests: ./dev-tools/run-tests.sh"
    echo "  3. Open in VS Code for development"
    echo "  4. Read CONTRIBUTING.md for development guidelines"
}

# Run main function
main "$@"