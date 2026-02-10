# LANGUAGE Programming Language

A simple, minimalist programming language built one feature at a time.

## Version 0.1.0 - Day 1

### Features
- Variable assignment
- Arithmetic operations (+, -, *, /)
- Print statements
- Proper operator precedence

### Language Design
- **Case-sensitive**: `x`, `X`, and `Print` are all different
- **Whitespace-sensitive**: Indentation will matter for code blocks (If, While, functions)
- **Commands use PascalCase**: `Print` (not `print`)

## Syntax Guide

### Variables
Variables are dynamically assigned and store numbers:
```language
x = 10
name = 42
my_var = 3.14
```

### Arithmetic
Supports basic math with proper operator precedence (multiply/divide before add/subtract):
```language
result = 5 + 3 * 2
Print result

x = 10
y = 3
z = x / y - 1
Print z
```

Operators: `+` `-` `*` `/`

### Print
Output values to the console:
```language
Print 42
Print x + y
```

### Complete Example
```language
x = 5
y = 10
z = x + y * 2
Print z

a = 100
b = 50
c = a - b / 2
Print c
```

Output:
```
25
75
```

## Source Code Architecture

### `src/main.cpp`
Entry point of the interpreter. Handles command-line arguments (`--version`, `--help`, file execution), reads the source file, and coordinates the lexer → parser → interpreter pipeline.

### `src/lexer.h` / `src/lexer.cpp`
**Tokenization phase.** Breaks raw source code into tokens (numbers, identifiers, operators, keywords). For example, `x = 5 + 3` becomes tokens: `IDENTIFIER(x)`, `ASSIGN`, `NUMBER(5)`, `PLUS`, `NUMBER(3)`.

### `src/parser.h` / `src/parser.cpp`
**Parsing phase.** Converts tokens into an Abstract Syntax Tree (AST). Handles operator precedence (multiply before add) and builds a tree structure representing the program's logic.

### `src/interpreter.h` / `src/interpreter.cpp`
**Execution phase.** Walks the AST and executes it. Maintains a variable table, evaluates expressions, and runs statements (assignments, Print).

## Building

**Requirements:**
- CMake 3.20+
- C++17 compiler (GCC, Clang, MSVC)

**Platforms:**
- ✅ Windows (MSVC, MinGW)
- ✅ Linux (GCC, Clang)
- ✅ macOS (Clang)

**In CLion:**
1. Open the project folder
2. CLion will auto-configure CMake
3. Click the Run button (▶️)
4. To run scripts: Edit Configurations → Program arguments → add `test.LANGUAGE`

**Command line (Windows/Linux/macOS):**
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

**On Windows with Visual Studio:**
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Running

```bash
LANGUAGE script.LANGUAGE      # Run a script
LANGUAGE --version            # Show version
LANGUAGE --help               # Show help
```

## Roadmap
- Day 2: Control flow (If statements)
- Day 3: Loops (While)
- Day 4: Functions
- Day 5: Toggle comments (#) & Else statements
- Day 6: Strings



hi there! Note from a human not an AI! I came up with the programming language's name NOT the AI. It's based on an inside joke with my brother.
