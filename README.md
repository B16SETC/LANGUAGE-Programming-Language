# LANGUAGE

A simple programming language built one feature at a time.

## About

LANGUAGE is a minimalist, whitespace-sensitive programming language. It follows Python-like indentation rules and uses PascalCase for keywords.

**Current Version:** 0.5.0 (Day 5)

## Quick Start

```language
Func greet(name)
  Print "Hello " + name
End

For i = 1 To 3
  greet("World")
End
```

## Syntax

### Variables
Variables store numbers, strings, booleans, or arrays:
```language
x = 42
name = "Alice"
flag = True
nums = [1, 2, 3]
```

### Operators

**Arithmetic:** `+` `-` `*` `/`

**String Concatenation:** `+`
```language
greeting = "Hello " + "World"
```

**Comparison:** `==` `!=` `<` `>` `<=` `>=`

**Logical:** `And` `Or` `Not`
```language
If x > 5 And x < 100
  Print "in range"
End
```

### Control Flow

**If / Elif / Else**
```language
If x < 10
  Print "small"
Elif x < 100
  Print "medium"
Else
  Print "large"
End
```

**While Loops**
```language
x = 5
While x > 0
  Print x
  x = x - 1
End
```

**For Loops**
```language
For i = 1 To 10
  Print i
End
```

### Functions
```language
Func add(a, b)
  Return a + b
End

result = add(5, 3)
Print result
```

### Arrays
```language
nums = [10, 20, 30]
Print nums[0]
nums[1] = 99
Print Length(nums)
Push(nums, 40)
Pop(nums)
```

### Booleans
```language
a = True
b = False
Print a
If a And Not b
  Print "works"
End
```

### Comments
`#` toggles comment mode on and off:
```language
# This is a comment #
x = 5  # inline comment # y = 10
```

### Built-in Commands

**Print** - Output any value:
```language
Print 42
Print "Hello"
Print True
Print [1, 2, 3]
```

**String Operations:**
```language
s = "Hello World"
Print Length(s)
Print Upper(s)
Print Lower(s)
Print Contains(s, "World")
Print Substring(s, 0, 5)
```

**Array Operations:**
```language
arr = [1, 2, 3]
Print Length(arr)
Push(arr, 4)
Pop(arr)
Print arr[0]
arr[0] = 99
```

## Language Rules

- **Case-sensitive**: `x` and `X` are different
- **Indentation**: 2 spaces per level
- **Keywords**: PascalCase (`Print`, `If`, `Elif`, `Else`, `While`, `For`, `To`, `Func`, `Return`, `End`, `And`, `Or`, `Not`, `True`, `False`)

## Installation

### Linux (Fedora/RHEL)
```bash
sudo dnf install ./LANGUAGE-0.5.0-1.fc43.x86_64.rpm
```

### Build from Source
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Project Structure
```
src/
  main.cpp        - Entry point and CLI
  lexer.cpp       - Tokenization
  parser.cpp      - AST generation
  interpreter.cpp - Execution
```

## Roadmap

- ✅ **Day 1:** Variables, arithmetic, Print
- ✅ **Day 2:** If statements, comparisons
- ✅ **Day 3:** While loops
- ✅ **Day 4:** Functions, Else, comments, strings & string operations
- ✅ **Day 5:** For loops, arrays, array operations, Elif, booleans, logical operators
- **Day 6:** User Input, type conversion → **v1.0.0**
- **Day 7:** Math built-ins, error handling
- **Day 8:** Modules & Import
- ...

## Examples

### FizzBuzz
```language
For i = 1 To 20
  If i == 15
    Print "FizzBuzz"
  Elif i == 3
    Print "Fizz"
  Elif i == 5
    Print "Buzz"
  Else
    Print i
  End
End
```

### Array sum
```language
nums = [10, 20, 30, 40, 50]
total = 0
i = 0
While i < Length(nums)
  total = total + nums[i]
  i = i + 1
End
Print total
```

## License

MIT
