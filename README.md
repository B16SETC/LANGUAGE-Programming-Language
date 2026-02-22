# LANGUAGE Programming Language

LANGUAGE is a minimalist, whitespace-sensitive programming language with a clean, readable syntax. It supports variables, arithmetic, strings, arrays, booleans, control flow, functions, loops, user input, type conversion, math built-ins, file I/O, error handling, and imports.

---

## Table of Contents

- [Running a Script](#running-a-script)
- [Comments](#comments)
- [Variables](#variables)
- [Data Types](#data-types)
- [Arithmetic](#arithmetic)
- [String Operations](#string-operations)
- [Math Built-ins](#math-built-ins)
- [Type Conversion](#type-conversion)
- [Booleans & Logical Operators](#booleans--logical-operators)
- [If / Elif / Else](#if--elif--else)
- [While Loops](#while-loops)
- [For Loops](#for-loops)
- [Break & Continue](#break--continue)
- [Arrays](#arrays)
- [Functions](#functions)
- [User Input](#user-input)
- [File I/O](#file-io)
- [Error Handling](#error-handling)
- [Importing Files](#importing-files)

---

## Running a Script

Save your code in a file with the `.LANGUAGE` extension and run it:

```bash
LANGUAGE myscript.LANGUAGE
```

Other options:

```bash
LANGUAGE --version    # Show version
LANGUAGE --help       # Show usage info
```

---

## Comments

Comments are wrapped in `#` characters. They can span multiple lines.

```
# This is a comment #

# This is a
  multi-line comment #

Print "Hello"  # inline comment #
```

---

## Variables

Variables are assigned with `=`. There is no type declaration — the type is inferred from the value.

```
name = "Alice"
age = 30
pi = 3.14159
isActive = True
```

Variable names are case-sensitive and can contain letters, numbers, and underscores, but must start with a letter or underscore.

---

## Data Types

LANGUAGE has four data types:

| Type    | Example              |
|---------|----------------------|
| Number  | `42`, `3.14`, `-7`   |
| String  | `"Hello, World!"`    |
| Boolean | `True`, `False`      |
| Array   | `[1, 2, 3]`          |

---

## Arithmetic

The standard arithmetic operators are supported:

```
a = 10
b = 3

Print a + b   # 13
Print a - b   # 7
Print a * b   # 30
Print a / b   # 3.333333
```

Strings can be concatenated with `+`:

```
Print "Hello" + ", " + "World!"   # Hello, World!
```

You can also mix numbers and strings — the number is automatically converted:

```
x = 5
Print "The value is: " + x   # The value is: 5
```

---

## String Operations

These built-in functions operate on strings:

```
s = "Hello, LANGUAGE!"

Print Length(s)              # 16
Print Upper(s)               # HELLO, LANGUAGE!
Print Lower(s)               # hello, language!
Print Substring(s, 7, 8)     # LANGUAGE   (start index, length)
Print Contains(s, "LANG")    # 1 (true)
Print Contains(s, "Python")  # 0 (false)
```

### Substring

`Substring(string, startIndex, length)` — extracts `length` characters starting at `startIndex` (zero-based).

```
Print Substring("abcdef", 2, 3)   # cde
```

### Contains

`Contains(string, search)` — returns `1` if the search string is found, `0` if not.

---

## Math Built-ins

```
Print Floor(3.7)       # 3
Print Ceil(3.2)        # 4
Print Sqrt(144)        # 12
Print Abs(-99)         # 99
Print Power(2, 10)     # 1024
```

| Function      | Description                  |
|---------------|------------------------------|
| `Floor(x)`    | Round down to nearest integer |
| `Ceil(x)`     | Round up to nearest integer  |
| `Sqrt(x)`     | Square root                  |
| `Abs(x)`      | Absolute value               |
| `Power(x, n)` | x raised to the power of n   |

---

## Type Conversion

```
numStr = "42"
num = ToNumber(numStr)
Print num + 8           # 50

n = 3.14
Print ToString(n)       # 3.14
```

| Function       | Description                        |
|----------------|------------------------------------|
| `ToNumber(x)`  | Convert string or boolean to number |
| `ToString(x)`  | Convert any value to a string      |

---

## Booleans & Logical Operators

```
t = True
f = False

Print t And f    # False
Print t Or f     # True
Print Not t      # False
```

Logical operators work in conditions:

```
x = 5

If x > 0 And x < 10
  Print "single digit positive"
End

If x < 0 Or x > 3
  Print "negative or greater than 3"
End

If Not x == 0
  Print "x is not zero"
End
```

---

## If / Elif / Else

Blocks are defined by indentation (2 spaces) and closed with `End`.

```
score = 85

If score >= 90
  Print "Grade: A"
Elif score >= 80
  Print "Grade: B"
Elif score >= 70
  Print "Grade: C"
Else
  Print "Grade: F"
End
```

You can have as many `Elif` clauses as you need. `Else` is optional.

---

## While Loops

```
i = 1
While i <= 5
  Print i
  i = i + 1
End
```

The loop runs as long as the condition is `True`.

---

## For Loops

`For` loops iterate over a range of numbers, inclusive on both ends.

```
For i = 1 To 5
  Print i
End
```

This prints `1`, `2`, `3`, `4`, `5`.

The loop variable can be used inside the body:

```
For n = 1 To 10
  Print n * n
End
```

---

## Break & Continue

`Break` exits the loop immediately. `Continue` skips to the next iteration.

```
# Print only odd numbers, stop at 9
For i = 1 To 20
  If i > 9
    Break
  End
  If i * Floor(i / 2) == i And Not i == 0
    Continue
  End
  Print i
End
```

Both work inside `While` and `For` loops.

---

## Arrays

Arrays are ordered lists of values and can hold any mix of types.

### Creating an Array

```
fruits = ["Apple", "Banana", "Cherry"]
nums = [10, 20, 30]
mixed = [1, "hello", True]
```

### Accessing Elements

Arrays are zero-indexed:

```
Print fruits[0]   # Apple
Print fruits[2]   # Cherry
```

### Modifying Elements

```
fruits[1] = "Blueberry"
Print fruits   # [Apple, Blueberry, Cherry]
```

### Push & Pop

`Push` adds an element to the end. `Pop` removes and discards the last element.

```
Push(fruits, "Dragonfruit")
Print fruits   # [Apple, Blueberry, Cherry, Dragonfruit]

Pop(fruits)
Print fruits   # [Apple, Blueberry, Cherry]
```

### Array Length

```
Print Length(fruits)   # 3
```

### Iterating an Array

```
names = ["Alice", "Bob", "Charlie"]
For i = 0 To Length(names) - 1
  Print names[i]
End
```

---

## Functions

Functions are defined with `Func` and closed with `End`. They can take parameters and return a value with `Return`.

```
Func greet(name)
  Return "Hello, " + name + "!"
End

Print greet("Alice")   # Hello, Alice!
```

### Multiple Parameters

```
Func add(a, b)
  Return a + b
End

Print add(3, 7)   # 10
```

### Recursion

```
Func factorial(n)
  If n <= 1
    Return 1
  End
  Return n * factorial(n - 1)
End

Print factorial(6)   # 720
```

### Functions Without a Return Value

If a function doesn't hit a `Return` statement, it returns `0` by default.

```
Func sayHello()
  Print "Hello!"
End

sayHello()
```

---

## User Input

`Input` reads a line of text from the user. You can provide a prompt string.

```
name = Input("What is your name? ")
Print "Hello, " + name + "!"
```

```
ageStr = Input("Enter your age: ")
age = ToNumber(ageStr)
Print "Next year you will be " + ToString(age + 1)
```

Input always returns a string, so use `ToNumber` if you need to do math with it.

---

## File I/O

### Reading a File

```
contents = ReadFile("data.txt")
Print contents
```

### Writing a File

`WriteFile` creates or overwrites the file:

```
WriteFile("output.txt", "Hello from LANGUAGE!\n")
```

### Appending to a File

`AppendFile` adds to the end of an existing file without overwriting:

```
AppendFile("output.txt", "Another line\n")
```

### Full Example

```
WriteFile("log.txt", "Start of log\n")
AppendFile("log.txt", "Line 2\n")
AppendFile("log.txt", "Line 3\n")

log = ReadFile("log.txt")
Print log
```

---

## Error Handling

Use `Try` and `Catch` to handle runtime errors gracefully. The `Catch` block receives the error message as a variable.

```
Try
  result = 10 / 0
Catch(err)
  Print "Error caught: " + err
End
```

```
Try
  data = ReadFile("missing.txt")
Catch(err)
  Print "Could not read file: " + err
End
```

The variable name inside `Catch(...)` can be anything you like. If you omit it, the error is stored in a variable called `error`:

```
Try
  x = 1 / 0
Catch
  Print "Something went wrong"
End
```

---

## Importing Files

You can split your code across multiple `.LANGUAGE` files and import them. Imported files can define functions and variables that become available in the importing file.

### Same Directory

```
Import "mathlib.LANGUAGE"
```

### Subdirectory

```
Import "libs/stringlib.LANGUAGE"
```

### Absolute Path

```
Import "/home/james/mylibs/utils.LANGUAGE"
```

Paths are resolved relative to the **directory of the file doing the importing**, not the working directory of the terminal. So if your script is at `/projects/myapp/main.LANGUAGE` and it does `Import "utils.LANGUAGE"`, LANGUAGE looks for `/projects/myapp/utils.LANGUAGE`.

### Duplicate Imports

Importing the same file more than once is safe — LANGUAGE tracks what's been imported and silently skips duplicates:

```
Import "mathlib.LANGUAGE"
Import "mathlib.LANGUAGE"   # Ignored, already loaded
```

### Example Library File (`stringlib.LANGUAGE`)

```
Func repeat(s, n)
  result = ""
  For i = 1 To n
    result = result + s
  End
  Return result
End

Print "Loaded stringlib.LANGUAGE"
```

### Using It

```
Import "stringlib.LANGUAGE"

Print repeat("ha", 4)   # hahahaha
```

---

## Indentation Rules

LANGUAGE is whitespace-sensitive. Blocks must be indented with **2 spaces** per level. Mixing tabs and spaces or using other indentation sizes will cause errors.

```
If x > 0
  Print "positive"     # 2 spaces
  If x > 10
    Print "big"        # 4 spaces (nested)
  End
End
```

---

## Quick Reference

```
# Variables
x = 42
name = "LANGUAGE"
flag = True
nums = [1, 2, 3]

# Output
Print "Hello, " + name

# Input
answer = Input("Enter something: ")

# Arithmetic
Print 10 + 3    # 13
Print 10 - 3    # 7
Print 10 * 3    # 30
Print 10 / 3    # 3.333333

# String ops
Print Length("hello")         # 5
Print Upper("hello")          # HELLO
Print Lower("HELLO")          # hello
Print Substring("hello", 1, 3) # ell
Print Contains("hello", "ell") # 1

# Math
Print Floor(3.9)    # 3
Print Ceil(3.1)     # 4
Print Sqrt(25)      # 5
Print Abs(-7)       # 7
Print Power(2, 8)   # 256

# Type conversion
Print ToNumber("99")   # 99
Print ToString(3.14)   # 3.14

# Conditionals
If x > 10
  Print "big"
Elif x > 5
  Print "medium"
Else
  Print "small"
End

# Loops
While x > 0
  x = x - 1
End

For i = 1 To 5
  Print i
End

# Arrays
Push(nums, 4)
Pop(nums)
Print nums[0]
Print Length(nums)

# Functions
Func square(n)
  Return n * n
End

# File I/O
WriteFile("out.txt", "hello")
AppendFile("out.txt", " world")
Print ReadFile("out.txt")

# Error handling
Try
  x = 1 / 0
Catch(err)
  Print err
End

# Import
Import "mylib.LANGUAGE"
```
