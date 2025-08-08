# Tesseract Programming Language

## Overview

Tesseract is a simple interpreted programming language featuring dynamic typing, arithmetic operations, control structures, functions, classes, and more. It supports modern programming concepts while maintaining simplicity.

**Key Features:**
- Dynamic typing
- Control structures (if/else, loops, break/continue)
- Functions and classes
- Lists and dictionaries
- Pattern matching
- File imports
- Boolean and bitwise operations
- File Handling
- Advanced Data Structures(Stacks, Queues, and Linked Lists)
- While Loops
- HTTP Web Requests
- Regular Expressions
- Exception Handling (try/catch/finally)
- Lambda Expressions with Closures
- String Interpolation
- Sets and Advanced Data Types
- Destructuring Assignment
- Enhanced Error Handling with Line Numbers
- Improved REPL with Error Recovery
- Generators and Iterators

## Getting Started

### Prerequisites
- libcurl development package

#### Installing libcurl
- Ubuntu/Debian: `sudo apt-get install libcurl4-openssl-dev`
- Fedora/RHEL: `sudo dnf install libcurl-devel`
- macOS with Homebrew: `brew install curl`
- Windows with MSYS2: `pacman -S mingw-w64-x86_64-curl`

### Installation
The Tesseract interpreter is pre-compiled and ready to use.

#### Adding Tesseract to System PATH
To run Tesseract files from anywhere on your system:

**Windows:**
1. Press `Win + R`, type `sysdm.cpl`, press Enter
2. Click "Environment Variables" button
3. In "User variables" section, select "Path" and click "Edit"
4. Click "New" and add the full path to your Tesseract directory (e.g., `C:\path\to\Tesseract`)
5. Click OK on all dialogs
6. Restart your command prompt/terminal
7. You can now run `tesser filename.tesseract` from any directory

**Linux/macOS:**
1. Add the following line to your shell profile (`~/.bashrc`, `~/.zshrc`, etc.):
   ```bash
   export PATH="$PATH:/path/to/Tesseract"
   ```
2. Reload your shell: `source ~/.bashrc` (or restart terminal)
3. You can now run `./tesser filename.tesseract` from any directory

### Fast Compilation
Tesseract uses precompiled headers to speed up compilation:
- On Linux/macOS: `make pch` will generate the precompiled header
- On Windows: Run `generate_pch.bat` before building

### Debug Mode (Linux)
```bash
gcc -g -o tesser src/*.c packages/core/package_loader.c packages/stdlib/*.c -Iinclude -lm -lcurl
```
- It is good practice to run this command before running the main Tesseract file on Linux

## Language Syntax

### General Rules
- Statements end with semicolons
- Blocks are enclosed in curly braces `{}`
- Comments start with `#`

### Variables

**Declaration and Assignment:**
```tesseract
let$var_name := value
```

**Example:**
```tesseract
let$x := 10
::print x  # prints 10
```

### Control Structures

**If-Else Statements:**
```tesseract
if$ condition {
    // then branch
} 
elseif$ condition {
    // elseif branch
}
else {
    // else branch
}
```

**Loops:**
```tesseract
loop$i := start => end {
    // loop body
    // i takes values from start to end (inclusive) with increment of 1
}

// Loop with custom increment/decrement
loop$i := start => end, increment {
    // loop body
    // i takes values from start to end with specified increment
    // Use positive values for ascending loops
    // Use negative values for descending loops
}
```

**Loop Examples:**
```tesseract
// Basic ascending loop
loop$i := 1 => 5 {
    ::print i  // prints 1, 2, 3, 4, 5
}

// Loop with increment of 2
loop$i := 0 => 10, 2 {
    ::print i  // prints 0, 2, 4, 6, 8, 10
}

// Descending loop
loop$i := 10 => 1, -1 {
    ::print i  // prints 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
}

// Loop with larger decrement
loop$i := 20 => 5, -3 {
    ::print i  // prints 20, 17, 14, 11, 8, 5
}

// Loop with fractional increment
loop$i := 0 => 2, 0.5 {
    ::print i  // prints 0, 0.5, 1, 1.5, 2
}
```

