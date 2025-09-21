# Tesseract Programming Language

A modern, interpreted programming language with dynamic typing, advanced data structures, and powerful features for rapid development.

## 🚀 Quick Start

### For Users
```bash
# Clone the repository
git clone <repository-url>
cd Tesseract

# Build Tesseract
make

# Run a program
./tesser examples/hello_world.tesseract
```

### For Developers
```bash
# Set up development environment
./dev-setup.sh

# Build with development tools
./dev-tools/build.sh

# Run tests
./dev-tools/run-tests.sh
```

## 🐳 Docker Development

```bash
# Build and run development container
cd docker
docker-compose -f docker-compose.dev.yml up -d

# Enter the container
docker exec -it tesseract-dev bash
```

## 📚 Documentation

- [Language Documentation](docs/README.md) - Complete language reference
- [Build Instructions](docs/BUILD.md) - Detailed build instructions
- [Contributing Guide](CONTRIBUTING.md) - How to contribute to the project

## 🛠️ Development Tools

- `./dev-tools/build.sh` - Build the project
- `./dev-tools/run-tests.sh` - Run test suite
- `./dev-tools/format.sh` - Format source code
- `./dev-tools/lint.sh` - Run static analysis
- `./dev-tools/profile.sh` - Performance profiling
- `./dev-tools/memcheck.sh` - Memory leak detection

## 🎯 Key Features

- **Dynamic Typing** - Flexible variable types
- **Advanced Data Structures** - Lists, stacks, queues, trees, graphs, sets
- **Control Flow** - Loops, conditionals, pattern matching
- **Functions & Classes** - Object-oriented programming support
- **Exception Handling** - Try/catch/finally blocks
- **Regular Expressions** - Built-in regex support
- **HTTP Requests** - Web API integration
- **File I/O** - File handling capabilities
- **Lambda Expressions** - Functional programming features

## 📖 Example

```tesseract
# Variables and functions
let$name := "World";
func$greet(person) => {
    ::print "Hello, @s!" (person);
}

greet(name);

# Data structures
let$numbers := [1, 2, 3, 4, 5];
let$stack := <stack>;
::push(stack, "item");

# Control flow
loop$i := 1 => 5 {
    ::print "Count: @s" (i);
}
```

## 🏗️ Project Structure

```
Tesseract/
├── src/           # Core interpreter source code
├── include/       # Header files
├── packages/      # Standard library and package system
├── docs/          # Documentation
├── tests/         # Test files
├── examples/      # Example programs
├── dev-tools/     # Development scripts
├── docker/        # Docker configuration
└── benchmarks/    # Performance benchmarks
```

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🔧 Requirements

- GCC or Clang compiler
- Make or CMake
- libcurl development package
- Optional: Docker for containerized development

## 🌟 Getting Help

- Check the [documentation](docs/README.md)
- Look at [examples](examples/)
- Open an issue for bugs or feature requests