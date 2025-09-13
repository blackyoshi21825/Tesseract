# 🍔 Tesseract Burger - Build Utility for Rapid Generation and Execution of Resources

The **Burger** package is a comprehensive developer toolkit for Tesseract that provides essential development utilities organized around the metaphor of building a delicious burger. Each "ingredient" represents a different category of development tools.

## 🍞 BUN - Project Scaffolding & Finalization

The bun holds everything together - it handles project setup and completion.

### Functions:
- `bun_scaffold(project_name, project_type)` - Creates a new project structure

**Example:**
```tesseract
bun_scaffold("MyApp", "c")  // Creates a C project with src/, include/, and main.c
```

## 🥬 LETTUCE - Logging & Debugging Utilities

Fresh logging capabilities to keep your development crisp and clear.

### Functions:
- `lettuce_log(level, message)` - Logs messages with different severity levels

**Example:**
```tesseract
lettuce_log("INFO", "Application started")
lettuce_log("ERROR", "Database connection failed")
lettuce_log("DEBUG", "Variable x = 42")
```

**Supported levels:** INFO (green), WARN (yellow), ERROR (red), DEBUG (cyan)

## 🍅 TOMATO - Testing Utilities

Juicy testing tools to ensure your code is ripe and ready.

### Functions:
- `tomato_assert(condition, test_name)` - Performs assertions with visual feedback

**Example:**
```tesseract
tomato_assert(1, "Basic functionality test")        // ✅ PASS
tomato_assert(0, "Expected failure test")           // ❌ FAIL
tomato_assert(5 > 3, "Math comparison test")        // ✅ PASS
```

## 🧀 CHEESE - Code Generation & Templates

Smooth code generation that melts perfectly into your project.

### Functions:
- `cheese_template(template_type, name, filename)` - Generates code templates

**Example:**
```tesseract
cheese_template("header", "MYHEADER", "myheader.h")
cheese_template("class", "MyClass", "myclass.h")
```

**Supported templates:** header, class

## 🥩 MEAT - Main Build & Compilation

The hearty core of your development process - building and compiling code.

### Functions:
- `meat_compile(source_file, output_file)` - Compiles source code

**Example:**
```tesseract
meat_compile("main.c", "main.exe")  // Compiles main.c to main.exe using gcc
```

## 🧅 ONION - Optimization & Performance Tools

Layers of performance analysis that might make you cry (tears of joy).

### Functions:
- `onion_profile(function_name)` - Profiles function execution time

**Example:**
```tesseract
onion_profile("my_function")  // Returns execution time in milliseconds
```

## 🥒 PICKLE - Package & Dependency Management

Preserved package management that keeps your dependencies fresh.

### Functions:
- `pickle_deps(action)` - Manages project dependencies

**Example:**
```tesseract
pickle_deps("list")    // Lists installed packages
pickle_deps("update")  // Updates package registry
```

## 🍯 SAUCE - Source Control & Versioning

The sweet finishing touch for version control operations.

### Functions:
- `sauce_version(action)` - Handles version control operations

**Example:**
```tesseract
sauce_version("status")  // Shows repository status
sauce_version("tag")     // Creates version tag
```

## 🍔 BURGER - Complete Build Pipeline

The ultimate function that combines all ingredients into a complete development workflow.

### Functions:
- `burger_build(project_name)` - Executes a complete build pipeline

**Example:**
```tesseract
burger_build("MyProject")  // Runs through all 7 steps of the build process
```

## Usage

1. **Load the burger package** (automatically loaded in Tesseract)
2. **Use individual functions** for specific tasks
3. **Run the complete pipeline** with `burger_build()` for full automation

## Demo

Run the included demo script:
```bash
tesseract burger_demo.tesseract
```

This will showcase all burger functionality with a complete development workflow simulation.

## Why "Burger"?

Each ingredient serves a specific purpose in the development process:
- **Bun**: Holds everything together (project structure)
- **Lettuce**: Keeps things fresh (logging/debugging)
- **Tomato**: Adds flavor and validation (testing)
- **Cheese**: Smooth and binding (code generation)
- **Meat**: The substantial core (compilation)
- **Onion**: Complex layers (optimization)
- **Pickle**: Preserved components (dependencies)
- **Sauce**: The finishing touch (version control)

Together, they create a complete, satisfying development experience! 🍔✨