**For-Each Examples:**
```tesseract
// Iterate through numbers
let$numbers := [1, 2, 3, 4, 5]
foreach$num in numbers {
    ::print num  // prints each number
}

// Iterate through strings
let$fruits := ["apple", "banana", "cherry"]
foreach$fruit in fruits {
    ::print fruit  // prints each fruit name
}

// Direct list iteration
foreach$item in [10, 20, 30] {
    ::print item  // prints 10, 20, 30
}

// Nested iteration (2D arrays)
let$matrix := [[1, 2], [3, 4]]
foreach$row in matrix {
    foreach$cell in row {
        ::print cell  // prints 1, 2, 3, 4
    }
}
```

**While Loops:**
```tesseract
while$ condition {
    // loop body
}
```

**For-Each Loops:**
```tesseract
foreach$item in list {
    // loop body
    // item contains current element
}
```

**Break and Continue:**
```tesseract
// Break - exits the current loop
loop$i := 1 => 10 {
    if$ i == 5 {
        break$;
    }
    ::print i  // prints 1, 2, 3, 4
}

// Continue - skips to next iteration
loop$i := 1 => 5 {
    if$ i == 3 {
        continue$;
    }
    ::print i  // prints 1, 2, 4, 5
}
```

**Ternary Operator:**
```tesseract
condition ? true_value : false_value
```

**Ternary Examples:**
```tesseract
// Basic ternary with strings
let$result := x > y ? "x is greater" : "y is greater"
::print result

// Numeric ternary
let$max := x > y ? x : y
::print max

// Boolean ternary
let$is_positive := x > 0 ? true : false
::print is_positive  // prints "true" or "false"

// Nested ternary
let$largest := x > y ? (x > z ? x : z) : (y > z ? y : z)
::print largest

// Direct printing
::print 5 > 3 ? "five is greater" : "three is greater"
```

**Switch-Case Statements:**
```tesseract
switch$ expression {
    case$ value1 {
        // code for value1
    }
    case$ value2 {
        // code for value2
    }
    default$ {
        // default code
    }
}
```

### Functions

**Definition:**
```tesseract
func$name(param1, param2) => {
    // function body
    // last expression is return value
}
```

**Function Call:**
```tesseract
name(arg1, arg2)
```

### Classes

Classes require an `init` function and support method calls:

```tesseract
class$ BankAccount {
    let$ owner := "";
    let$ balance := 0;

    func$ init(owner, initial) => {
        let$ self.owner := owner;
        let$ self.balance := initial;
    }

    func$ deposit(amount) => {
        let$ self.balance := self.balance + amount;
        ::print "Deposited: @s" (amount)
    }

    func$ show() => {
        ::print "Account owner: @s" (self.owner)
        ::print "Balance: @s" (self.balance)
    }
}

# Usage
let$ acc := BankAccount()
acc.init("John", 1000)
acc.show()
acc.deposit(250)
```

## Data Types

### Lists

**Creation and Access:**
```tesseract
let$ myList := [1, 2, 3, 4, 5]
::print myList[2]  # prints 3
```

**List Functions:**
- `::len(myList)` - Get length
- `::append(myList, value)` - Add to end
- `::prepend(myList, value)` - Add to beginning
- `::pop(myList)` - Remove last element

### Dictionaries

**Creation:**
```tesseract
let$ myDict := dict{"name" := "John", "age" := 25}
```

**Operations:**
- `::get(dict, key)` - Get value
- `::set(dict, key, value)` - Set value
- `::keys(dict)` - Get all keys
- `::values(dict)` - Get all values

### Stacks

**Creation:**
```tesseract
let$ myStack := <stack>
```

