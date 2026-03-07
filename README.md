# LANGUAGE

A programming language built from scratch in C++.

-----

## Hello World

```
Print "Hello, World!"
```

-----

## Variables

```
x = 42
name = "James"
b = True
nothing = Null
```

-----

## Types

|Type      |Example                 |
|----------|------------------------|
|Number    |`x = 42`                |
|String    |`s = "hello"`           |
|Boolean   |`b = True` / `b = False`|
|Array     |`arr = [1, 2, 3]`       |
|Dictionary|`d = {"key": "value"}`  |
|Null      |`x = Null`              |

-----

## Arithmetic

```
x = 10 + 3
x = 10 - 3
x = 10 * 3
x = 10 / 3
x = Power(2, 8)
x = Sqrt(144)
x = Abs(-5)
x = Mod(17, 5)
```

-----

## Strings

```
s = "Hello, World!"
Print Length(s)
Print Upper(s)
Print Lower(s)
Print Substring(s, 0, 5)
Print Replace(s, "World", "James")
Print Contains(s, "Hello")
Print StartsWith(s, "Hello")
Print EndsWith(s, "!")
Print Trim("  spaces  ")
Print ToString(42)
```

### String Interpolation

```
name = "James"
age = 21
Print "Hello, {name}!"
Print "You are {age} years old."
Print "2 + 2 = {2 + 2}"
Print "Escaped: {{not interpolated}}"
```

### Multiline Strings

```
msg = `Hello,
this spans
multiple lines.`
Print msg
```

-----

## Arrays

```
arr = [1, 2, 3, 4, 5]
Print arr[0]
arr[0] = 99
Push(arr, 6)
Pop(arr)
Print Length(arr)
Print Sort(arr)
Print Reverse(arr)
Print Contains(arr, 3)
Print IndexOf(arr, 3)
```

-----

## Dictionaries

```
person = {"name": "James", "age": 21}
Print person["name"]
person["age"] = 22
person["country"] = "Malta"

Print DictKeys(person)
Print DictValues(person)
Print DictHas(person, "name")
Print DictSize(person)
DictRemove(person, "country")

d1 = {"a": 1, "b": 2}
d2 = {"b": 99, "c": 3}
Print DictMerge(d1, d2)
```

-----

## Null

```
x = Null
Print IsNull(x)

If x == Null
  Print "x is null"
End
```

-----

## Type Checks

```
Print IsNull(x)
Print IsNumber(x)
Print IsString(x)
Print IsBool(x)
Print IsArray(x)
Print IsDict(x)
```

-----

## JSON

```
data = {"name": "James", "age": 21, "active": True}
json = JsonStringify(data)
Print json

parsed = JsonParse(json)
Print parsed["name"]
Print parsed["age"]
```

-----

## Comments

```
# This is a comment #
```

-----

## If / Elif / Else

```
score = 85

If score >= 90
  Print "A"
Elif score >= 80
  Print "B"
Elif score >= 70
  Print "C"
Else
  Print "F"
End
```

-----

## While Loop

```
i = 1
While i <= 5
  Print ToString(i)
  i = i + 1
End
```

-----

## For Loop

```
For i = 1 To 10
  Print ToString(i)
End
```

-----

## Break and Continue

```
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

-----

## Functions

```
Func Add(a, b)
  Return a + b
End

Print ToString(Add(3, 4))
```

### Default Parameters

```
Func Greet(name, greeting = "Hello")
  Return "{greeting}, {name}!"
End

Print Greet("James")
Print Greet("James", "Hey")
```

### Recursion

```
Func Factorial(n)
  If n <= 1
    Return 1
  End
  Return n * Factorial(n - 1)
End

Print ToString(Factorial(6))
```

-----

## Error Handling

```
Try
  x = 1 / 0
Catch(err)
  Print "Error: " + err
End
```

-----

## User Input

```
name = Input("What is your name? ")
Print "Hello, " + name + "!"
```

-----

## File I/O

```
WriteFile("output.txt", "Hello!")
content = ReadFile("output.txt")
Print content
AppendFile("output.txt", " More text.")
```

-----

## Imports

```
# Import another LANGUAGE file
Import "utils.LANGUAGE"

# Import a LANGPACK (native C++ extension)
Import MYPACK
```

-----

## Math Library

```
Floor(3.7)        # 3
Ceil(3.2)         # 4
Round(3.5)        # 4
Sin(0)            # 0
Cos(0)            # 1
Tan(0)            # 0
Asin(1)
Acos(1)
Atan(1)
Atan2(1, 1)
Log(1)            # 0
Log10(100)        # 2
Log2(8)           # 3
Exp(1)
GCD(48, 18)       # 6
LCM(4, 6)         # 12
Clamp(15, 0, 10)  # 10
Lerp(0, 100, 0.5) # 50
Mean([1,2,3,4,5]) # 3
Median([1,2,3])   # 2
StdDev([1,2,3])
Variance([1,2,3])
Sum([1,2,3])      # 6
Product([1,2,3])  # 6
Min(3, 7)         # 3
Max(3, 7)         # 7
Sign(-5)          # -1
Hypot(3, 4)       # 5
Cbrt(27)          # 3
BitAnd(12, 10)
BitOr(12, 10)
BitXor(12, 10)
BitNot(12)
BitShiftLeft(1, 4)
BitShiftRight(16, 2)
IsNaN(x)
IsInf(x)
```

-----

## DNS

```
ip   = DnsResolve("example.com")
all  = DnsResolveAll("example.com")
ipv6 = DnsResolveIPv6("example.com")
host = DnsReverse("1.2.3.4")
```

-----

## TCP Sockets

```
# Client
sock = SocketConnect("127.0.0.1", 8080)
SocketSend(sock, "Hello!")
response = SocketReceive(sock)
line = SocketReceiveLine(sock)
SocketSetTimeout(sock, 5000)
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

-----

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

# Receive with sender info
pkt = UdpReceiveFull(sock)
Print pkt["data"]
Print pkt["ip"]
Print ToString(pkt["port"])

# Broadcast
UdpBroadcast(sock, 9000, "Hello everyone!")
UdpClose(sock)
```

-----

## HTTP Client

```
body   = HttpGet("https://api.example.com")
body   = HttpPost("https://api.example.com", "key=value")
body   = HttpPut("https://api.example.com/1", "data")
body   = HttpDelete("https://api.example.com/1")

# JSON helpers
data   = HttpGetJson("https://api.example.com/json")
result = HttpPostJson("https://api.example.com/json", {"key": "value"})

# Full response
full   = HttpGetFull("https://api.example.com")
Print full["body"]
Print ToString(full["status"])
Print full["headers"]

# With timeout
body   = HttpGetWithTimeout("https://api.example.com", 5000)

# Download file
bytes  = HttpDownload("https://example.com/file.zip", "file.zip")

# Custom request
body   = HttpRequest("https://api.example.com", "PATCH", "{\"key\":\"val\"}", "Authorization: Bearer token")
```

-----

## HTTP Server

```
server = HttpServerCreate("0.0.0.0", 8080)

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

-----

## WebSockets

```
ws = WsConnect("ws://localhost:8080")
WsSend(ws, "Hello!")
msg  = WsReceive(ws)
line = WsReceiveLine(ws)
WsClose(ws)
```

-----

## Indentation

Use 2 spaces per level. Tabs are not supported.