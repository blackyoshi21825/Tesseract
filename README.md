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
::front(queue)   # prints 10
::dequeue(queue) # prints 10
::print ::qsize(queue)   # prints 1
```

### Booleans

Available operators: `and`, `not`, `or`, `true`, `false`
- `true` returns 1, `false` returns 0
- Use only with comparative operators

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
- File handling
- Additional data structures
- Algorithm implementations
- Enhanced error handling
- Cross-platform support