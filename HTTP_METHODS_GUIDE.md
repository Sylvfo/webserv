# HTTP Methods Implementation Guide
## POST and DELETE Methods for Webserv

---

## Table of Contents
1. [POST Method - Quick Refresher](#post-method---quick-refresher)
2. [DELETE Method - Explanation](#delete-method---explanation)
3. [Similarities Between POST and DELETE](#similarities-between-post-and-delete)
4. [DELETE Implementation Guide](#delete-implementation-guide)

---

## POST Method - Quick Refresher

### What POST Does
**Purpose:** Send data FROM client TO server (create/upload)

**Real-world examples:**
- Submitting a form (contact form, login)
- Uploading a file
- Creating a new resource

### How POST Works

```
Browser                           Webserv
   |                                 |
   | POST /upload HTTP/1.1           |
   | Content-Type: application/...   |
   | Content-Length: 123             |
   |                                 |
   | name=John&email=test@mail.com   |  ← Request Body (the data!)
   |-------------------------------->|
   |                                 |
   |        Parse form data          |
   |        Save to file             |
   |                                 |
   |   HTTP/1.0 201 Created          |
   |<--------------------------------|
   |   Data saved successfully!      |
```

### Key Components of Your POST Implementation

#### 1. Entry Point (`PostRequest()`)
```cpp
void HttpRequest::PostRequest()
{
    // Check Content-Type header
    // Determines HOW to parse the body
    
    if (ContentType == "application/x-www-form-urlencoded")
        HandleFormData();  // Simple forms
    else if (ContentType == "multipart/form-data")
        // File uploads (not implemented)
    else
        // Return 415 Unsupported Media Type
}
```

#### 2. Form Data Handler (`HandleFormData()`)
```cpp
void HttpRequest::HandleFormData()
{
    // STEP 1: Get upload path from config
    // Matches URI to location block
    uploadPath = Server.locations[i].upload_path;
    
    // STEP 2: Parse the body
    // "name=John&email=test@mail.com" → map<string, string>
    formData = parseFormData(RequestBody);
    
    // STEP 3: URL decode values
    // "Hello+World" → "Hello World"
    // "test%40mail.com" → "test@mail.com"
    
    // STEP 4: Generate unique filename
    filename = "post_" + timestamp + ".txt";
    
    // STEP 5: Write to file
    ofstream file(uploadPath + "/" + filename);
    file << "=== POST Data ===\n";
    file << "name: " << formData["name"] << "\n";
    
    // STEP 6: Return success
    StatusCode = "201 Created";
}
```

#### 3. Helper Functions

**parseFormData():** Splits `key=value&key2=value2` into map
```cpp
Input:  "name=John&email=test@mail.com"
Output: {"name": "John", "email": "test@mail.com"}
```

**urlDecode():** Converts URL encoding to normal text
```cpp
Input:  "Hello+World%21"
Output: "Hello World!"
```

**getCurrentTimestamp():** Generates unique identifier
```cpp
Output: "1733065234" (seconds since epoch)
```

### Important POST Details

**HTTP Status Codes:**
- `201 Created` - Resource successfully created ✅
- `400 Bad Request` - Malformed data
- `413 Payload Too Large` - Body exceeds limit
- `415 Unsupported Media Type` - Unknown Content-Type
- `500 Internal Server Error` - Server failure

**Security Considerations:**
- Check `client_max_body_size` from config
- Validate `Content-Length` header exists
- Only save to allowed directories

**Data Flow:**
```
Request Body → Parse → Decode → Save → Response
```

---

## DELETE Method - Explanation

### What DELETE Does
**Purpose:** Remove a resource from the server

**Real-world examples:**
- Delete uploaded file
- Remove user account
- Clear cache file

### How DELETE Works

```
Browser                           Webserv
   |                                 |
   | DELETE /uploads/file.txt HTTP/1.1
   | Host: localhost:3333            |
   |-------------------------------->|
   |                                 |
   |        Check file exists        |
   |        Verify permissions       |
   |        Delete file              |
   |                                 |
   |   HTTP/1.0 204 No Content       |
   |<--------------------------------|
   |   (empty body)                  |
```

### Key Differences from POST

| Aspect | POST | DELETE |
|--------|------|--------|
| **Direction** | Client → Server (upload) | Client tells server to remove |
| **Has Body?** | ✅ Yes (the data) | ❌ No |
| **Creates?** | Creates new resource | Removes existing resource |
| **Idempotent?** | No (2 POSTs = 2 files) | Yes (2 DELETEs = same result) |
| **Success Code** | 201 Created | 204 No Content or 200 OK |
| **Main Operation** | Write file | Delete file |
| **System Call** | `open()`, `write()` | `unlink()` |

### What "Idempotent" Means

**POST (NOT idempotent):**
```
POST /upload (file1.txt created)
POST /upload (file2.txt created)  ← Different result!
POST /upload (file3.txt created)
```

**DELETE (idempotent):**
```
DELETE /file.txt (file deleted)
DELETE /file.txt (file still deleted)  ← Same result!
DELETE /file.txt (file still deleted)
```

### The DELETE Request

**No body needed - everything is in the URI:**
```http
DELETE /uploads/post_1733065234.txt HTTP/1.1
Host: localhost:3333

[no body]
```

**What the server needs to do:**
1. Parse URI to get file path
2. Build full filesystem path
3. Check security (prevent attacks)
4. Verify file exists
5. Delete the file
6. Return appropriate status

---

## Similarities Between POST and DELETE

Both methods share these patterns:

### 1. Configuration Lookup (Same Logic!)
```cpp
// Both need to find the matching location block
std::string currentUri = HTTPHeader.getUri();

for (size_t i = 0; i < Server.locations.size(); i++)
{
    if (currentUri.find(Server.locations[i].path) == 0)
    {
        // Found matching location!
        // POST uses: upload_path
        // DELETE uses: root path
    }
}
```

### 2. Path Building (Similar!)
```cpp
// POST builds: uploadPath + "/" + filename
std::string savePath = uploadPath + "/post_123.txt";

// DELETE builds: root + uri
std::string deletePath = Server.root + "/uploads/post_123.txt";
```

### 3. Security Checks (Critical for Both!)
```cpp
// Both must prevent directory traversal attacks
if (path.find("..") != std::string::npos)
{
    StatusCode = "403 Forbidden";  // Dangerous path!
    return;
}
```

### 4. Error Handling (Same Pattern!)
```cpp
// Both check conditions and set status codes
if (/* something wrong */)
{
    StatusCode = "XXX Error Message";
    AnswerBody = "Error description";
    ContentType = "text/html";
    ContentLenght = IntToString(AnswerBody.size());
    return;
}
```

### 5. Success Response (Similar Structure!)
```cpp
// POST:
StatusCode = "201 Created";
AnswerBody = "Data saved successfully!";

// DELETE:
StatusCode = "204 No Content";
AnswerBody = "";  // No body for 204
```

---

## DELETE Implementation Guide

### Step-by-Step Implementation

#### Step 1: Understanding What We Receive

When DELETE request arrives, you have access to:
- `HTTPHeader.getUri()` → e.g., "/uploads/post_123.txt"
- `Server.root` → e.g., "/home/bastian/webserv/www"
- `Server.locations[]` → Config location blocks

**You do NOT have:**
- Request body (DELETE has no body!)
- Form data to parse

#### Step 2: Start Simple - Log What You Get

```cpp
void HttpRequest::DeleteRequest()
{
    std::cout << PASTEL_AQUA "Enter DELETE Request Handler" << RESET << std::endl;
    
    // Get the URI (path to delete)
    std::string uri = HTTPHeader.getUri();
    std::cout << "DELETE request for URI: " << uri << std::endl;
    
    // Continue to next steps...
}
```

**Test it:**
```bash
curl -X DELETE http://localhost:3333/uploads/test.txt
# Check server logs - should print the URI
```

#### Step 3: Build Full File Path

```cpp
void HttpRequest::DeleteRequest()
{
    // ... previous code ...
    
    // Build full filesystem path
    std::string filePath = Server.root + uri;
    // Example: "/home/bastian/webserv/www" + "/uploads/test.txt"
    //       → "/home/bastian/webserv/www/uploads/test.txt"
    
    std::cout << "Full path to delete: " << filePath << std::endl;
    
    // Continue to next steps...
}
```

#### Step 4: CRITICAL - Security Check

**Prevent directory traversal attacks:**

```cpp
void HttpRequest::DeleteRequest()
{
    // ... previous code ...
    
    // SECURITY: Block ".." in paths
    if (filePath.find("..") != std::string::npos)
    {
        std::cout << "Security violation: path contains .." << std::endl;
        StatusCode = "403 Forbidden";
        AnswerBody = "<html><body><h1>403 Forbidden</h1></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
        return;  // Stop here!
    }
    
    // Continue if safe...
}
```

**Why this matters:**
```
Bad request: DELETE /uploads/../../../etc/passwd
Would become: /home/bastian/webserv/www/uploads/../../../etc/passwd
Simplifies to: /etc/passwd  ← DANGER! System file!
```

#### Step 5: Check If File Exists

Use the `stat()` system call:

```cpp
void HttpRequest::DeleteRequest()
{
    // ... previous code ...
    
    // Check if file exists
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0)
    {
        // File doesn't exist
        std::cout << "File not found: " << filePath << std::endl;
        StatusCode = "404 Not Found";
        AnswerBody = "<html><body><h1>404 Not Found</h1></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
        return;
    }
    
    // Continue if exists...
}
```

**What stat() does:**
- Returns 0 if file exists
- Returns -1 if file doesn't exist
- Fills `fileStat` with file information

#### Step 6: Check It's Not a Directory

```cpp
void HttpRequest::DeleteRequest()
{
    // ... previous code ...
    
    // Don't delete directories!
    if (S_ISDIR(fileStat.st_mode))
    {
        std::cout << "Cannot delete directory" << std::endl;
        StatusCode = "403 Forbidden";
        AnswerBody = "<html><body><h1>403 Forbidden</h1><p>Cannot delete directory</p></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
        return;
    }
    
    // Continue if it's a file...
}
```

**What S_ISDIR() does:**
- Checks if the path is a directory
- Returns true for directories
- Returns false for regular files

#### Step 7: Actually Delete the File

```cpp
void HttpRequest::DeleteRequest()
{
    // ... previous code ...
    
    // Try to delete the file
    if (unlink(filePath.c_str()) == 0)
    {
        // Success!
        std::cout << "Successfully deleted: " << filePath << std::endl;
        StatusCode = "204 No Content";
        AnswerBody = "";  // 204 should have empty body
        ContentType = "text/plain";
        ContentLenght = "0";
    }
    else
    {
        // Failed to delete
        std::cout << "Failed to delete (errno: " << errno << ")" << std::endl;
        StatusCode = "500 Internal Server Error";
        AnswerBody = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
    }
}
```

**What unlink() does:**
- System call to delete a file
- Returns 0 on success
- Returns -1 on failure (check `errno` for reason)

### Complete Implementation Template

```cpp
#include "Webserv.hpp"

void HttpRequest::DeleteRequest()
{
    std::cout << PASTEL_AQUA "Enter DELETE Request Handler" << RESET << std::endl;
    
    // STEP 1: Get URI
    std::string uri = HTTPHeader.getUri();
    std::cout << "DELETE request for URI: " << uri << std::endl;
    
    // STEP 2: Build full file path
    std::string filePath = Server.root + uri;
    std::cout << "Full path: " << filePath << std::endl;
    
    // STEP 3: Security check - prevent directory traversal
    if (filePath.find("..") != std::string::npos)
    {
        std::cout << "Security violation: path contains .." << std::endl;
        StatusCode = "403 Forbidden";
        AnswerBody = "<html><body><h1>403 Forbidden</h1><p>Invalid path</p></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
        return;
    }
    
    // STEP 4: Check if file exists
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0)
    {
        std::cout << "File not found: " << filePath << std::endl;
        StatusCode = "404 Not Found";
        AnswerBody = "<html><body><h1>404 Not Found</h1><p>File does not exist</p></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
        return;
    }
    
    // STEP 5: Don't delete directories
    if (S_ISDIR(fileStat.st_mode))
    {
        std::cout << "Cannot delete directory" << std::endl;
        StatusCode = "403 Forbidden";
        AnswerBody = "<html><body><h1>403 Forbidden</h1><p>Cannot delete directory</p></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
        return;
    }
    
    // STEP 6: Delete the file
    if (unlink(filePath.c_str()) == 0)
    {
        std::cout << "Successfully deleted: " << filePath << std::endl;
        StatusCode = "204 No Content";
        AnswerBody = "";
        ContentType = "text/plain";
        ContentLenght = "0";
    }
    else
    {
        std::cout << "Failed to delete (errno: " << errno << ")" << std::endl;
        StatusCode = "500 Internal Server Error";
        AnswerBody = "<html><body><h1>500 Internal Server Error</h1><p>Failed to delete file</p></body></html>";
        ContentType = "text/html";
        ContentLenght = IntToString(AnswerBody.size());
    }
}
```

### Testing Your DELETE Implementation

#### 1. Create a Test File First (Using POST)
```bash
# Start webserv
./webserv config/default.conf

# Create a test file
curl -X POST http://localhost:3333/upload \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "test=data"

# List files to see what was created
ls -la www/uploads/
# Example: post_1733065234.txt
```

#### 2. Test DELETE
```bash
# Delete the file
curl -v -X DELETE http://localhost:3333/uploads/post_1733065234.txt

# Expected response:
# HTTP/1.0 204 No Content
```

#### 3. Verify Deletion
```bash
# File should be gone
ls -la www/uploads/

# Try to delete again (should get 404)
curl -v -X DELETE http://localhost:3333/uploads/post_1733065234.txt
# Expected: 404 Not Found
```

#### 4. Test Security (Directory Traversal)
```bash
# Try to delete a file outside uploads
curl -v -X DELETE http://localhost:3333/uploads/../config/default.conf

# Expected: 403 Forbidden (blocked by ".." check)
```

#### 5. Test Error Cases
```bash
# Non-existent file
curl -v -X DELETE http://localhost:3333/uploads/doesnotexist.txt
# Expected: 404 Not Found

# Try to delete a directory
curl -v -X DELETE http://localhost:3333/uploads/
# Expected: 403 Forbidden
```

### Common Mistakes to Avoid

❌ **Don't forget to check for ".."**
```cpp
// BAD: No security check
std::string path = Server.root + uri;
unlink(path.c_str());  // Dangerous!
```

❌ **Don't forget to set all response fields**
```cpp
// BAD: Missing ContentType and ContentLenght
StatusCode = "404 Not Found";
AnswerBody = "Not found";
// Missing: ContentType and ContentLenght!
```

❌ **Don't use wrong status codes**
```cpp
// BAD: Wrong code for successful deletion
StatusCode = "200 OK";  // Should be 204 No Content
```

✅ **DO check file exists before deleting**
```cpp
// GOOD: Check first
if (stat(path.c_str(), &fileStat) != 0) {
    return 404;
}
unlink(path.c_str());
```

### Status Codes Summary

| Code | Meaning | When to Use |
|------|---------|-------------|
| `204 No Content` | Success, no body | File deleted successfully |
| `200 OK` | Success with body | Alternative (with confirmation message) |
| `404 Not Found` | File doesn't exist | stat() returns -1 |
| `403 Forbidden` | Not allowed | Path contains "..", is directory, no permission |
| `500 Internal Server Error` | Server failure | unlink() fails unexpectedly |

### Next Steps

1. Copy the template to `src/runservers/09_DeleteMethod.cpp`
2. Compile: `make`
3. Test with curl commands above
4. Check server logs for debug output
5. Verify files are actually deleted from filesystem

---

## Quick Reference

### POST vs DELETE Comparison

```cpp
// POST: Receives data, saves file
RequestBody → Parse → Decode → Write File → 201 Created

// DELETE: Receives path, removes file
URI → Build Path → Security Check → Delete File → 204 No Content
```

### Required Headers (Already in Webserv.hpp)
```cpp
#include <sys/stat.h>   // For stat(), S_ISDIR()
#include <unistd.h>     // For unlink()
#include <cerrno>       // For errno
```

### Key Functions
- `stat(path, &fileStat)` - Check if file exists
- `S_ISDIR(fileStat.st_mode)` - Check if directory
- `unlink(path)` - Delete file
- `HTTPHeader.getUri()` - Get request URI
- `IntToString()` - Convert int to string

Good luck with your implementation! 🚀

---

## HTTP/1.0 Specific Considerations

### What is HTTP/1.0?

HTTP/1.0 is the protocol version you're implementing. It's simpler than modern HTTP/1.1 or HTTP/2.

### Key HTTP/1.0 Characteristics

#### 1. Connection Behavior
```
HTTP/1.0: One request per connection
   Client → Server: GET /index.html
   Server → Client: 200 OK + HTML
   Connection CLOSES immediately
   
   Client → Server: GET /style.css (NEW CONNECTION)
   Server → Client: 200 OK + CSS
   Connection CLOSES immediately

HTTP/1.1: Keep-Alive (multiple requests per connection)
   Client → Server: GET /index.html
   Server → Client: 200 OK + HTML
   Connection STAYS OPEN
   
   Client → Server: GET /style.css (SAME CONNECTION)
   Server → Client: 200 OK + CSS
```

**For your webserv:**
```cpp
// After sending response, ALWAYS close the connection
send(client_fd, response.c_str(), response.length(), 0);
close(client_fd);  // HTTP/1.0 behavior
```

#### 2. Response Format
```http
HTTP/1.0 200 OK\r\n
Content-Type: text/html\r\n
Content-Length: 1234\r\n
Connection: close\r\n
\r\n
<html>...</html>
```

**Important headers for HTTP/1.0:**
- `Connection: close` - Tell client connection will close
- `Content-Length` - REQUIRED (can't use chunked encoding)

#### 3. Host Header
```
HTTP/1.0: Host header is OPTIONAL
   GET / HTTP/1.0
   (no Host header needed)

HTTP/1.1: Host header is REQUIRED
   GET / HTTP/1.1
   Host: example.com  ← Must be present!
```

**But in practice:**
Even though HTTP/1.0 doesn't require it, modern browsers always send the Host header. Your webserv should accept it when present.

### How This Affects POST and DELETE

#### POST in HTTP/1.0

**Request:**
```http
POST /upload HTTP/1.0\r\n
Host: localhost:3333\r\n
Content-Type: application/x-www-form-urlencoded\r\n
Content-Length: 27\r\n
\r\n
name=John&email=test@mail.com
```

**Response:**
```http
HTTP/1.0 201 Created\r\n
Content-Type: text/html\r\n
Content-Length: 52\r\n
Connection: close\r\n
\r\n
<html><body>Data saved successfully!</body></html>
```

**Then:** Connection closes immediately!

#### DELETE in HTTP/1.0

**Request:**
```http
DELETE /uploads/file.txt HTTP/1.0\r\n
Host: localhost:3333\r\n
\r\n
```

**Response:**
```http
HTTP/1.0 204 No Content\r\n
Connection: close\r\n
\r\n
(no body for 204)
```

**Then:** Connection closes immediately!

### What Your Response Must Include

For **every** response (POST, DELETE, GET), you must set:

```cpp
// 1. Status line (HTTP/1.0 + status code)
StatusCode = "200 OK";  // or 201, 204, 404, etc.

// 2. Content-Type (even if body is empty)
ContentType = "text/html";

// 3. Content-Length (ALWAYS in HTTP/1.0)
ContentLenght = IntToString(AnswerBody.size());

// 4. Connection header (optional but good practice)
// Add "Connection: close" to response headers
```

### Browser Compatibility

**Modern browsers send HTTP/1.1 requests:**
```http
GET / HTTP/1.1
Host: localhost:3333
```

**Your webserv responds with HTTP/1.0:**
```http
HTTP/1.0 200 OK
Content-Type: text/html
```

**This is perfectly fine!** Browsers understand:
- Server uses older protocol
- Connection will close after response
- Need to reconnect for next resource

### Example: Complete Request/Response Cycle

**Scenario:** Browser requests page, then deletes a file

```
1. Browser → Webserv:
   GET /test_post.html HTTP/1.1
   Host: localhost:3333

2. Webserv → Browser:
   HTTP/1.0 200 OK
   Content-Type: text/html
   Content-Length: 3606
   Connection: close
   
   <!DOCTYPE html>...

3. Connection CLOSES

4. Browser → Webserv (NEW CONNECTION):
   DELETE /uploads/old.txt HTTP/1.1
   Host: localhost:3333

5. Webserv → Browser:
   HTTP/1.0 204 No Content
   Connection: close

6. Connection CLOSES
```

### HTTP/1.0 vs HTTP/1.1 Feature Comparison

| Feature | HTTP/1.0 | HTTP/1.1 | Your Webserv |
|---------|----------|----------|--------------|
| **Connection** | Closes after each response | Keep-alive | Closes (1.0) ✅ |
| **Host header** | Optional | Required | Accept both ✅ |
| **Chunked encoding** | Not supported | Supported | Not needed ✅ |
| **Persistent connections** | No | Yes | No ✅ |
| **Content-Length** | Required | Optional (if chunked) | Required ✅ |
| **Methods** | GET, POST, HEAD | GET, POST, PUT, DELETE, etc. | All methods work ✅ |

### Common Questions

**Q: Browser sends HTTP/1.1, we respond HTTP/1.0. Problem?**
A: No! This is called **version negotiation**. Browser adapts to server's version.

**Q: Do we need to implement keep-alive?**
A: No! HTTP/1.0 doesn't support it. Always close after response.

**Q: What if browser expects chunked encoding?**
A: Won't happen. Browsers only expect chunked if you claim HTTP/1.1 support. Since you respond with HTTP/1.0, they expect Content-Length.

**Q: Can POST and DELETE work in HTTP/1.0?**
A: Yes! The methods are the same, only connection behavior differs.

### Testing with Different HTTP Versions

```bash
# Force curl to use HTTP/1.0
curl --http1.0 -X POST http://localhost:3333/upload -d "test=data"

# curl default (HTTP/1.1)
curl -X POST http://localhost:3333/upload -d "test=data"

# Both work! Your server responds with HTTP/1.0 in both cases
```

### Your Response Building Code

Make sure your response includes HTTP/1.0:

```cpp
std::string buildResponse()
{
    std::ostringstream response;
    
    // Status line: HTTP/1.0 (not 1.1!)
    response << "HTTP/1.0 " << StatusCode << "\r\n";
    
    // Headers
    response << "Content-Type: " << ContentType << "\r\n";
    response << "Content-Length: " << ContentLenght << "\r\n";
    response << "Connection: close\r\n";  // HTTP/1.0 behavior
    
    // Blank line
    response << "\r\n";
    
    // Body
    response << AnswerBody;
    
    return response.str();
}
```

### Summary: What You Need to Know

✅ **Always respond with "HTTP/1.0"** in status line
✅ **Always include Content-Length** header
✅ **Always close connection** after sending response
✅ **Accept HTTP/1.1 requests** but respond with HTTP/1.0
✅ **POST and DELETE work identically** in HTTP/1.0 and 1.1
✅ **No keep-alive, no chunked encoding** needed

The main difference is **connection management**, not the methods themselves. Your POST and DELETE implementations work the same regardless of HTTP version! 🎯
