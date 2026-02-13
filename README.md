# LANGUAGE

A simple programming language built one feature at a time.

## About

LANGUAGE is a minimalist, whitespace-sensitive programming language created as an experiment. It follows Python-like indentation rules and uses PascalCase for keywords.

**Current Version:** 0.4.0 (Day 4)

## Quick Start

```language
Func greet(name)
  Print "Hello " + name
End

greet("World")
```

## Syntax

### Variables
Variables store numbers or strings and don't need declaration:
```language
x = 42
name = "Alice"
```

### Operators

**Arithmetic:** `+` `-` `*` `/`
```language
result = 10 + 5 * 2
```

**String Concatenation:** `+`
```language
greeting = "Hello " + "World"
```

**Comparison:** `==` `!=` `<` `>` `<=` `>=`
```language
If x == 10
  Print "ten"
End
```

### Control Flow

**If / Else**
```language
If x > 10
  Print "big"
Else
  Print "small"
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

- Blocks must be indented with 2 spaces
- Closed with `End` keyword

### Functions
```language
Func add(a, b)
  Return a + b
End

result = add(5, 3)
Print result
```

### Comments
`#` toggles comment mode on and off:
```language
# This is a comment #
x = 5  # inline comment # y = 10
Print y

# Everything here is commented out
until the next hash closes it #
```

### Built-in Commands

**Print** - Output a value:
```language
Print 42
Print "Hello"
Print x + y
```

**String Operations:**
```language
s = "Hello World"
Print Length(s)           # prints 11 #
Print Upper(s)            # prints HELLO WORLD #
Print Lower(s)            # prints hello world #
Print Contains(s, "World") # prints 1 (true) #
Print Substring(s, 0, 5)  # prints Hello #
```

## Language Rules

- **Case-sensitive**: `x`, `X`, and `Print` are all different
- **Indentation**: Use 2 spaces per indentation level
- **Keywords**: PascalCase (`Print`, `If`, `Else`, `While`, `Func`, `Return`, `End`)
- **Types**: Numbers and strings (no declaration needed)

## Installation

### Linux (Fedora/RHEL)
Download the `.rpm` from [releases](https://github.com/yourusername/LANGUAGE/releases) and install:
```bash
sudo dnf install ./LANGUAGE-0.4.0-1.fc43.x86_64.rpm
```

### Build from Source

**Requirements:**
- CMake 3.20+
- C++17 compiler

**Build:**
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Development

### Project Structure
```
src/
  main.cpp        - Entry point and CLI
  lexer.cpp       - Tokenization
  parser.cpp      - AST generation
  interpreter.cpp - Execution
```

### Building in CLion
1. Open project folder
2. CLion auto-configures CMake
3. Click Run (▶️)
4. Edit Configurations → Program arguments → add `test.LANGUAGE`

## Roadmap

- ✅ **Day 1:** Variables, arithmetic, Print
- ✅ **Day 2:** If statements, comparisons
- ✅ **Day 3:** While loops
- ✅ **Day 4:** Functions, Else, comments, strings & string operations
- **Day 5:** For loops
- **Day 6:** Arrays
- **Day 7:** Array operations
- **Day 8:** Elif
- ...
- **Day 15:** v1.0.0

## Examples

### FizzBuzz
```language
x = 1
While x <= 20
  If x == 15
    Print "FizzBuzz"
  Else
    If x == 3
      Print "Fizz"
    Else
      Print x
    End
  End
  x = x + 1
End
```

### String manipulation
```language
name = "language"
Print Upper(name)
Print Length(name)
Print Contains(name, "ang")
Print Substring(name, 0, 4)
```

## Contributing

This is an experimental project. Feel free to explore the code and see how a programming language is built step by step!

## License

MIT
