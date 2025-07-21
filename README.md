# Tesseract Programming Language

## Overview

Tesseract is a simple interpreted programming language featuring dynamic typing, arithmetic operations, control structures, functions, classes, and more. It supports modern programming concepts while maintaining simplicity.

**Key Features:**
- Dynamic typing
- Control structures (if/else, loops)
- Functions and classes
- Lists and dictionaries
- Pattern matching
- File imports
- Boolean and bitwise operations
- File Handling
- Advanced Data Structures(Stacks, Queues, and Linked Lists)
- While Loops

## Getting Started

### Prerequisites
- GCC compiler
- Make tools
- Mac/Linux (Windows users should use WSL)

### Installation
```bash
make
make clear  # to run test.tesseract
```

### Debug Mode (Linux)
```bash
gcc -g -o tesser src/*.c -Iinclude -lm
```
- It is good practice to run this command before running the main Tesseract file on Linux

## Language Syntax

### General Rules
- Statements end with semicolons (optional in most cases)
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
    // i takes values from start to end (inclusive)
}
```

**While Loops:**
```tesseract
while$ condition {
    // loop body
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

## Implementation Details

### Supported AST Node Types
- Numbers, Strings, Variables
- Binary operations (+, -, *, /, %, comparisons)
- Assignments, Control structures
- Function definitions and calls
- Lists, Booleans, Classes
- Import statements, Print statements

### Memory Management
- Automatic memory allocation/deallocation for AST nodes
- Variables stored in symbol table
- Functions stored in function table

### Error Handling
- Syntax errors cause immediate termination
- Runtime errors terminate with error messages
- No error recovery mechanism

### Current Limitations
- Maximum 4 function parameters
- No error recovery

## Roadmap

### Planned Features
- Nothing, for now...