# HTTP Parsing Guide for Webserv

> **Your complete reference for understanding, implementing, and debugging HTTP request parsing**

---

## 📋 Table of Contents

1. [What is HTTP Parsing?](#what-is-http-parsing)
2. [The Complete HTTP Flow](#the-complete-http-flow)
3. [HTTP Request Structure](#http-request-structure)
4. [Parsing Rules & Standards](#parsing-rules--standards)
5. [Current Implementation Overview](#current-implementation-overview)
6. [Data Flow & Interfaces](#data-flow--interfaces)
7. [Step-by-Step Parsing Process](#step-by-step-parsing-process)
8. [Validation & Error Handling](#validation--error-handling)
9. [Quick Reference](#quick-reference)

---

## What is HTTP Parsing?

**HTTP Parsing** is the process of converting raw text data received from a client (browser, curl, etc.) into structured data your web server can understand and act upon.

```
┌─────────────────────────────────────────────────────────────┐
│  RAW BYTES FROM NETWORK                                     │
│  "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"      │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ PARSING
                            ▼
┌─────────────────────────────────────────────────────────────┐
│  STRUCTURED DATA                                            │
│  • Method:  "GET"                                           │
│  • URI:     "/index.html"                                   │
│  • Version: "HTTP/1.1"                                      │
│  • Headers: { "Host": "localhost" }                         │
│  • Body:    "" (empty for GET)                              │
└─────────────────────────────────────────────────────────────┘
```

**Why is it critical?**
- Without parsing, you can't know what the client wants
- Invalid parsing → crashes, security holes, wrong responses
- Proper parsing → correct routing to GET/POST/DELETE handlers

---

## The Complete HTTP Flow

```
┌──────────┐                                                    ┌──────────┐
│  CLIENT  │                                                    │  SERVER  │
│ (Browser)│                                                    │ (Webserv)│
└──────────┘                                                    └──────────┘
     │                                                                 │
     │  1. TCP Connection                                              │
     │────────────────────────────────────────────────────────────────>│
     │                                                                 │
     │  2. Send HTTP Request (Raw Bytes)                               │
     │  "POST /upload HTTP/1.1\r\n"                                    │
     │  "Content-Type: application/x-www-form-urlencoded\r\n"          │
     │  "Content-Length: 23\r\n"                                       │
     │  "\r\n"                                                         │
     │  "name=Bob&email=b@e.com"                                       │
     │────────────────────────────────────────────────────────────────>│
     │                                                                 │
     │                                   ┌─────────────────────────┐  │
     │                                   │ 3. RECEIVE (recv())     │  │
     │                                   │    Store raw bytes      │  │
     │                                   └─────────────────────────┘  │
     │                                                │                │
     │                                                ▼                │
     │                                   ┌─────────────────────────┐  │
     │                                   │ 4. PARSE REQUEST        │  │
     │                                   │    ├─ Parse Headers     │  │
     │                                   │    └─ Extract Body      │  │
     │                                   └─────────────────────────┘  │
     │                                                │                │
     │                                                ▼                │
     │                                   ┌─────────────────────────┐  │
     │                                   │ 5. VALIDATE & CHECK     │  │
     │                                   │    ├─ Method allowed?   │  │
     │                                   │    ├─ Resource exists?  │  │
     │                                   │    └─ Body size OK?     │  │
     │                                   └─────────────────────────┘  │
     │                                                │                │
     │                                                ▼                │
     │                                   ┌─────────────────────────┐  │
     │                                   │ 6. ROUTE TO HANDLER     │  │
     │                                   │    ├─ GET → GetRequest()│  │
     │                                   │    ├─ POST → PostRequest│  │
     │                                   │    └─ DELETE → Delete...│  │
     │                                   └─────────────────────────┘  │
     │                                                │                │
     │                                                ▼                │
     │  7. Send HTTP Response                        │                │
     │<────────────────────────────────────────────────────────────────│
     │  "HTTP/1.0 200 OK\r\n"                                          │
     │  "Content-Type: text/html\r\n"                                  │
     │  "Content-Length: 13\r\n"                                       │
     │  "\r\n"                                                         │
     │  "<h1>Success</h1>"                                             │
     │                                                                 │
```

**🔍 HTTP Parsing happens at steps 3-4** - This is where your code transforms raw text into usable data.

---

## HTTP Request Structure

An HTTP request has **THREE main parts**:

### 1️⃣ Request Line (First Line)

```
METHOD  URI  VERSION
  │      │      │
  ▼      ▼      ▼
GET /index.html HTTP/1.1
```

| Component | Description | Examples |
|-----------|-------------|----------|
| **METHOD** | What action to perform | `GET`, `POST`, `DELETE` |
| **URI** | Path to resource | `/`, `/upload`, `/images/cat.jpg` |
| **VERSION** | HTTP protocol version | `HTTP/1.0`, `HTTP/1.1` |

**Format:** `METHOD<SPACE>URI<SPACE>VERSION<CRLF>`

### 2️⃣ Headers (Key-Value Pairs)

```
Host: localhost:3333
Content-Type: application/x-www-form-urlencoded
Content-Length: 23
Accept: text/html
```

**Format:** `Key:<SPACE>Value<CRLF>`

**Common Headers:**
- `Host` - Server hostname (required in HTTP/1.1)
- `Content-Type` - Format of request body
- `Content-Length` - Size of body in bytes
- `Accept` - What response formats client accepts
- `User-Agent` - Client software info

### 3️⃣ Body (Optional)

```
name=Bob&email=bob@example.com
```

- **Separated from headers by:** `\r\n\r\n` (blank line)
- **Present in:** POST requests (data to send)
- **Absent in:** GET, DELETE (no data to send)

### Complete Example

```http
POST /upload HTTP/1.1\r\n
Host: localhost:3333\r\n
Content-Type: application/x-www-form-urlencoded\r\n
Content-Length: 23\r\n
\r\n
name=Bob&email=b@e.com
```

**Visual breakdown:**
```
┌────────────────────────────────────┐
│ POST /upload HTTP/1.1              │ ← Request Line
├────────────────────────────────────┤
│ Host: localhost:3333               │ ┐
│ Content-Type: application/x...     │ │← Headers
│ Content-Length: 23                 │ ┘
├────────────────────────────────────┤
│                                    │ ← Blank line (separator)
├────────────────────────────────────┤
│ name=Bob&email=b@e.com             │ ← Body
└────────────────────────────────────┘
```

---

## Parsing Rules & Standards

### 📜 RFC 1945 (HTTP/1.0) Requirements

Your webserv implements HTTP/1.0, which has specific rules:

#### Line Endings
- **Standard:** `\r\n` (CRLF - Carriage Return + Line Feed)
- **Be flexible:** Also accept `\n` (LF only) for compatibility
- **Never accept:** Just `\r` alone

```cpp
// Good parsing - handle both
size_t pos = request.find("\r\n\r\n");  // Standard
if (pos == std::string::npos)
    pos = request.find("\n\n");         // Flexible fallback
```

#### Whitespace Rules

| Location | Rule | Example |
|----------|------|---------|
| Request line | Single space separator | `GET /path HTTP/1.1` ✅<br>`GET  /path  HTTP/1.1` ❌ |
| Header value | Optional leading space | `Host: localhost` ✅<br>`Host:localhost` ✅ |
| Header value | Trim trailing spaces | `Host: localhost   ` → `localhost` |

#### Case Sensitivity

```cpp
// Header NAMES are case-INSENSITIVE
"Content-Type"  == "content-type"  == "CONTENT-TYPE"  ✅

// Header VALUES are case-SENSITIVE
"text/html" != "Text/HTML"  ⚠️

// Methods are case-SENSITIVE
"GET" != "get"  ⚠️
```

#### Required vs Optional

**Required:**
- ✅ Request line (method, URI, version)
- ✅ Host header (for HTTP/1.1, optional for 1.0)

**Optional:**
- 🔷 All other headers
- 🔷 Request body (depends on method)

### 🚨 Critical Parsing Rules

#### 1. Header/Body Separator

```
Headers\r\n
\r\n          ← This blank line is MANDATORY
Body starts here
```

**Why it matters:** Without this, you can't tell where headers end and body begins.

#### 2. Content-Length Header

```cpp
// If Content-Length exists → read exactly that many bytes for body
Content-Length: 23
→ Read 23 bytes after blank line

// If Content-Length missing → no body expected
→ Body = empty string
```

#### 3. Maximum Sizes

Protect against attacks:

```cpp
const int BUFFER_SIZE = 30720;  // ~30KB
const int MAX_BODY_SIZE = ???;  // Define based on config

// Check during parsing
if (body.size() > MAX_BODY_SIZE)
    return 413;  // Payload Too Large
```

#### 4. Invalid Characters

Reject requests with:
- `\0` (null bytes) in headers
- Invalid UTF-8 sequences
- Control characters where not allowed

---

## Current Implementation Overview

### File Organization

```
src/runservers/
├── 04_recieve_request.cpp    ← Main parsing logic
│   ├── recieveRequest()      ← Receives raw bytes via recv()
│   ├── parseRequest()        ← Orchestrates parsing
│   └── extractRequestBody()  ← Separates body from headers
│
src/parsing/
└── RequestHeader.cpp         ← Header parsing logic
    ├── parseRequest()        ← Parses request line + headers
    └── parseHeaderRequest()  ← Additional header processing

inc/
└── HttpRequest.hpp           ← Data structures
    ├── class RequestHeader   ← Stores parsed header data
    └── class HttpRequest     ← Main request handler
```

### Current Classes

#### `RequestHeader` (Temporary - marked for refactoring)

```cpp
class RequestHeader {
private:
    std::string method;                            // "GET", "POST", "DELETE"
    std::string uri;                               // "/index.html"
    std::string version;                           // "HTTP/1.1"
    std::map<std::string, std::string> headers;    // All headers
    std::string Accept;                            // Parsed Accept header
    std::string Path;                              // Parsed PATH header

public:
    void parseRequest(const std::string& request); // Main parser
    void parseHeaderRequest();                     // Additional parsing
    std::map<std::string, std::string> getHeaders() const;
    std::string getUri();
    std::string getMethod();
    // ... other getters
};
```

#### `HttpRequest` (Main class)

```cpp
class HttpRequest {
public:
    // Input data
    RequestHeader HTTPHeader;      // Parsed header info
    std::string RawRequest;        // Original raw bytes
    std::string RequestBody;       // Extracted body
    ServerConfig* Server;          // Server configuration
    int socket_fd;                 // Client socket

    // Output data (response)
    int StatusCode;                // 200, 404, 500, etc.
    std::string ContentType;       // "text/html", etc.
    std::string AnswerBody;        // Response content
    
    // Main functions
    void recieveRequest();         // Receive from socket
    void parseRequest();           // Parse received data
    void extractRequestBody();     // Separate body from headers
    void checkRequest();           // Validate parsed data
    
    // Method handlers
    void GetRequest();
    void PostRequest();
    void DeleteRequest();
};
```

---

## Data Flow & Interfaces

### Input → Parsing → Output

```
┌─────────────────────────────────────────────────────────────────┐
│                    INPUT (Network)                              │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ recv(socket_fd, buff, ...)
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│  HttpRequest::RawRequest (std::string)                          │
│  "POST /upload HTTP/1.1\r\nHost: localhost\r\n\r\nbody..."      │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ HTTPHeader.parseRequest(RawRequest)
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│  RequestHeader (object)                                         │
│  ├─ method:  "POST"                                             │
│  ├─ uri:     "/upload"                                          │
│  ├─ version: "HTTP/1.1"                                         │
│  └─ headers: {"Host": "localhost", "Content-Length": "23"}      │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ extractRequestBody()
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│  HttpRequest::RequestBody (std::string)                         │
│  "name=Bob&email=bob@example.com"                               │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ checkRequest()
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│  Validation Complete                                            │
│  ├─ AnswerType: STATIC / CGI / ERROR                            │
│  └─ StatusCode: 200 / 400 / 404 / etc.                          │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ Route to handler
                            ▼
┌──────────────┬──────────────┬──────────────┐
│ GetRequest() │PostRequest() │DeleteRequest()│
└──────────────┴──────────────┴──────────────┘
```

### Key Data Structures

```cpp
// What you receive from network
std::string RawRequest;
// Example: "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"

// What you extract from RawRequest
std::string method;      // "GET"
std::string uri;         // "/index.html"
std::string version;     // "HTTP/1.1"
std::map<std::string, std::string> headers;
// { "Host": "localhost", "Accept": "text/html" }

std::string RequestBody; // "" (empty for GET)
```

---

## Step-by-Step Parsing Process

### Phase 1: Receive Raw Data

**File:** `src/runservers/04_recieve_request.cpp`  
**Function:** `HttpRequest::recieveRequest()`

```cpp
void HttpRequest::recieveRequest()
{
    int BUFFER_SIZE = 30720;     // ~30KB buffer
    char buff[30720] = {0};      // Initialize with zeros
    
    // Read from socket (blocking)
    int bytes = recv(socket_fd, buff, BUFFER_SIZE, 0);
    
    // Store in RawRequest
    RawRequest = buff;
}
```

**Input:** File descriptor `socket_fd`  
**Output:** `RawRequest` string containing raw HTTP request  
**Error Handling:** Currently minimal - needs improvement

---

### Phase 2: Parse Headers

**File:** `src/parsing/RequestHeader.cpp`  
**Function:** `RequestHeader::parseRequest(const std::string& request)`

#### Step 2.1: Isolate Header Section

```cpp
// Find the blank line separating headers from body
std::string head;
size_t pos = request.find("\r\n\r\n");

if (pos != std::string::npos)
    head = request.substr(0, pos);      // Found CRLF separator
else {
    pos = request.find("\n\n");
    if (pos != std::string::npos)
        head = request.substr(0, pos);  // Found LF separator
    else
        head = request;                  // No body, all headers
}
```

#### Step 2.2: Parse Request Line

```cpp
std::istringstream stream(head);
std::string line;

// First line: "GET /index.html HTTP/1.1"
if (std::getline(stream, line)) {
    // Remove trailing \r if present
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    
    // Split by spaces
    std::istringstream rl(line);
    rl >> method >> uri >> version;
    // method = "GET"
    // uri = "/index.html"
    // version = "HTTP/1.1"
}
```

#### Step 2.3: Parse Header Lines

```cpp
// Remaining lines: "Key: Value"
while (std::getline(stream, line)) {
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    
    // Find colon separator
    size_t colon = line.find(':');
    if (colon != std::string::npos) {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        
        // Trim leading whitespace from value
        size_t start = value.find_first_not_of(" \t");
        if (start != std::string::npos)
            value = value.substr(start);
        
        // Store in map
        headers[key] = value;
    }
}
```

**Example:**
```
Input:  "Content-Type: text/html"
        ↓
Key:    "Content-Type"
Value:  "text/html"
        ↓
headers["Content-Type"] = "text/html"
```

---

### Phase 3: Extract Body

**File:** `src/runservers/04_recieve_request.cpp`  
**Function:** `HttpRequest::extractRequestBody()`

```cpp
void HttpRequest::extractRequestBody()
{
    // Find the blank line separator
    size_t pos = RawRequest.find("\r\n\r\n");
    
    if (pos != std::string::npos) {
        // Body starts 4 characters after separator
        RequestBody = RawRequest.substr(pos + 4);
    }
    else {
        // Try alternate separator
        pos = RawRequest.find("\n\n");
        if (pos != std::string::npos) {
            RequestBody = RawRequest.substr(pos + 2);
        }
        // else: no body, RequestBody stays empty
    }
}
```

**Visual:**
```
"POST /upload HTTP/1.1\r\nContent-Length: 5\r\n\r\nHello"
                                                 ^   ^^^^^
                                        pos+4 ───┘   └─ Body
```

---

### Phase 4: Validate & Route

**File:** `src/runservers/04_recieve_request.cpp`  
**Function:** `HttpRequest::checkRequest()`

```cpp
void HttpRequest::checkRequest()
{
    AnswerType = STATIC;  // Default
    
    // TODO: Add validation:
    // - Is method allowed for this location?
    // - Does resource exist?
    // - Is body size within limits?
    // - Are required headers present?
    
    // Set AnswerType based on checks:
    // - ERROR if validation fails
    // - CGI if resource is CGI script
    // - STATIC for normal files
}
```

**Then route in:** `src/runservers/03_handle_request.cpp`

```cpp
if (method == "GET")
    Request.GetRequest();
else if (method == "POST")
    Request.PostRequest();
else if (method == "DELETE")
    Request.DeleteRequest();
else
    StatusCode = 501;  // Not Implemented
```

---

## Validation & Error Handling

### Essential Checks

```cpp
// ✅ 1. Empty request
if (RawRequest.empty()) {
    StatusCode = 400;  // Bad Request
    return;
}

// ✅ 2. Valid request line
if (method.empty() || uri.empty() || version.empty()) {
    StatusCode = 400;  // Bad Request
    return;
}

// ✅ 3. Supported HTTP version
if (version != "HTTP/1.0" && version != "HTTP/1.1") {
    StatusCode = 505;  // HTTP Version Not Supported
    return;
}

// ✅ 4. Valid method
if (method != "GET" && method != "POST" && method != "DELETE") {
    StatusCode = 501;  // Not Implemented
    return;
}

// ✅ 5. URI not empty
if (uri.empty() || uri[0] != '/') {
    StatusCode = 400;  // Bad Request
    return;
}

// ✅ 6. Body size limit
std::map<std::string, std::string>::iterator it = headers.find("Content-Length");
if (it != headers.end()) {
    int contentLen = atoi(it->second.c_str());
    if (contentLen > MAX_BODY_SIZE) {
        StatusCode = 413;  // Payload Too Large
        return;
    }
}

// ✅ 7. Method-specific checks
if (method == "POST" && RequestBody.empty()) {
    StatusCode = 400;  // Bad Request - POST needs body
    return;
}

// ✅ 8. Host header (required for HTTP/1.1)
if (version == "HTTP/1.1" && headers.find("Host") == headers.end()) {
    StatusCode = 400;  // Bad Request
    return;
}
```

### HTTP Status Codes for Parsing Errors

| Code | Reason | When to Use |
|------|--------|-------------|
| **400** | Bad Request | Malformed request, missing required fields |
| **405** | Method Not Allowed | Valid method but not allowed for this resource |
| **411** | Length Required | POST without Content-Length |
| **413** | Payload Too Large | Body exceeds maximum size |
| **414** | URI Too Long | URI exceeds maximum length |
| **431** | Request Header Fields Too Large | Headers too big |
| **501** | Not Implemented | Valid HTTP method but not supported by server |
| **505** | HTTP Version Not Supported | Version other than 1.0/1.1 |

---

## Quick Reference

### Complete Parsing Checklist

- [ ] **Receive** raw bytes via `recv()`
- [ ] **Store** in `RawRequest` string
- [ ] **Find** header/body separator (`\r\n\r\n` or `\n\n`)
- [ ] **Parse** request line (method, URI, version)
- [ ] **Parse** headers (key: value pairs)
- [ ] **Extract** body (if present)
- [ ] **Validate** method, version, URI
- [ ] **Check** required headers
- [ ] **Verify** body size limits
- [ ] **Route** to correct handler (GET/POST/DELETE)

### Parser Function Call Chain

```
main()
  └─> start_servers()
       └─> run_main() [epoll loop]
            └─> handle_request()
                 ├─> Request.recieveRequest()      // Receive from socket
                 ├─> Request.parseRequest()        // Parse headers
                 │    ├─> HTTPHeader.parseRequest()
                 │    └─> extractRequestBody()
                 ├─> Request.checkRequest()        // Validate
                 └─> Route to method:
                      ├─> Request.GetRequest()
                      ├─> Request.PostRequest()
                      └─> Request.DeleteRequest()
```

### Common Parsing Patterns

```cpp
// Pattern 1: Find and split by delimiter
size_t pos = str.find("delimiter");
std::string before = str.substr(0, pos);
std::string after = str.substr(pos + delimiter_len);

// Pattern 2: Trim whitespace
size_t start = str.find_first_not_of(" \t");
size_t end = str.find_last_not_of(" \t");
str = str.substr(start, end - start + 1);

// Pattern 3: Parse key-value pair
size_t colon = line.find(':');
std::string key = line.substr(0, colon);
std::string value = line.substr(colon + 1);

// Pattern 4: Stream parsing
std::istringstream stream(text);
std::string line;
while (std::getline(stream, line)) {
    // Process each line
}
```

### Testing Your Parser

```bash
# Test 1: Simple GET
echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 3333

# Test 2: GET with path
curl -v http://localhost:3333/index.html

# Test 3: POST with body
curl -X POST http://localhost:3333/upload \
     -H "Content-Type: application/x-www-form-urlencoded" \
     -d "name=Test&email=test@example.com"

# Test 4: Malformed request (missing version)
echo -e "GET /\r\n\r\n" | nc localhost 3333

# Test 5: Invalid method
curl -X PATCH http://localhost:3333/  # Should return 501

# Test 6: Very long URI
curl http://localhost:3333/$(python -c 'print("a"*10000)')
```

---

## Where Parsing is Used in Your Project

```
┌─────────────────────────────────────────────────────────┐
│  FILE: 02_run_main.cpp                                  │
│  Epoll event loop - detects incoming data               │
└─────────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────┐
│  FILE: 03_handle_request.cpp                            │
│  Creates HttpRequest object, calls parsing              │
│  Request.recieveRequest()                               │
│  Request.parseRequest()                                 │
│  Request.checkRequest()                                 │
└─────────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────┐
│  FILE: 04_recieve_request.cpp                           │
│  ├─ recieveRequest()     → recv() from socket           │
│  ├─ parseRequest()       → calls RequestHeader parser   │
│  ├─ extractRequestBody() → separates body               │
│  └─ checkRequest()       → validates parsed data        │
└─────────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────┐
│  FILE: parsing/RequestHeader.cpp                        │
│  └─ parseRequest() → parses request line + headers      │
└─────────────────────────────────────────────────────────┘
```

### What Each File Does

| File | Responsibility | Input | Output |
|------|----------------|-------|--------|
| `02_run_main.cpp` | Event loop | Socket events | Calls handler |
| `03_handle_request.cpp` | Orchestration | Socket FD | Parsed request |
| `04_recieve_request.cpp` | Receiving & orchestrating parse | Raw bytes | `RawRequest`, `RequestBody` |
| `RequestHeader.cpp` | Header parsing | `RawRequest` | `method`, `uri`, `version`, `headers` |
| `07_GetMethod.cpp` | GET handler | Parsed request | Response |
| `08_PostMethod.cpp` | POST handler | Parsed request | Response |
| `09_DeleteMethod.cpp` | DELETE handler | Parsed request | Response |

---

## Next Steps for Refactoring

Based on the `// todoparsing` comments in your code:

### Recommended Changes

1. **Merge RequestHeader into HttpRequest**
   - Remove duplicate data (method, uri, version stored in both classes)
   - Simplify interface - one class instead of two
   
2. **Make members private**
   - Add proper getters/setters
   - Encapsulate data for better error checking

3. **Improve error handling**
   - Check `recv()` return value
   - Handle partial receives (need to loop until full request received)
   - Validate each parsing step

4. **Add Content-Length handling**
   - Currently doesn't check if full body received
   - Need to read exact number of bytes specified

5. **Security improvements**
   - Add max URI length check (prevent buffer overflow)
   - Add max header count check (prevent DoS)
   - Validate characters in URI (prevent injection)

### Suggested New Structure

```cpp
class HttpRequest {
private:
    // Input
    std::string rawRequest;
    int socketFd;
    
    // Parsed data
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    
    // Response
    int statusCode;
    std::string responseBody;
    
    // Helper functions
    bool parseRequestLine(const std::string& line);
    bool parseHeaders(std::istringstream& stream);
    bool extractBody();
    bool validate();
    
public:
    // Main interface
    bool receiveAndParse();  // Combines receive + parse
    void handleRequest();    // Routes to GET/POST/DELETE
    
    // Getters
    std::string getMethod() const { return method; }
    std::string getUri() const { return uri; }
    // ... more getters
};
```

---

## 🎯 Summary

**HTTP Parsing in webserv:**

1. **Receives** raw bytes from client via `recv()`
2. **Separates** headers from body using `\r\n\r\n`
3. **Parses** request line into method, URI, version
4. **Extracts** headers into key-value map
5. **Validates** all parsed data
6. **Routes** to appropriate handler (GET/POST/DELETE)

**Key Files:**
- `04_recieve_request.cpp` - Main parsing orchestration
- `RequestHeader.cpp` - Header parsing logic
- `HttpRequest.hpp` - Data structures

**Critical Rules:**
- Handle both `\r\n` and `\n` line endings
- Trim whitespace from header values
- Check Content-Length before reading body
- Validate method, URI, version
- Return appropriate error codes (400, 501, 505, etc.)

---

**Good luck with your refactoring! 🚀**

*This guide covers everything you need to understand, implement, and debug HTTP parsing in your webserv project.*
