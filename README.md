# LANGUAGE Programming Language

LANGUAGE is a programming language built from scratch in C++. It features variables, arithmetic, strings, arrays, dictionaries, null, JSON, string interpolation, multiline strings, functions with default parameters, control flow, recursion, error handling, file I/O, a full math library, networking (DNS, TCP, UDP, HTTP, WebSockets), and a native package system (LANGPACK).

---

## Table of Contents

- [Running a Script](#running-a-script)
- [Comments](#comments)
- [Variables](#variables)
- [Data Types](#data-types)
- [Arithmetic](#arithmetic)
- [String Operations](#string-operations)
- [String Interpolation](#string-interpolation)
- [Multiline Strings](#multiline-strings)
- [Math Built-ins](#math-built-ins)
- [Type Conversion](#type-conversion)
- [Type Checks](#type-checks)
- [Booleans & Logical Operators](#booleans--logical-operators)
- [Null](#null)
- [If / Elif / Else](#if--elif--else)
- [While Loops](#while-loops)
- [For Loops](#for-loops)
- [Break & Continue](#break--continue)
- [Arrays](#arrays)
- [Dictionaries](#dictionaries)
- [JSON](#json)
- [Functions](#functions)
- [User Input](#user-input)
- [File I/O](#file-io)
- [Error Handling](#error-handling)
- [Importing Files](#importing-files)
- [DNS](#dns)
- [TCP Sockets](#tcp-sockets)
- [UDP](#udp)
- [HTTP Client](#http-client)
- [HTTP Server](#http-server)
- [WebSockets](#websockets)

---

## Running a Script

Save your code in a file with the `.LANGUAGE` extension and run it:

```bash
LANGUAGE myscript.LANGUAGE
```

Other options:

```bash
LANGUAGE --version        # Show version
LANGUAGE --help           # Show usage info
LANGUAGE --update         # Update to latest release
LANGUAGE --install <pkg>  # Install a LANGPACK
LANGUAGE --uninstall <pkg># Uninstall a LANGPACK
LANGUAGE --list           # List installed LANGPACKs
LANGUAGE --search [query] # Search available LANGPACKs
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
nothing = Null
```

Variable names are case-sensitive and can contain letters, numbers, and underscores, but must start with a letter or underscore.

---

## Data Types

| Type        | Example                      |
|-------------|------------------------------|
| Number      | `42`, `3.14`, `-7`           |
| String      | `"Hello, World!"`            |
| Boolean     | `True`, `False`              |
| Array       | `[1, 2, 3]`                  |
| Dictionary  | `{"key": "value"}`           |
| Null        | `Null`                       |

---

## Arithmetic

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

```
s = "Hello, LANGUAGE!"

Print Length(s)               # 16
Print Upper(s)                # HELLO, LANGUAGE!
Print Lower(s)                # hello, language!
Print Substring(s, 7, 8)      # LANGUAGE
Print Contains(s, "LANG")     # True
Print StartsWith(s, "Hello")  # True
Print EndsWith(s, "!")        # True
Print Replace(s, "LANGUAGE", "World")  # Hello, World!
Print Trim("  spaces  ")      # spaces
Print ToString(42)            # 42
```

### Substring

`Substring(string, startIndex, length)` — extracts `length` characters starting at `startIndex` (zero-based).

```
Print Substring("abcdef", 2, 3)   # cde
```

### Contains

`Contains(string, search)` — returns `True` if found, `False` if not. Also works on arrays.

---

## String Interpolation

Embed any expression directly inside a string using `{}`:

```
name = "James"
age = 21
Print "Hello, {name}!"
Print "You are {age} years old."
Print "2 + 2 = {2 + 2}"
Print "Power: {Power(2, 8)}"
```

Escape curly braces with `{{` and `}}`:

```
Print "Escaped: {{not interpolated}}"   # Escaped: {not interpolated}
```

---

## Multiline Strings

Use backticks to write strings that span multiple lines:

```
msg = `Hello,
this spans
multiple lines.`
Print msg
```

---

## Math Built-ins

```
Print Floor(3.7)        # 3
Print Ceil(3.2)         # 4
Print Round(3.5)        # 4
Print Sqrt(144)         # 12
Print Abs(-99)          # 99
Print Power(2, 10)      # 1024
Print Mod(17, 5)        # 2
Print Sin(0)            # 0
Print Cos(0)            # 1
Print Tan(0)            # 0
Print Asin(1)
Print Acos(1)
Print Atan(1)
Print Atan2(1, 1)
Print Log(1)            # 0
Print Log10(100)        # 2
Print Log2(8)           # 3
Print Exp(1)
Print GCD(48, 18)       # 6
Print LCM(4, 6)         # 12
Print Clamp(15, 0, 10)  # 10
Print Lerp(0, 100, 0.5) # 50
Print Mean([1,2,3,4,5]) # 3
Print Median([1,2,3])   # 2
Print StdDev([1,2,3,4,5])
Print Variance([1,2,3,4,5])
Print Sum([1,2,3])      # 6
Print Product([1,2,3])  # 6
Print Min(3, 7)         # 3
Print Max(3, 7)         # 7
Print Sign(-5)          # -1
Print Hypot(3, 4)       # 5
Print Cbrt(27)          # 3
Print BitAnd(12, 10)
Print BitOr(12, 10)
Print BitXor(12, 10)
Print BitNot(12)
Print BitShiftLeft(1, 4)
Print BitShiftRight(16, 2)
Print IsNaN(x)
Print IsInf(x)
```

---

## Type Conversion

```
numStr = "42"
num = ToNumber(numStr)
Print num + 8           # 50

n = 3.14
Print ToString(n)       # 3.14
```

| Function       | Description                         |
|----------------|-------------------------------------|
| `ToNumber(x)`  | Convert string or boolean to number |
| `ToString(x)`  | Convert any value to a string       |

---

## Type Checks

```
Print IsNull(x)     # True if x is Null
Print IsNumber(x)   # True if x is a number
Print IsString(x)   # True if x is a string
Print IsBool(x)     # True if x is a boolean
Print IsArray(x)    # True if x is an array
Print IsDict(x)     # True if x is a dictionary
```

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

## Null

`Null` represents the absence of a value.

```
x = Null
Print IsNull(x)    # True

If x == Null
  Print "x is null"
End

If x != 42
  Print "x is not 42"
End
```

---

## If / Elif / Else

Blocks are closed with `End`.

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

`For` loops iterate over a range of numbers, inclusive on both ends.

```
For i = 1 To 5
  Print i
End
```

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
For i = 1 To 10
  If i == 5
    Break
  End
  If Mod(i, 2) == 0
    Continue
  End
  Print i
End
```

Both work inside `While` and `For` loops.

---

## Arrays

Arrays are ordered lists of values and can hold any mix of types.

```
fruits = ["Apple", "Banana", "Cherry"]
nums = [10, 20, 30]
mixed = [1, "hello", True]
```

### Accessing & Modifying

```
Print fruits[0]      # Apple
fruits[1] = "Blueberry"
```

### Built-ins

```
Push(fruits, "Dragonfruit")
Pop(fruits)
Print Length(fruits)
Print Sort(nums)
Print Reverse(nums)
Print Contains(fruits, "Apple")
Print IndexOf(fruits, "Apple")
```

### Iterating

```
For i = 0 To Length(fruits) - 1
  Print fruits[i]
End
```

---

## Dictionaries

Dictionaries store key-value pairs.

```
person = {"name": "James", "age": 21}
Print person["name"]
person["age"] = 22
person["country"] = "Malta"
```

### Built-ins

```
Print DictKeys(person)
Print DictValues(person)
Print DictHas(person, "name")
Print DictSize(person)
DictRemove(person, "country")

d1 = {"a": 1, "b": 2}
d2 = {"b": 99, "c": 3}
Print DictMerge(d1, d2)   # {"a": 1, "b": 99, "c": 3}
```

---

## JSON

```
data = {"name": "James", "age": 21, "active": True, "score": Null}
json = JsonStringify(data)
Print json

parsed = JsonParse(json)
Print parsed["name"]
Print ToString(parsed["age"])
```

`JsonParse` supports objects, arrays, strings, numbers, booleans, and null.

---

## Functions

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

### Default Parameters

```
Func greet(name, greeting = "Hello")
  Return "{greeting}, {name}!"
End

Print greet("James")           # Hello, James!
Print greet("James", "Hey")    # Hey, James!
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

`Input` reads a line of text from the user.

```
name = Input("What is your name? ")
Print "Hello, " + name + "!"

ageStr = Input("Enter your age: ")
age = ToNumber(ageStr)
Print "Next year you will be " + ToString(age + 1)
```

Input always returns a string, so use `ToNumber` if you need to do math with it.

---

## File I/O

```
# Write (creates or overwrites)
WriteFile("output.txt", "Hello from LANGUAGE!")

# Read
contents = ReadFile("output.txt")
Print contents

# Append
AppendFile("output.txt", " More text.")
```

---

## Error Handling

```
Try
  result = 10 / 0
Catch(err)
  Print "Error caught: " + err
End

Try
  data = ReadFile("missing.txt")
Catch(err)
  Print "Could not read file: " + err
End
```

---

## Importing Files

```
# Same directory
Import "mathlib.LANGUAGE"

# Subdirectory
Import "libs/stringlib.LANGUAGE"

# Absolute path
Import "/home/james/mylibs/utils.LANGUAGE"

# LANGPACK (native C++ extension)
Import MYPACK
```

Paths are resolved relative to the directory of the file doing the importing. Importing the same file more than once is safe — LANGUAGE skips duplicates.

---

## DNS

```
ip   = DnsResolve("example.com")
all  = DnsResolveAll("example.com")
ipv6 = DnsResolveIPv6("example.com")
host = DnsReverse("1.2.3.4")
```

---

## TCP Sockets

```
# Client
sock = SocketConnect("127.0.0.1", 8080)
SocketSetTimeout(sock, 5000)
SocketSend(sock, "Hello!")
response = SocketReceive(sock)
line = SocketReceiveLine(sock)
Print SocketIsValid(sock)
SocketClose(sock)

# Server
server = SocketListen("0.0.0.0", 8080)
client = SocketAccept(server)
msg = SocketReceive(client)
SocketSend(client, "Got it!")
SocketClose(client)
SocketClose(server)
```

---

## UDP

```
# Send
sock = UdpCreate(0)
UdpSend(sock, "127.0.0.1", 9000, "Hello!")
UdpClose(sock)

# Receive
sock = UdpCreate(9000)
UdpSetTimeout(sock, 2000)
msg = UdpReceive(sock)
Print msg

# Receive with sender info
pkt = UdpReceiveFull(sock)
Print pkt["data"]
Print pkt["ip"]
Print ToString(pkt["port"])

# Broadcast
UdpBroadcast(sock, 9000, "Hello everyone!")
UdpClose(sock)
```

---

## HTTP Client

```
body   = HttpGet("https://api.example.com")
body   = HttpPost("https://api.example.com", "key=value")
body   = HttpPut("https://api.example.com/1", "data")
body   = HttpDelete("https://api.example.com/1")

# JSON helpers
data   = HttpGetJson("https://api.example.com/json")
Print data["name"]
result = HttpPostJson("https://api.example.com/json", {"key": "value"})

# Full response (body, status, headers)
full   = HttpGetFull("https://api.example.com")
Print full["body"]
Print ToString(full["status"])
Print full["headers"]

# With timeout (milliseconds)
body   = HttpGetWithTimeout("https://api.example.com", 5000)

# Download a file
bytes  = HttpDownload("https://example.com/file.zip", "file.zip")
Print "Downloaded: " + ToString(bytes) + " bytes"

# Custom request
body   = HttpRequest("https://api.example.com", "PATCH", "{\"key\":\"val\"}", "Authorization: Bearer token")
```

---

## HTTP Server

```
server = HttpServerCreate("0.0.0.0", 8080)
Print "Listening on port 8080"

While True
  conn   = HttpServerAccept(server)
  method = HttpRequestMethod(conn)
  path   = HttpRequestPath(conn)
  query  = HttpRequestQuery(conn)
  body   = HttpRequestBody(conn)
  ip     = HttpRequestIP(conn)

  If path == "/hello"
    HttpRespondJson(conn, 200, {"message": "Hello!"})
  Elif path == "/redirect"
    HttpRespondRedirect(conn, "/hello")
  Elif path == "/"
    HttpRespondFile(conn, 200, "index.html")
  Else
    HttpRespondJson(conn, 404, {"error": "Not found"})
  End
End
```

---

## WebSockets

```
ws   = WsConnect("ws://localhost:8080")
WsSend(ws, "Hello!")
msg  = WsReceive(ws)
line = WsReceiveLine(ws)
WsClose(ws)
```

---

## Indentation Rules

LANGUAGE is whitespace-sensitive. Blocks must be indented with **2 spaces** per level. Mixing tabs and spaces or using other indentation sizes will cause errors.

```
If x > 0
  Print "positive"       # 2 spaces
  If x > 10
    Print "big"          # 4 spaces (nested)
  End
End
```
