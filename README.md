# LANGUAGE

A simple programming language built one feature at a time.

## About

LANGUAGE is a minimalist, whitespace-sensitive programming language created as an experiment. It follows Python-like indentation rules and uses PascalCase for keywords.

**Current Version:** 0.2.0 (Day 2)

## Quick Start

```language
x = 10
y = 5

If x > y
  Print x
End
```

## Syntax

### Variables
Variables store numbers and don't need declaration:
```language
x = 42
pi = 3.14
```

### Operators

**Arithmetic:** `+` `-` `*` `/`
```language
result = 10 + 5 * 2
```

**Comparison:** `==` `!=` `<` `>` `<=` `>=`
```language
If x == 10
  Print 1
End
```

### Control Flow

**If Statements**
```language
If condition
  Print 1
  x = 10
End
```

- Must be followed by indented block (2 spaces)
- Closed with `End` keyword

### Built-in Commands

**Print** - Output a value
```language
Print 42
Print x + y
```

## Language Rules

- **Case-sensitive**: `x`, `X`, and `Print` are all different
- **Indentation**: Use 2 spaces per indentation level
- **Keywords**: Use PascalCase (`Print`, `If`, `End`)
- **Comments**: Not yet implemented (coming in Day 5)

## Installation

### Linux (Fedora/RHEL)
Download the `.rpm` from [releases](https://github.com/yourusername/LANGUAGE/releases) and install:
```bash
sudo dnf install ./LANGUAGE-0.2.0-1.fc43.x86_64.rpm
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

**Run:**
```bash
./LANGUAGE script.LANGUAGE
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
- **Day 3:** While loops
- **Day 4:** Functions
- **Day 5:** Comments & Else statements
- **Day 6:** Strings

## Examples

### Basic arithmetic
```language
x = 5
y = 10
z = x + y * 2
Print z
```

### Conditionals
```language
age = 20

If age >= 18
  Print 1
End

If age < 21
  Print 2
End
```

## Contributing

This is an experimental project. Feel free to explore the code and see how a programming language is built step by step!

## License

MIT
