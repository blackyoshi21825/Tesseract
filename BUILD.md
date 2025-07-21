# Optimized Build Instructions

## Using Make (Original Method)
```bash
# Standard build (with optimizations)
make

# Debug build
make debug

# Clean build
make clean && make
```

## Using CMake (Recommended for faster builds)
### On Windows
```bash
# Run the build script
build_win.bat

# Or manually:
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build . --config Release -j 8
```

### On Linux/Mac
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release -j $(nproc)
```

## Build Options
- `-j N`: Set N to the number of CPU cores for parallel compilation
- `--config Release`: Build with optimizations
- `--config Debug`: Build with debug symbols