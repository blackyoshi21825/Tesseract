# Example Packages

This directory contains example packages that demonstrate how to create custom packages for Tesseract.

## hello_world.c

A simple example package that demonstrates:
- Basic function structure
- String parameter handling
- Number parameter handling
- Return values
- Package initialization

### Functions:
- `hello()` - Prints a simple hello message
- `greet(name)` - Greets a specific person by name
- `add_numbers(a, b)` - Adds two numbers together

### Usage Example:

```tesseract
# Import the example package
import$ "packages/examples/hello_world.c"

# Use the functions
hello()                    # prints "Hello from Tesseract package!"
greet("Alice")            # prints "Hello, Alice!"
let$sum := add_numbers(5, 3)
::print sum               # prints 8
```

## Creating Your Own Package

Follow this template structure:

```c
#include "../core/package_loader.h"
#include "../../include/ast.h"

// Your function implementations
ASTNode *your_function(ASTNode **args, int arg_count) {
    // Validate arguments
    if (arg_count != expected_count) {
        return ast_new_number(0); // or appropriate error value
    }
    
    // Check argument types
    if (args[0]->type != NODE_NUMBER) {
        return ast_new_number(0);
    }
    
    // Your logic here
    double result = /* your calculation */;
    
    // Return result
    return ast_new_number(result);
    // or return ast_new_string("result");
}

// Package initialization
void init_your_package() {
    register_package_function("your_function", your_function);
}
```

### Tips:
1. Always validate argument count and types
2. Use appropriate return types (ast_new_number, ast_new_string)
3. Handle edge cases gracefully
4. Register all functions in the init function
5. Follow consistent naming conventions