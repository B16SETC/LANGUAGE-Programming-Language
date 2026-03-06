# LANGUAGE

A programming language built from scratch in C++.

> Started as a joke. Became real.

---

## Features

- Numbers, strings, booleans, arrays, dictionaries, null
- String interpolation and multiline strings
- If / Elif / Else, While, For loops
- Functions with default parameters
- Recursion, Try / Catch error handling
- File I/O
- Full math library (70+ functions)
- Complete networking stack — DNS, TCP, UDP, HTTP client, HTTP server, WebSockets
- JSON parsing and stringification
- LANGPACK system — extend LANGUAGE with native C++ packages
- Package manager — `--install`, `--uninstall`, `--list`, `--search`
- Cross-platform — Linux and Windows

---

## Installation

Download the latest binary from [Releases](https://github.com/B16SETC/LANGUAGE-Programming-Language/releases).

**Linux:**
```bash
chmod +x LANGUAGE
./LANGUAGE script.LANGUAGE
```

**Windows:**
```
LANGUAGE.exe script.LANGUAGE
```

---

## Quick Start

```
# Hello World
Print "Hello, World!"

# Variables
name = "James"
age = 21
Print "Hello, {name}! You are {age} years old."

# Functions
Func Greet(name, greeting = "Hello")
  Return "{greeting}, {name}!"
End

Print Greet("James")
Print Greet("James", "Hey")

# Dictionaries
person = {"name": "James", "lang": "LANGUAGE"}
Print person["name"]
person["version"] = 2
Print ToString(DictKeys(person))

# JSON
data = {"score": 100, "passed": True}
json = JsonStringify(data)
Print json
parsed = JsonParse(json)
Print ToString(parsed["score"])
```

---

## Language Reference

### Types

| Type | Example |
|------|---------|
| Number | `x = 42` |
| String | `s = "hello"` |
| Boolean | `b = True` or `b = False` |
| Array | `arr = [1, 2, 3]` |
| Dictionary | `d = {"key": "value"}` |
| Null | `x = Null` |

### Strings

```
# Interpolation
name = "James"
Print "Hello, {name}!"
Print "2 + 2 = {2 + 2}"
Print "Escaped: {{not interpolated}}"

# Multiline (backtick)
html = `<html>
  <body>Hello</body>
</html>`
Print html
```

### Control Flow

```
# If / Elif / Else
If x > 10
  Print "big"
Elif x > 5
  Print "medium"
Else
  Print "small"
End

# While
While x > 0
  x = x - 1
End

# For
For i = 1 To 10
  Print ToString(i)
End

# Break / Continue
For i = 1 To 10
  If i == 5
    Break
  End
  If Mod(i, 2) == 0
    Continue
  End
  Print ToString(i)
End
```

### Functions

```
Func Add(a, b)
  Return a + b
End

# Default parameters
Func Greet(name, greeting = "Hello")
  Return "{greeting}, {name}!"
End

# Recursion
Func Factorial(n)
  If n <= 1
    Return 1
  End
  Return n * Factorial(n - 1)
End
```

### Arrays

```
arr = [1, 2, 3, 4, 5]
Print arr[0]
arr[0] = 99
Push(arr, 6)
Pop(arr)
Print Length(arr)
Print ToString(Sort(arr))
Print ToString(Reverse(arr))
Print ToString(Contains(arr, 3))
Print ToString(IndexOf(arr, 3))
```

### Dictionaries

```
d = {"name": "James", "age": 21}
Print d["name"]
d["age"] = 22
d["country"] = "Malta"

Print ToString(DictKeys(d))
Print ToString(DictValues(d))
Print ToString(DictHas(d, "name"))
Print ToString(DictSize(d))
DictRemove(d, "country")

d2 = {"age": 99, "lang": "LANGUAGE"}
Print ToString(DictMerge(d, d2))
```

### JSON

```
data = {"name": "LANGUAGE", "version": 2, "cool": True, "nothing": Null}
json = JsonStringify(data)
Print json

parsed = JsonParse(json)
Print parsed["name"]
Print ToString(parsed["version"])
```

### Error Handling

```
Try
  result = 1 / 0
Catch(err)
  Print "Error: " + err
End
```

### File I/O

```
WriteFile("output.txt", "Hello!")
content = ReadFile("output.txt")
Print content
AppendFile("output.txt", " More text.")
```

### Comments

```
# This is a comment #
```

### Imports

```
# Import a LANGUAGE file
Import "utils.LANGUAGE"

# Import a LANGPACK
Import LANGQT
```

---

## Networking

### DNS

```
ip = DnsResolve("example.com")
all = DnsResolveAll("example.com")
ipv6 = DnsResolveIPv6("example.com")
host = DnsReverse("1.2.3.4")
```

### TCP

```
# Client
sock = SocketConnect("127.0.0.1", 8080)
SocketSend(sock, "Hello!")
response = SocketReceive(sock)
SocketClose(sock)

# Server
server = SocketListen("0.0.0.0", 8080)
client = SocketAccept(server)
msg = SocketReceive(client)
SocketSend(client, "Got it!")
SocketClose(client)
SocketClose(server)
```

### UDP

```
# Send
sock = UdpCreate(0)
UdpSend(sock, "127.0.0.1", 9000, "Hello UDP!")
UdpClose(sock)

# Receive
sock = UdpCreate(9000)
UdpSetTimeout(sock, 2000)
msg = UdpReceive(sock)
pkt = UdpReceiveFull(sock)
Print pkt["data"]
Print pkt["ip"]
Print ToString(pkt["port"])
UdpClose(sock)

# Broadcast
sock = UdpCreate(0)
UdpBroadcast(sock, 9000, "Hello everyone!")
UdpClose(sock)
```

### HTTP Client
*Requires `-DUSE_CURL=ON`*

```
# Basic requests
body = HttpGet("https://api.example.com/data")
body = HttpPost("https://api.example.com/submit", "key=value")
body = HttpPut("https://api.example.com/item/1", "data")
body = HttpDelete("https://api.example.com/item/1")

# JSON helpers
data = HttpGetJson("https://api.example.com/json")
Print data["name"]

payload = {"key": "value", "number": 42}
response = HttpPostJson("https://api.example.com/json", payload)

# Full response with status and headers
full = HttpGetFull("https://api.example.com/data")
Print full["body"]
Print ToString(full["status"])
Print full["headers"]

# Download a file
bytes = HttpDownload("https://example.com/file.zip", "file.zip")
Print "Downloaded: " + ToString(bytes) + " bytes"

# Custom request
body = HttpRequest("https://api.example.com", "PATCH", "{\"key\":\"val\"}", "Authorization: Bearer token123")
```

### HTTP Server

```
server = HttpServerCreate("0.0.0.0", 8080)
Print "Server running on port 8080"

While True
  conn = HttpServerAccept(server)

  method = HttpRequestMethod(conn)
  path = HttpRequestPath(conn)
  ip = HttpRequestIP(conn)

  If path == "/api/hello"
    HttpRespondJson(conn, 200, {"message": "Hello!", "from": "LANGUAGE"})
  Elif path == "/"
    HttpRespondFile(conn, 200, "index.html")
  Elif path == "/redirect"
    HttpRespondRedirect(conn, "/api/hello")
  Else
    HttpRespondJson(conn, 404, {"error": "Not found"})
  End
End
```

### WebSockets
*Requires `-DUSE_WEBSOCKETS=ON`*

```
ws = WsConnect("ws://localhost:8080/chat")
WsSend(ws, "Hello!")
msg = WsReceive(ws)
Print msg
WsClose(ws)
```

---

## Math Library

```
Floor, Ceil, Round, Abs, Sqrt, Power, Mod
Sin, Cos, Tan, Asin, Acos, Atan, Atan2
Log, Log10, Log2, Exp, Factorial, GCD, LCM
Mean, Median, StdDev, Variance, Sum, Product
Min, Max, Clamp, Lerp, Sign, Hypot, Cbrt
Erf, Erfc, Gamma, Beta, IsNaN, IsInf
BitAnd, BitOr, BitXor, BitNot, BitShiftLeft, BitShiftRight
```

---

## Type Checks

```
IsNull(x)
IsNumber(x)
IsString(x)
IsBool(x)
IsArray(x)
IsDict(x)
```

---

## Package Manager

```bash
LANGUAGE --install PACKAGENAME      # install a package
LANGUAGE --uninstall PACKAGENAME    # uninstall a package
LANGUAGE --list                     # list installed packages
LANGUAGE --search                   # browse available packages
LANGUAGE --search qt                # search by name
```

Packages are installed to:
- **Linux:** `~/.language/packages/`
- **Windows:** `%APPDATA%\LANGUAGE\packages\`

---

## LANGPACK Development

LANGPACKs extend LANGUAGE with native C++ code. They are compiled shared libraries (`.so` / `.dll`) renamed to `.langpack`.

**1. Download `language_api.h` from this repo**

**2. Write your LANGPACK:**

```cpp
#include "language_api.h"

LangValue* my_cube(LangArgs* args) {
    double x = lang_to_number(lang_arg(args, 0));
    return lang_number(x * x * x);
}

LANGPACK_EXPORT const char* langpack_name()    { return "MYPACK"; }
LANGPACK_EXPORT const char* langpack_version() { return "1.0.0"; }
LANGPACK_EXPORT const char* langpack_author()  { return "YourName"; }

LANGPACK_EXPORT void langpack_register(LangInterp* interp) {
    lang_register(interp, "MyCube", my_cube);
}
```

**3. Compile:**

```bash
# Linux
g++ -std=c++17 -shared -fPIC -o MYPACK.langpack mypack.cpp

# Windows (MinGW)
g++ -std=c++17 -shared -o MYPACK.langpack mypack.cpp
```

**4. Use:**
```
Import MYPACK
Print ToString(MyCube(3))   # 27
```

**5. Publish:** Create a GitHub repo, upload releases for Linux and Windows, then open an issue or contact the maintainer to get it added to the registry.

---

## Building from Source

```bash
git clone https://github.com/B16SETC/LANGUAGE-Programming-Language
cd LANGUAGE-Programming-Language
mkdir build && cd build

# Basic build
cmake ..
make

# With HTTP/HTTPS support
cmake -DUSE_CURL=ON ..
make

# With WebSocket support
cmake -DUSE_CURL=ON -DUSE_WEBSOCKETS=ON ..
make
```

**Dependencies (optional):**
```bash
# Ubuntu/Debian
sudo apt install libcurl4-openssl-dev libwebsockets-dev

# Arch
sudo pacman -S curl libwebsockets
```

---

## CLI

```
LANGUAGE <script.LANGUAGE>     Run a script
LANGUAGE --help                Show help
LANGUAGE --version             Show version
LANGUAGE --update              Update to latest release
LANGUAGE --install <pkg>       Install a package
LANGUAGE --uninstall <pkg>     Uninstall a package
LANGUAGE --list                List installed packages
LANGUAGE --search [query]      Search available packages
```

---

## Indentation

Use **2 spaces** per level. Tabs are not supported.

---

## License

MIT
