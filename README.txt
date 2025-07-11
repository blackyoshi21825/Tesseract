1. Tesseract is a simple interpreted programming language with:
    -Dynamic typing
    -Basic arithmetic operations
    -Control structures (if/else, loops)
    -Function definitions and calls
    -File imports
    -String and number support
    -Libraries
    -Lists
    -Booleans
    -BitWise Operators
    -Pattern Matching
    -Classes

2. General Syntax Rules
    -Statements end with semicolons (optional in most cases)
    -Blocks are enclosed in curly braces {}
    -Comments: Are accessed by using a hashtag "#" and then typing the commented code

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

8. Libraries
    -Until now, the only library that exists is math.
    -You may access this by typing import$ "lib/math.tesseract" at the top of your program
    -The functions can be viewed by going to the file
    -Please note that when you use these functions you don't need to print, you can just use the function and it will print directly.

9. Booleans
    -The Booleans that are avaliable are and, not, or, true, and false
    -Please note that True returns 1 and False returns 0, so please keep that in mind.
    -You can use true or false in only comparitive operators, so regular equal sign will not work.

10. BitWise Operaters
    -The BitWise operators are &(AND), |(OR), ^(XOR), and ~(NOT)
    -These do not behave like regular booleans, but they return values instead of 1(true) or 0(false).
    -For example, 5 & 3 returns 1(NOT BOOLEAN TRUE BY THE WAY), because 5 is 101 in binary, and 3 is 011.
    -Then, each digit is compared with AND, and then the outputted value is 001, which is one in binary.

11. Pattern Matching
    -This function takes two arguments; first one is the pattern and the second one is the noise. Both of them have to be strings
    -This function finds the pattern in the noise and outputs a list with the starting indexes of where the pattern is in the noise
    -Please note that this function uses a naive algorithm, so expect delays or issues
    -To call this function, ::pattern_match(pattern,noise)

12. Formatting
    -Currently, formatting only works with strings, not numbers or floats, if you would like to use numebrs, enclose them in quotations.
    -To use formatting, use the @s for strings. @d and @f are for numbers and floats, respectively, but these currently raise errors. This error will be fixed ASAP.
    -Usage
            let$ name := "John"
            ::print "Hello @s" (name) //Print Hello John
13. Classes
    -In classes, you first have to use an init function.
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
                ::print "New balance: @s" (self.balance)
            }

            func$ withdraw(amount) => {
                if$ self.balance >= amount {
                    let$ self.balance := self.balance - amount;
                    ::print "Withdrew: @s" (amount)
                }
                else {
                    ::print "Insufficient funds!"
                }
                ::print "Balance: @s" (self.balance)
            }

            func$ show() => {
                ::print "Account owner: @s" (self.owner)
                ::print "Balance: @s" (self.balance)
            }
        }

        let$ acc := BankAccount()
        acc.init("John", 1000)
        acc.show()
        acc.deposit(250)
        acc.withdraw(500)
        acc.withdraw(1000)
        acc.show()
14. Implementation Notes
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
        -Lists
        -Booleans
        -BitWise Operators
        -Pattern Matching
        -Formatting
        -Classes

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
        -No dictionaries yet...

15. Running Tesseract
    -This language can only be run on a Mac or Linux computer, if you have a Windows, install WSL and run it that way.
    -Install GCC and Make tools to run Tesseract.
    -On Mac, use "make" and then "make clear" to use test.tesseract
    -On Linux, if you get a segmentation fault, even though you know that your code is right run this command to debug your code
    "gcc -g -o tesser src/*.c -Iinclude -lm"