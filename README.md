<h2>1. Overview</h2><br>
Tesseract is a simple interpreted programming language with dynamic typing, basic arithmetic operations, control structures (if/else, loops), function definitions and calls, file imports, string and number support, libraries, lists, booleans, bitWise operators, pattern matching, and classes

<h2>2. General Syntax Rules</h2> <br>
    -Statements end with semicolons (optional in most cases) <br>
    -Blocks are enclosed in curly braces {} <br>
    -Comments: Are accessed by using a hashtag "#" and then typing the commented code <br>

<h2>3. Variables</h2>
    Declaration and Assignment

        
        let$var_name := value
        

    Usage

       
        let$x := 10
        ::print x  (prints 10)
        

<h2>4. Control structures</h2>
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
        ```
    Loop Statements

        
        loop$i := start => end {
            // loop body
            // i will take values from start to end (inclusive)
        }
        

<h2>5. Functions</h2>
    Function Definitions

        
        func$name(param1, param2) => {
            (function body)
            (last expression is return value)
        }
        

    Function Call

        name(arg1, arg2)
        

<h2>6. Import System</h2>

    ```
    import$ "filename.tesseract"
    ```

    -Imports and executes the specified file
    -Path is relative to the current file

<h2>7. Lists</h2>
    -Lists can be made by declaring a lists

        ```
        let$ myList := [1,2,3,4,5,6,7]
        ::print myList (Prints the list)
        ```

    -You can even index lists to get specific values

        ```
        ::print mylist[2] (Prints 3)
        ```

    -List Functions

        
        ::len(myList) - Gets the length of the list
        ::append(mylist,value) - Adds the value to the end of the list
        ::prepend(myList,value) - Adds the value to the starting of the list
        ::pop(myList) - Deletes the ending value of the list
        

<h2>8. Libraries</h2> <br>
    -Until now, the only library that exists is math. <br>
    -You may access this by typing import$ "lib/math.tesseract" at the top of your program <br>
    -The functions can be viewed by going to the file <br>
    -Please note that when you use these functions you don't need to print, you can just use the function and it will print directly. <br>

<h2>9. Booleans</h2> <br>
    -The Booleans that are avaliable are and, not, or, true, and false <br>
    -Please note that True returns 1 and False returns 0, so please keep that in mind. <br>
    -You can use true or false in only comparitive operators, so regular equal sign will not work. <br>

<h2>10. BitWise Operaters</h2> <br>
    -The BitWise operators are &(AND), |(OR), ^(XOR), and ~(NOT) <br>
    -These do not behave like regular booleans, but they return values instead of 1(true) or 0(false). <br>
    -For example, 5 & 3 returns 1(NOT BOOLEAN TRUE BY THE WAY), because 5 is 101 in binary, and 3 is 011. <br>
    -Then, each digit is compared with AND, and then the outputted value is 001, which is one in binary. <br>

<h2>11. Pattern Matching</h2> <br>
    -This function takes two arguments; first one is the pattern and the second one is the noise. Both of them have to be strings <br>
    -This function finds the pattern in the noise and outputs a list with the starting indexes of where the pattern is in the noise <br>
    -Please note that this function uses a naive algorithm, so expect delays or issues <br>
    -To call this function use the floowing syntax <br>
    
        ::pattern_match(pattern,noise)

<h2>12. Formatting</h2> <br> 
    -Currently, formatting only works with strings, not numbers or floats, if you would like to use numebrs, enclose them in quotations. <br>
    -To use formatting, use the @s for strings. @d and @f are for numbers and floats, respectively, but these currently raise errors. This error will be fixed ASAP. <br>
    -Usage

        
        let$ name := "John"
        ::print "Hello @s" (name) // Prints Hello John
        

<h2>13. Classes</h2>
    -In classes, you first have to use an init function. Then, you may create functions and then call these fucntions by assigning the class to a variable, as shown in the example below

    ```
        let$ variable := myClass()
    ```
    Then, you may call the functions created in the class by calling it using the variable.
    ```
        variable.function("param1","param2")
    ```

    A full class is shows below

    ```
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
    ```

<h2>14. Implementation Notes</h2> <br>
    AST Node Types <br>
        -The language supports the following AST node types: <br>
        -Numbers, Strings, Variables <br>
        -Binary operations (+, -, *, /, %, comparisons) <br>
        -Assignments <br>
        -If/elseif/else statements <br>
        -Loops <br>
        -Function definitions and calls <br>
        -Blocks of code <br>
        -Import statements <br>
        -Print statements <br>
        -Lists <br>
        -Booleans <br>
        -BitWise Operators <br>
        -Pattern Matching <br>
        -Formatting <br>
        -Classes <br>
    Memory Management <br>
        -The interpreter handles memory allocation and deallocation for AST nodes <br>
        -Variables are stored in a symbol table <br>
        -Functions are stored in a function table <br>
    Error Handling <br>
        -Syntax errors cause immediate termination with error messages <br>
        -Runtime errors (like undefined variables) also terminate execution with messages <br>
    Limitations <br>
        -Maximum of 4 function parameters <br>
        -Maximum of 1024 variables <br>
        -No error recovery - first error terminates execution <br>
        -No dictionaries yet... <br>

<h2>15. Running Tesseract</h2> <br>
    -This language can only be run on a Mac or Linux computer, if you have a Windows, install WSL and run it that way. <br>
    -Install GCC and Make tools to run Tesseract. <br>
    -On Mac, use "make" and then "make clear" to use test.tesseract <br>
    -On Linux, if you get a segmentation fault, even though you know that your code is right run this command to debug your code <br>

    
    gcc -g -o tesser src/*.c -Iinclude -lm
    

<h2>16. Dictionaries</h2><br>
    -Dictionaries can be created using dict{} syntax with key := value pairs<br>
    -Access values with ::get(dict, key)<br>
    -Set values with ::set(dict, key, value)<br>
    -Get all keys with ::keys(dict)<br>
    -Get all values with ::values(dict)<br>
    
        ```
        let$ myDict := dict{"name" := "John", "age" := 25}
        ::print ::get(myDict, "name")  # Prints John
        ::set(myDict, "city", "NYC")
        ::keys(myDict)  # Prints all keys
        ```

<h2>17. Things That Will Be Added </h2><br>
    -File Handling <br>
    -Data structures <br>
    -Algorithms <br>