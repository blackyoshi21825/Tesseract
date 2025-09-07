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

### Code Quality
- `./dev-tools/format.sh` - Format code with clang-format
- `./dev-tools/lint.sh` - Run static analysis
- `./dev-tools/profile.sh` - Performance profiling

### Development Server
- `./dev-tools/dev-server.sh` - Auto-rebuild on file changes

## Code Style

- Use 4 spaces for indentation
- Follow existing naming conventions
- Add comments for complex logic
- Update tests when adding features
- Run `./dev-tools/format.sh` before committing

## Adding New Features

1. Create feature branch: `git checkout -b feature/new-feature`
2. Implement feature with tests
3. Run full test suite: `./dev-tools/run-tests.sh`
4. Format code: `./dev-tools/format.sh`
5. Run static analysis: `./dev-tools/lint.sh`
6. Update documentation
7. Submit pull request

## Testing Guidelines

### Unit Tests
- Place in `tests/unit/`
- Test individual functions and components
- Use descriptive test names

### Integration Tests
- Place in `tests/integration/`
- Test feature interactions
- Include real-world scenarios

### Benchmarks
- Place in `benchmarks/`
- Focus on performance-critical code
- Include baseline comparisons

## Debugging

Use VS Code with the provided configuration for debugging:
- Set breakpoints in source files
- Use "Debug Tesseract" launch configuration
- Inspect variables and call stack

For command-line debugging:
```bash
make debug
gdb ./tesser
```

## Performance Testing

Run benchmarks to ensure performance:
```bash
./dev-tools/benchmark.sh
```

Monitor memory usage:
```bash
./dev-tools/memcheck.sh
```

Profile performance:
```bash
./dev-tools/profile.sh
```

## Docker Development

Use Docker for consistent development environment:
```bash
cd docker
docker-compose -f docker-compose.dev.yml up -d
docker exec -it tesseract-dev bash
```

## Submitting Changes

1. Ensure all tests pass
2. Format code consistently
3. Update documentation
4. Write clear commit messages
5. Submit pull request with description

## Getting Help

- Check existing documentation
- Look at example code
- Ask questions in issues
- Join development discussions