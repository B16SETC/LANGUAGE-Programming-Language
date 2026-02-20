# LANGUAGE

A minimalist programming language built one feature at a time.

## About

LANGUAGE is a whitespace-sensitive programming language with PascalCase keywords and Python-like indentation. Simple to learn, with an ambitious roadmap.

**Current Version:** 1.1.0 (Day 7) 🔥

---

## Quick Start

```language
Func greet(name)
  Print "Hello " + name
End

For i = 1 To 3
  greet("World")
End
```

---

## Learning LANGUAGE

### If you've never programmed before

Welcome! Programming is just giving instructions to a computer. LANGUAGE is designed to be as readable as possible.

**Step 1 - Your first program:**
```language
Print "Hello World"
```
That's it. `Print` outputs text. Simple.

**Step 2 - Variables (storing values):**
```language
name = "Alice"
age = 25
Print name
Print age
```
A variable is just a named box that holds a value.

**Step 3 - Making decisions:**
```language
x = 10
If x > 5
  Print "x is big"
Else
  Print "x is small"
End
```
`If` checks a condition. If it's true, run the block. `End` closes the block.

**Step 4 - Repeating things:**
```language
For i = 1 To 5
  Print i
End
```
This prints 1, 2, 3, 4, 5. Loops repeat code automatically.

**Step 5 - Functions (reusable code):**
```language
Func add(a, b)
  Return a + b
End

result = add(3, 4)
Print result
```
Functions are named blocks you can call over and over.

---

### If you already know another language

LANGUAGE uses PascalCase keywords and indentation-based blocks (no braces, no semicolons).

| Feature | LANGUAGE | Python equivalent |
|---|---|---|
| Print | `Print x` | `print(x)` |
| If/Else | `If x > 5 ... Else ... End` | `if x > 5: ... else: ...` |
| While | `While x > 0 ... End` | `while x > 0: ...` |
| For | `For i = 1 To 10 ... End` | `for i in range(1, 11): ...` |
| Function | `Func name(a, b) ... End` | `def name(a, b): ...` |
| Comments | `# comment #` | `# comment` |

---

## Full Syntax Reference

### Variables
```language
x = 42
name = "Alice"
flag = True
nums = [1, 2, 3]
```

### Operators
**Arithmetic:** `+` `-` `*` `/`
**Comparison:** `==` `!=` `<` `>` `<=` `>=`
**Logical:** `And` `Or` `Not`
**String concat:** `+`

### Control Flow
```language
If x < 10
  Print "small"
Elif x < 100
  Print "medium"
Else
  Print "large"
End
```

### Loops
```language
While x > 0
  Print x
  x = x - 1
End

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
If a And Not b
  Print "works"
End
```

### User Input
```language
name = Input("What is your name? ")
Print "Hello " + name
```

### Type Conversion
```language
num = ToNumber("42")
str = ToString(100)
Print num + 1
Print str + " items"
```

### Math Built-ins
```language
Print Floor(4.9)
Print Ceil(4.1)
Print Sqrt(16)
Print Abs(-7)
Print Power(2, 8)
```

### File I/O
```language
WriteFile("hello.txt", "Hello World")
contents = ReadFile("hello.txt")
Print contents
AppendFile("hello.txt", " - appended")
```

### Error Handling
```language
Try
  result = ToNumber("oops")
Catch(err)
  Print "Error: " + err
End
```

### Comments
`#` toggles comment mode on and off:
```language
# This is a comment #
x = 5  # inline comment # y = 10
```

---

## Built-in Reference

| Function | Description | Example |
|---|---|---|
| `Print` | Output a value | `Print "hello"` |
| `Input(prompt)` | Read user input | `name = Input("Name: ")` |
| `Length(x)` | Length of string or array | `Length("hi")` → 2 |
| `Upper(s)` | Uppercase string | `Upper("hi")` → "HI" |
| `Lower(s)` | Lowercase string | `Lower("HI")` → "hi" |
| `Contains(s, sub)` | Check if string contains sub | `Contains("hello", "ell")` → True |
| `Substring(s, start, len)` | Slice a string | `Substring("hello", 1, 3)` → "ell" |
| `Push(arr, val)` | Add to array | `Push(nums, 99)` |
| `Pop(arr)` | Remove last from array | `Pop(nums)` |
| `ToNumber(x)` | Convert to number | `ToNumber("42")` → 42 |
| `ToString(x)` | Convert to string | `ToString(42)` → "42" |
| `Floor(n)` | Round down | `Floor(4.9)` → 4 |
| `Ceil(n)` | Round up | `Ceil(4.1)` → 5 |
| `Sqrt(n)` | Square root | `Sqrt(16)` → 4 |
| `Abs(n)` | Absolute value | `Abs(-7)` → 7 |
| `Power(n, exp)` | Exponentiation | `Power(2, 8)` → 256 |
| `ReadFile(path)` | Read file contents | `ReadFile("file.txt")` |
| `WriteFile(path, content)` | Write to file | `WriteFile("out.txt", "hi")` |
| `AppendFile(path, content)` | Append to file | `AppendFile("out.txt", "!")` |

---

## Example Programs

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

### User Greeter
```language
name = Input("Enter your name: ")
Print "Hello " + name + "!"
```

### Safe Division
```language
Func divide(a, b)
  Try
    If b == 0
      Print "Cannot divide by zero"
    Else
      Return a / b
    End
  Catch(err)
    Print "Error: " + err
  End
End

Print divide(10, 2)
Print divide(10, 0)
```

### Array Sum
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

---

## Installation

### Linux (Fedora/RHEL)
```bash
sudo dnf install ./LANGUAGE-1.0.0-Linux-x86_x64.rpm
```

### Windows
Run `LANGUAGE-1.0.0-Setup.exe`

### Build from Source
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

---

## Language Rules

- **Case-sensitive**: `x` and `X` are different variables
- **Indentation**: 2 spaces per block level
- **Keywords**: PascalCase (`Print`, `If`, `Elif`, `Else`, `While`, `For`, `To`, `Func`, `Return`, `End`, `And`, `Or`, `Not`, `True`, `False`, `Try`, `Catch`, `Input`)

---

## Project Structure
```
src/
  main.cpp        - Entry point and CLI
  lexer.cpp       - Tokenization
  parser.cpp      - AST generation
  interpreter.cpp - Execution
```

---

## Roadmap

- ✅ **Day 1:** Variables, arithmetic, Print
- ✅ **Day 2:** If statements, comparisons
- ✅ **Day 3:** While loops
- ✅ **Day 4:** Functions, Else, comments, strings & string operations
- ✅ **Day 5:** For loops, arrays, Elif, booleans, logical operators
- ✅ **Day 6:** User Input, type conversion, math built-ins, file I/O, Try/Catch → **v1.0.0**
- **Day 7:** Modules & Import system
- **Day 8+:** Networking, UI, compiler, game engine...

---

## VS Code Extension

Install the [LANGUAGE Extension](https://marketplace.visualstudio.com/items?itemName=James6769.LANGUAGE-Programming-Language-Extension) for syntax highlighting.

---

## License

MIT
