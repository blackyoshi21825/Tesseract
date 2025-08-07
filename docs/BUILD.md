# Building Tesseract for Maximum Performance

This document provides instructions for building Tesseract with optimal compilation speed and runtime performance.

## Prerequisites

- GCC/Clang compiler
- Make or CMake build system
- libcurl development package
- Ninja build system (optional, but recommended for faster builds)

## Optimizing Compilation Speed

### Windows

1. **Install Ninja build system** (optional but recommended):
   - Download from https://github.com/ninja-build/ninja/releases
   - Add to your PATH

2. **Build with optimized settings**:
   ```batch
   build_win.bat
   ```
   This script automatically:
   - Detects the number of CPU cores
   - Uses Ninja if available
   - Enables link-time optimization
   - Uses precompiled headers

### Linux/macOS

1. **Generate precompiled header**:
   ```bash
   make pch
   ```

2. **Build with parallel compilation**:
   ```bash
   make -j$(nproc)  # Linux
   make -j$(sysctl -n hw.ncpu)  # macOS
   ```

3. **Use Clang if available** (often faster than GCC):
   ```bash
   CC=clang make
   ```

## Advanced Optimization Tips

1. **Use ccache** to speed up repeated builds:
   ```bash
   # Install ccache
   sudo apt install ccache  # Ubuntu/Debian
   brew install ccache      # macOS

   # Use with make
   CC="ccache gcc" make
   ```

2. **Use distcc** for distributed compilation across multiple machines:
   ```bash
   # Install distcc
   sudo apt install distcc  # Ubuntu/Debian
   brew install distcc      # macOS

   # Use with make
   CC="distcc gcc" make -j16
   ```

3. **Memory usage optimization**:
   If you have limited RAM, reduce the number of parallel jobs:
   ```bash
   make -j2  # Use fewer parallel jobs
   ```

4. **Temporary disable antivirus scanning** of build directories on Windows
   for faster compilation.

## Troubleshooting

- If you encounter "out of memory" errors, reduce the number of parallel jobs
- If precompiled headers fail, try building without them: `make TARGET`