**Operations:**
- `::push(stack, value)` - Push value to top
- `::pop(stack)` - Remove and return top value
- `::peek(stack)` - Return top value without removing
- `::size(stack)` - Get number of elements
- `::empty(stack)` - Check if stack is empty (returns 1 for empty, 0 for non-empty)

**Example:**
```tesseract
let$ stack := <stack>
::push(stack, 10)
::push(stack, 20)
::print ::peek(stack)  # prints 20
::print ::pop(stack)   # prints 20
::print ::size(stack)  # prints 1
```

### Queues

**Creation:**
```tesseract
let$ myQueue := <queue>
```

**Operations:**
- `::enqueue(queue, value)` - Add value to back
- `::dequeue(queue)` - Remove and return front value
- `::front(queue)` - Return front value without removing
- `::back(queue)` - Return back value
- `::isEmpty(queue)` - Check if queue is empty (returns 1 for empty, 0 for non-empty)
- `::qsize(queue)` - Get number of elements

**Example:**
```tesseract
let$ queue := <queue>
::enqueue(queue, 10)
::enqueue(queue, 20)
::print ::front(queue)   # prints 10
::print ::dequeue(queue) # prints 10
::print ::qsize(queue)   # prints 1
```

### Linked Lists

**Creation:**
```tesseract
let$ myList := <linked>
```

**Operations:**
- `::ladd(list, value)` - Add value to the list
- `::lremove(list, value)` - Remove first occurrence of value
- `::lget(list, index)` - Get element at index
- `::lsize(list)` - Get number of elements
- `::lisEmpty(list)` - Check if list is empty (returns 1 for empty, 0 for non-empty)

**Example:**
```tesseract
let$ list := <linked>
::ladd(list, 10)
::ladd(list, 20)
::ladd(list, 30)
::print ::lget(list, 1)   # prints 20
::lremove(list, 20)
::print ::lsize(list)     # prints 2
```

### Regular Expressions

**Creation:**
```tesseract
let$ myRegex := <regex> "pattern"//flags
```

**Operations:**
- `::rmatch(regex, text)` - Test if pattern matches text (returns 1 for match, 0 for no match)
- `::rfind_all(regex, text)` - Find all match positions in text
- `::rreplace(regex, text, replacement)` - Replace first match with replacement text

**Example:**
```tesseract
let$ pattern := <regex> "hello"//i
let$ text := "Hello World"
let$ match := ::rmatch(pattern, text)  # returns 1 (case-insensitive match)
::print match  # prints 1

let$ text2 := "hello world hello tesseract"
::rfind_all(pattern, text2)  # prints positions of matches
::rreplace(pattern, text2, "hi")  # prints "hi world hello tesseract"
```

**Flags:**
- `i` - Case insensitive matching
- `g` - Global matching (find/replace all occurrences)

## Exception Handling

**Try/Catch/Finally:**
```tesseract
try$ {
    let$result := 10 / 0;  # Division by zero
} catch$ {
    ::print "Error caught";
} finally$ {
    ::print "Cleanup";
}
```

**Custom Exceptions:**
```tesseract
try$ {
    throw$ "Custom error message";
} catch$ {
    ::print "Caught custom exception";
}
```

## Lambda Expressions

**Basic Syntax:**
```tesseract
let$add := (x, y) => x + y;
let$double := (x) => x * 2;
let$hello := () => "Hello World";
```

## String Interpolation

**Syntax:**
```tesseract
let$name := "World";
let$age := 25;
::print "Hello ${name}!";  # prints "Hello World!"
::print "I am ${age} years old";  # prints "I am 25 years old"
```

## Sets

**Creation:**
```tesseract
let$mySet := {1, 2, 3, 2, 1};  # Automatically removes duplicates
::print mySet;  # prints {1, 2, 3}
```



### Booleans

Available operators: `and`, `not`, `or`, `true`, `false`
- Both boolean operations and comparison operators print as `true` or `false`

