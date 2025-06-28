1. Tesseract is a simple interpreted programming language with:
    -Dynamic typing
    -Basic arithmetic operations
    -Control structures (if/else, loops)
    -Function definitions and calls
    -File imports
    -String and number support
    -Libraries
    -REPL

2. General Syntax Rules
    -Statements end with semicolons (optional in most cases)
    -Blocks are enclosed in curly braces {}
    -Comments: Are accessed by using a hashtag "#" and then typing the commented code

    let$       - variable declaration
    ::print    - print function
    if$        - if statement
    elseif$    - else-if statement
    else       - else statement
    loop$      - loop statement
    import$    - import statement
    func$      - function declaration
    :=         - assignment operator
    => or ⟶   - arrow operator (for ranges and function definitions)
    + - * / %  - arithmetic operators
    > < >= <= == != - comparison operators
    ()         - function calls and grouping
    {}         - code blocks
    ""         - string literals
    ::         - built in functions(print is the only one to date)
    $          - put after a statement or declaration

3. Variables
    Declaration and Assignment
        let$var_name := value
    Usage
        let$x := 10
        ::print x  (prints 10)

4. Control structures
    If-Else Statements
        if$ condition {
            // then branch
        } 
        elseif$ condition {
            // elseif branch
        }
        else {
            // else branch
        }
    Loop Statements
        loop$i := start => end {
            // loop body
            // i will take values from start to end (inclusive)
        }

5. Functions
    Function Definitions
        func$name(param1, param2) => {
            (function body)
            (last expression is return value)
        }
    Function Call
        name(arg1, arg2)

6. Import System
    import$ "filename.tesseract"
    
    -Imports and executes the specified file
    -Path is relative to the current file

7. Lists
    -Lists can be made by declaring a list.
        let$ myList := [1,2,3,4,5,6,7]
        ::print myList (Prints the list)
    -You can even index lists to get specific values
        ::print mylist[2] (Prints 3)
    -List Functions
        ::len(myList) - Gets the length of the list
        ::append(mylist,value) - Adds the value to the end of the list
        ::prepend(myList,value) - Adds the value to the starting of the list
        ::pop(myList) - Deletes the ending value of the list
        ::insert(myList) - CURRENTLY NOT WORKING, PLEASE DON'T USE THIS FUNCTION
        ::remove(myList, index) - Deletes the value of that list at that index


8. Libraries
    -Until now, the only library that exists is math.
    -You may access this by typing import$ "lib/math.tesseract" at the top of your program
    -The functions can be viewed by going to the file
    -Please note that when you use these functions you don't need to print, you can just use the function and it will print directly.

9.Implementation Notes
    AST Node Types
        -The language supports the following AST node types:
        -Numbers, Strings, Variables
        -Binary operations (+, -, *, /, %, comparisons)
        -Assignments
        -If/elseif/else statements
        -Loops
        -Function definitions and calls
        -Blocks of code
        -Import statements
        -Print statements

    Memory Management
        -The interpreter handles memory allocation and deallocation for AST nodes
        -Variables are stored in a symbol table
        -Functions are stored in a function table

    Error Handling
        -Syntax errors cause immediate termination with error messages
        -Runtime errors (like undefined variables) also terminate execution with messages

    Limitations
        -Maximum of 4 function parameters
        -Maximum of 1024 variables
        -No error recovery - first error terminates execution
        -No dictionaries or classes, yet...
