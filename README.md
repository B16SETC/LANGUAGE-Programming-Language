# LANGUAGE Programming Language

LANGUAGE is a minimalist, whitespace-sensitive programming language with a clean, readable syntax. It supports variables, arithmetic, strings, arrays, booleans, control flow, functions, loops, user input, type conversion, an extensive math library, file I/O, error handling, TCP networking, and imports.

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
- [TCP Sockets](#tcp-sockets)
- [Importing Files](#importing-files)

---

## Running a Script

```bash
LANGUAGE myscript.LANGUAGE
LANGUAGE --version
LANGUAGE --help
```

---

## Comments

```
# This is a comment #

# Multi
  line comment #

Print "Hello"  # inline #
```

---

## Variables

```
name = "Alice"
age = 30
pi = 3.14159
isActive = True
```

---

## Data Types

| Type    | Example           |
|---------|-------------------|
| Number  | `42`, `3.14`, `-7`|
| String  | `"Hello"`         |
| Boolean | `True`, `False`   |
| Array   | `[1, 2, 3]`       |

---

## Arithmetic

```
Print 10 + 3    # 13
Print 10 - 3    # 7
Print 10 * 3    # 30
Print 10 / 3    # 3.333333

Print "Hello" + " World"   # Hello World
Print "Value: " + 42       # Value: 42
```

---

## String Operations

```
s = "Hello, LANGUAGE!"

Print Length(s)              # 16
Print Upper(s)               # HELLO, LANGUAGE!
Print Lower(s)               # hello, language!
Print Substring(s, 7, 8)     # LANGUAGE
Print Contains(s, "LANG")    # 1
Print Contains(s, "Python")  # 0
```

`Substring(string, startIndex, length)` — zero-based index.
`Contains` returns `1` if found, `0` if not.

---

## Math Built-ins

### Basic

```
Print Floor(3.7)        # 3
Print Ceil(3.2)         # 4
Print Round(3.5)        # 4
Print Abs(-99)          # 99
Print Sqrt(144)         # 12
Print Power(2, 10)      # 1024
Print Mod(17, 5)        # 2
Print Min(3, 7)         # 3
Print Max(3, 7)         # 7
Print Clamp(15, 0, 10)  # 10
Print Lerp(0, 100, 0.3) # 30
```

| Function             | Description                           |
|----------------------|---------------------------------------|
| `Floor(x)`           | Round down                            |
| `Ceil(x)`            | Round up                              |
| `Round(x)`           | Round to nearest                      |
| `Abs(x)`             | Absolute value                        |
| `Sqrt(x)`            | Square root                           |
| `Power(x, n)`        | x to the power of n                  |
| `Mod(a, b)`          | Remainder                             |
| `Min(a, b)`          | Smaller of two                        |
| `Max(a, b)`          | Larger of two                         |
| `Clamp(x, min, max)` | Constrain between min and max        |
| `Lerp(a, b, t)`      | Linear interpolation                  |

### Trigonometry

All trig functions use **radians**.

```
Print Sin(0)            # 0
Print Cos(0)            # 1
Print Tan(0)            # 0
Print Asin(1)           # 1.5708
Print Acos(1)           # 0
Print Atan(1)           # 0.7854
Print Atan2(1, 1)       # 0.7854
Print Deg2Rad(180)      # 3.14159
Print Rad2Deg(3.14159)  # 180
```

| Function      | Description                                 |
|---------------|---------------------------------------------|
| `Sin(x)`      | Sine (radians)                              |
| `Cos(x)`      | Cosine (radians)                            |
| `Tan(x)`      | Tangent (radians)                           |
| `Asin(x)`     | Arcsine                                     |
| `Acos(x)`     | Arccosine                                   |
| `Atan(x)`     | Arctangent                                  |
| `Atan2(y, x)` | Two-argument arctangent                     |
| `Deg2Rad(x)`  | Degrees to radians                          |
| `Rad2Deg(x)`  | Radians to degrees                          |

### Hyperbolic

```
Print Sinh(1)    # 1.1752
Print Cosh(1)    # 1.5431
Print Tanh(1)    # 0.7616
Print Asinh(1)   # 0.8814
Print Acosh(1)   # 0
Print Atanh(0.5) # 0.5493
```

### Logarithms & Exponentials

```
Print Log(2.71828)   # ~1
Print Log10(1000)    # 3
Print Log2(8)        # 3
Print Exp(1)         # 2.71828
Print LogBase(8, 2)  # 3
```

| Function      | Description                  |
|---------------|------------------------------|
| `Log(x)`      | Natural log (base e)         |
| `Log10(x)`    | Base-10 log                  |
| `Log2(x)`     | Base-2 log                   |
| `Exp(x)`      | e to the power of x         |
| `LogBase(x,b)`| Log of x in any base         |

### Number Theory

```
Print Factorial(6)      # 720
Print IsPrime(17)       # 1
Print GCD(12, 8)        # 4
Print LCM(4, 6)         # 12
Print RandomInt(1, 100) # random 1-100
```

### Extra Math

```
Print Sign(-5)          # -1
Print Sign(0)           # 0
Print Sign(7)           # 1
Print Truncate(3.9)     # 3
Print Truncate(-3.9)    # -3
Print Frac(3.75)        # 0.75
Print Hypot(3, 4)       # 5
Print Cbrt(27)          # 3
Print Cbrt(-8)          # -2
Print CopySign(5, -1)   # -5
```

| Function          | Description                              |
|-------------------|------------------------------------------|
| `Sign(x)`         | -1, 0, or 1                             |
| `Truncate(x)`     | Chop decimal (no rounding)              |
| `Frac(x)`         | Fractional part only                    |
| `Hypot(a, b)`     | sqrt(a²+b²), safe distance calc        |
| `Cbrt(x)`         | Cube root (handles negatives correctly) |
| `CopySign(x, y)`  | x with the sign of y                   |
| `LogBase(x, b)`   | Logarithm in any base                   |

### Number Checks

```
Print IsNaN(0)     # False
Print IsInf(1)     # False
Print IsEven(4)    # True
Print IsOdd(7)     # True
```

### Bitwise

```
Print BitAnd(12, 10)       # 8
Print BitOr(12, 10)        # 14
Print BitXor(12, 10)       # 6
Print BitNot(0)            # -1
Print BitShiftLeft(1, 4)   # 16
Print BitShiftRight(16, 2) # 4
```

### Statistics (Array-Based)

```
nums = [4, 8, 15, 16, 23, 42]

Print Sum(nums)       # 108
Print Product(nums)   # 7418880
Print Mean(nums)      # 18
Print Median(nums)    # 15.5
Print Variance(nums)  # 151.666
Print StdDev(nums)    # 12.315
```

### Pure Mathematics

```
Print Gamma(5)       # 24
Print Beta(2, 3)     # 0.0833
Print Erf(1)         # 0.8427
Print Erfc(1)        # 0.1573
```

---

## Type Conversion

```
num = ToNumber("42")
Print num + 8        # 50

Print ToString(3.14) # 3.14
```

---

## Booleans & Logical Operators

```
Print True And False   # False
Print True Or False    # True
Print Not True         # False
```

```
x = 5
If x > 0 And x < 10
  Print "single digit positive"
End
```

---

## If / Elif / Else

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

---

## While Loops

```
i = 1
While i <= 5
  Print i
  i = i + 1
End
```

---

## For Loops

```
For i = 1 To 5
  Print i
End
```

Inclusive on both ends.

---

## Break & Continue

```
For i = 1 To 10
  If i > 5
    Break
  End
  If Mod(i, 2) == 0
    Continue
  End
  Print i   # 1, 3, 5
End
```

---

## Arrays

```
fruits = ["Apple", "Banana", "Cherry"]
Print fruits[0]        # Apple
fruits[1] = "Blueberry"
Push(fruits, "Dragonfruit")
Pop(fruits)
Print Length(fruits)   # 3

For i = 0 To Length(fruits) - 1
  Print fruits[i]
End
```

---

## Functions

```
Func add(a, b)
  Return a + b
End

Print add(3, 7)   # 10
```

### Recursion

```
Func fib(n)
  If n <= 1
    Return n
  End
  Return fib(n - 1) + fib(n - 2)
End
```

Functions return `0` by default if no `Return` is hit.

---

## User Input

```
name = Input("What is your name? ")
Print "Hello, " + name + "!"

age = ToNumber(Input("Your age: "))
Print "Next year: " + ToString(age + 1)
```

---

## File I/O

```
WriteFile("out.txt", "Hello\n")
AppendFile("out.txt", "World\n")
contents = ReadFile("out.txt")
Print contents
```

---

## Error Handling

```
Try
  result = 10 / 0
Catch(err)
  Print "Caught: " + err
End

Try
  data = ReadFile("missing.txt")
Catch(err)
  Print "File error: " + err
End
```

---

## TCP Sockets

### Client

```
conn = SocketConnect("127.0.0.1", 9000)
Print "Connected, handle: " + ToString(conn)

SocketSetTimeout(conn, 2000)
SocketSend(conn, "Hello from LANGUAGE!\n")

response = SocketReceive(conn)
Print "Got: " + response

line = SocketReceiveLine(conn)
Print "Line: " + line

Print "Valid: " + ToString(SocketIsValid(conn))
SocketClose(conn)
Print "Valid after close: " + ToString(SocketIsValid(conn))
```

### Server

```
server = SocketListen("0.0.0.0", 9000)
Print "Listening..."

client = SocketAccept(server)
Print "Client connected!"

msg = SocketReceiveLine(client)
Print "Received: " + msg

SocketSend(client, "ACK\n")
SocketClose(client)
SocketClose(server)
```

| Function                    | Description                              |
|-----------------------------|------------------------------------------|
| `SocketConnect(host, port)` | Connect to server, returns handle        |
| `SocketListen(host, port)`  | Start server, returns handle             |
| `SocketAccept(handle)`      | Accept incoming connection               |
| `SocketSend(handle, msg)`   | Send a string                            |
| `SocketReceive(handle)`     | Receive up to 4096 bytes                 |
| `SocketReceiveLine(handle)` | Receive one line (stops at `\n`)         |
| `SocketClose(handle)`       | Close connection                         |
| `SocketIsValid(handle)`     | Check if handle is still open            |
| `SocketSetTimeout(h, ms)`   | Set receive timeout in milliseconds      |

---

## Importing Files

```
Import "mathlib.LANGUAGE"         # same directory
Import "libs/utils.LANGUAGE"      # subdirectory
Import "/home/user/lib.LANGUAGE"  # absolute path
```

Paths resolve relative to the **importing file's directory**. Duplicate imports are silently ignored.

---

## Indentation Rules

Blocks use **2 spaces** per level, closed with `End`.

```
If x > 0
  Print "positive"
  If x > 10
    Print "big"
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

# Output & Input
Print "Hello " + name
answer = Input("Enter: ")

# Arithmetic
Print 10 + 3 / 10 - 3 * 10

# String ops
Print Length("hi")
Print Upper("hi")
Print Lower("HI")
Print Substring("hello", 1, 3)
Print Contains("hello", "ell")

# Basic math
Print Floor(3.9) / Ceil(3.1) / Round(3.5)
Print Sqrt(25) / Abs(-7) / Power(2,8)
Print Mod(10,3) / Min(3,7) / Max(3,7)
Print Clamp(15,0,10) / Lerp(0,100,0.5)

# Trig
Print Sin(0) / Cos(0) / Tan(0)
Print Deg2Rad(180) / Rad2Deg(3.14159)

# Logs
Print Log(2.71828) / Log10(100) / Log2(8) / Exp(1)

# Number theory
Print Factorial(5) / IsPrime(17)
Print GCD(12,8) / LCM(4,6) / RandomInt(1,10)

# Extra math
Print Sign(-5) / Truncate(3.9) / Frac(3.75)
Print Hypot(3,4) / Cbrt(27) / CopySign(5,-1)

# Checks
Print IsNaN(0) / IsInf(1) / IsEven(4) / IsOdd(7)

# Bitwise
Print BitAnd(12,10) / BitOr(12,10) / BitXor(12,10)
Print BitShiftLeft(1,4) / BitShiftRight(16,2)

# Statistics
arr = [1,2,3,4,5]
Print Sum(arr) / Mean(arr) / Median(arr)
Print StdDev(arr) / Variance(arr) / Product(arr)

# Pure math
Print Gamma(5) / Erf(1) / Erfc(1) / Beta(2,3)

# Type conversion
Print ToNumber("99") / ToString(3.14)

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
Push(nums, 4) / Pop(nums)
Print nums[0] / Length(nums)

# Functions
Func square(n)
  Return n * n
End

# File I/O
WriteFile("f.txt", "hello")
AppendFile("f.txt", " world")
Print ReadFile("f.txt")

# Error handling
Try
  x = 1 / 0
Catch(err)
  Print err
End

# TCP sockets
conn = SocketConnect("127.0.0.1", 9000)
SocketSend(conn, "hello\n")
data = SocketReceive(conn)
SocketClose(conn)

# Import
Import "mylib.LANGUAGE"
```