**Example:**
```tesseract
let$ a := 10
let$ b := 20
::print a and b  # prints true
::print a < b    # prints true
::print a == b   # prints false
```



### Bitwise Operators

- `&` (Logical AND)
- `|` (Logical OR) 
- `^` (Bitwise XOR)
- `~` (Bitwise NOT)

**Example:**
```tesseract
5 & 3  # returns 1 (binary: 101 & 011 = 001)
```

## Advanced Features

### Type Conversion and Checking

**String/Number Conversion:**
```tesseract
::to_str(42)      # converts number to string
::to_int("123")   # converts string to number
```

**Type Checking:**
```tesseract
::type(variable)  # returns the type of a variable as a string
```

**UNDEF (Null) Values:**
```tesseract
let$x := UNDEF    # assigns undefined value
::print x         # prints "UNDEF"
::type(x)         # returns "undef"

# Variables are automatically UNDEF when accessed without assignment
::print undefined_var  # prints "UNDEF" (auto-created)
::type(undefined_var)  # returns "undef"
```

**Example:**
```tesseract
let$num := 42
let$str := "hello"
let$list := [1, 2, 3]
let$undefined := UNDEF

::print ::type(num)        # prints "number"
::print ::type(str)        # prints "string"
::print ::type(list)       # prints "list"
::print ::type(undefined)  # prints "undef"
::print ::type(nonexistent) # prints "undef"

# UNDEF evaluates to 0 in numeric contexts
let$result := undefined + 5  # result is 5
```

**Supported Types:**
- `"number"` - Numeric values
- `"string"` - String values
- `"list"` - Lists/arrays
- `"dict"` - Dictionaries
- `"stack"` - Stack data structures
- `"queue"` - Queue data structures
- `"linked_list"` - Linked list data structures
- `"regex"` - Regular expressions
- `"set"` - Set data structures
- `"temporal"` - Temporal variables
- `"undef"` - Undefined/null values

### String Methods

Tesseract provides several built-in string manipulation methods:

**String Split:**
```tesseract
::split(string, delimiter)  # Split string by delimiter, returns list
::split("hello,world", ",")  # returns ["hello", "world"]
```

**String Join:**
```tesseract
::join(list, separator)  # Join list elements with separator
let$words := ["hello", "world"]
::join(words, " ")  # returns "hello world"
```

**String Replace:**
```tesseract
::replace(string, old, new)  # Replace occurrences of old with new
::replace("hello world", "world", "tesseract")  # returns "hello tesseract"
```

**String Substring:**
```tesseract
::substring(string, start, length)  # Extract substring
::substring("hello world", 6, 5)  # returns "world"
```

**String Length:**
```tesseract
::length(string)  # Get string length
::length("hello")  # returns 5
```

**String Case Conversion:**
```tesseract
::upper(string)  # Convert to uppercase
::upper("hello")  # returns "HELLO"

::lower(string)  # Convert to lowercase
::lower("HELLO")  # returns "hello"
```

### String Formatting

Currently supports formatting with `@s`:
```tesseract
let$ name := "John"
::print "Hello @s" (name)  # prints "Hello John"
```

### Pattern Matching

Find patterns in strings:
```tesseract
::pattern_match(pattern, noise)  # returns list of starting indices
```

### Random Number Generation

The `::random(start, end, increment)` function generates random numbers within specified constraints.

**Syntax:**
```tesseract
::random(start, end)           # Random number between start and end (increment = 1)
::random(start, end, increment) # Random number with custom increment
```

**Examples:**
```tesseract
::random(1, 10)        # Random integer between 1 and 10
::random(0, 20, 2)     # Random even number between 0 and 20
::random(1, 5, 0.5)    # Random number: 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, or 5
```

### Input Function

The `::input()` function prompts the user for input and returns the entered value as a string.

**Example:**
```tesseract
let$ name := ::input("Enter your name: ")
::print "Hello, @s" (name)
```

### Import System

