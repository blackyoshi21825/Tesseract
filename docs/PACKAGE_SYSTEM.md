# Tesseract Package System

This directory contains the package system for the Tesseract programming language.

## Directory Structure

```
packages/
├── core/                   # Core package system files
│   ├── package_loader.c    # Package loading functionality
│   ├── package_loader.h    # Package loader header
│   ├── package_manager.c   # Package management system
│   ├── package_manager.h   # Package manager header
│   └── tpm.c              # Tesseract Package Manager CLI
├── stdlib/                 # Standard library packages
│   ├── math_utils.c       # Mathematical functions
│   └── string_utils.c     # String manipulation functions
├── examples/              # Example packages
└── registry.txt           # Package registry
```

## Package Types

### Core Packages
Located in `core/` - These are essential system packages that provide the foundation for the package system.

### Standard Library Packages
Located in `stdlib/` - These provide commonly used functionality like math operations, string manipulation, etc.

### Example Packages
Located in `examples/` - These demonstrate how to create custom packages.

## Available Packages

### Math Utils (`stdlib/math_utils.c`)
Mathematical functions and utilities:
- `factorial(n)` - Calculate factorial of n
- `power(base, exp)` - Calculate base^exp
- `sqrt(n)` - Square root of n
- `abs(n)` - Absolute value of n
- `sin(n)`, `cos(n)`, `tan(n)` - Trigonometric functions
- `is_prime(n)` - Check if n is prime
- `gcd(a, b)` - Greatest common divisor
- `lcm(a, b)` - Least common multiple

### String Utils (`stdlib/string_utils.c`)
String manipulation functions:
- `str_reverse(str)` - Reverse a string
- `str_contains(str, substr)` - Check if string contains substring
- `str_starts_with(str, prefix)` - Check if string starts with prefix
- `str_ends_with(str, suffix)` - Check if string ends with suffix
- `str_trim(str)` - Remove leading/trailing whitespace
- `str_repeat(str, count)` - Repeat string count times

## Using Packages

To use a package in your Tesseract code:

```tesseract
# Installed packaged are automatically imported, there is no need to add an import line at the top

# Use the functions
let$result := factorial(5)
::print result  # prints 120
```

Please note that built in Tesseract package functions do not require `::`. You may add it to your own custom packages.

## Creating Custom Packages

1. Create a new `.c` file in the appropriate directory
2. Include the package loader header: `#include "../package_loader.h"`
3. Implement your functions following the pattern:
   ```c
   ASTNode *your_function(ASTNode **args, int arg_count) {
       // Your implementation
       return ast_new_number(result);
   }
   ```
4. Create an initialization function:
   ```c
   void init_your_package() {
       register_package_function("your_function", your_function);
   }
   ```

## Package Manager (TPM)

The Tesseract Package Manager allows you to install, uninstall, and manage packages:

```bash
# Install a package
./tpm install package_name source_file.c

# List installed packages
./tpm list

# Uninstall a package
./tpm uninstall package_name

# Show help
./tpm help
```

## Registry Format

The `registry.txt` file tracks installed packages in the format:
```
package_name|version|path
```

Example:
```
math_utils|1.0.0|packages/stdlib/math_utils.c
string_utils|1.0.0|packages/stdlib/string_utils.c
```