```tesseract
import$ "filename.tesseract"
```
- Imports and executes the specified file
- Path is relative to current file

### Libraries

Currently available: Math library
```tesseract
import$ "lib/math.tesseract"
```

### File Handling

**File Operations:**
- `::fopen(filename, mode)` - Open a file
- `::fread(file_handle)` - Read from a file
- `::fwrite(file_handle, content)` - Write to a file
- `::fclose(file_handle)` - Close a file

**File Modes:**
- `"r"`: Read - Opens a file for reading. The file must exist.
- `"w"`: Write - Creates an empty file for writing. If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
- `"a"`: Append - Appends to a file. Writing operations append data at the end of the file. The file is created if it does not exist.
- `"r+"`: Read/Update - Opens a file for update (both for reading and writing). The file must exist.
- `"w+"`: Write/Update - Creates an empty file for both reading and writing.
- `"a+"`: Append/Update - Opens a file for reading and appending.

**Example:**
```tesseract
let$ file_handle := ::fopen("test.txt", "w");
::fwrite(file_handle, "Hello, Tesseract!");
::fclose(file_handle);

let$ read_handle := ::fopen("test.txt", "r");
let$ content := ::fread(read_handle);
::fclose(read_handle);

::print(content);
```

### HTTP Web Requests

Tesseract supports making HTTP requests to interact with web services:

**HTTP GET Request:**
```tesseract
# Simple GET request
let$response := ::http_get("http://example.com/api")
::print response

# GET request with headers
let$headers := dict{"User-Agent" := "Tesseract/1.0", "Accept" := "application/json"}
let$response := ::http_get("http://example.com/api", headers)
```

**HTTP POST Request:**
```tesseract
# POST request with data
let$data := "{\"name\": \"Tesseract\", \"value\": 42}"
let$response := ::http_post("http://example.com/api", data)

# POST with data and headers
let$headers := dict{"Content-Type" := "application/json"}
let$response := ::http_post("http://example.com/api", data, headers)
```

**HTTP PUT and DELETE Requests:**
```tesseract
# PUT request
let$data := "{\"updated\": true}"
let$response := ::http_put("http://example.com/api/resource", data)

# DELETE request
let$response := ::http_delete("http://example.com/api/resource")
```

## Implementation Details

### Supported AST Node Types
- Numbers, Strings, Variables
- Binary operations (+, -, *, /, %, comparisons)
- Assignments, Control structures
- Function definitions and calls
- Lists, Booleans, Classes
- Import statements, Print statements
- Type conversions (to_str, to_int)

### Memory Management
- Automatic memory allocation/deallocation for AST nodes
- Variables stored in symbol table
- Functions stored in function table

### Error Handling
- **Enhanced Error System**: Comprehensive error handling with line numbers and better error messages
- **Line Number Tracking**: Errors show exact line and file location
- **Error Types**: Syntax, runtime, division by zero, undefined variables, type mismatches, index out of bounds, file not found
- **Error Format**: `Error: [Type] - [Message] (in [file]:[line])`
- **Example**: `Error: Division by Zero - Division by zero (in test.tesseract:6)`

### Current Limitations
- Maximum 4 function parameters
- No error recovery

## Generators and Iterators

Tesseract supports generators and iterators for creating sequences of values on-demand.

**Generator Definition:**
```tesseract
gen$ range(start, end) => {
    let$ i := start;
    while$ i < end {
        yield$ i;
        i := i + 1;
    }
}
```

**Iterator Creation and Usage:**
```tesseract
let$ my_iter := iter$ range(0, 5);
::print next$ my_iter;  # prints 0
::print next$ my_iter;  # prints 1
::print next$ my_iter;  # prints 2
```

**Key Features:**
- Lazy evaluation - values generated only when requested
- Memory efficient - only current state maintained
- Support for infinite sequences
- State preservation between calls

See [GENERATORS.md](GENERATORS.md) for detailed documentation.

## Roadmap

### Planned Features
- Nothing, for now...