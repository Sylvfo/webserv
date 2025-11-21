# POST Implementation - Complete Flow Guide

## 🎯 POST Request Flow - Simple Overview

```
CLIENT
  │
  │  curl -X POST http://localhost:3333/upload -d "name=Bob&email=bob@example.com"
  │
  ▼
┌─────────────────────────────────────────────────┐
│  1. RECEIVE                                     │
│  → recv() gets raw HTTP request                 │
│  → Store in RawRequest                          │
└─────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────┐
│  2. PARSE                                       │
│  → Extract method, URI, headers                 │
│  → Extract body: "name=Bob&email=bob@..."       │
│  → Store in RequestBody                         │
└─────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────┐
│  3. ROUTE                                       │
│  → Check method == "POST"                       │
│  → Call PostRequest()                           │
└─────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────┐
│  4. PROCESS POST                                │
│  → Check Content-Type                           │
│  → Route to HandleFormData()                    │
└─────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────┐
│  5. HANDLE FORM DATA                            │
│  → Parse: "name=Bob" → map["name"] = "Bob"      │
│  → Decode: "bob%40test.com" → "bob@test.com"    │
│  → Save to: uploads/post_[timestamp].txt        │
│  → Set: StatusCode = "201 Created"              │
└─────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────┐
│  6. BUILD RESPONSE                              │
│  → SetStatusLine()                              │
│  → SetResponseHeader()                          │
│  → HttpAnswer = "HTTP/1.0 201 Created\r\n..."   │
└─────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────┐
│  7. SEND                                        │
│  → send() back to client                        │
└─────────────────────────────────────────────────┘
  │
  ▼
CLIENT receives: "201 Created - Data saved!"
```

---

## 📁 Files Involved

| Step | File | Function |
|------|------|----------|
| 1 | `04_recieve_request.cpp` | `recieveRequest()` |
| 2 | `04_recieve_request.cpp` | `parseRequest()` + `extractRequestBody()` |
| 3 | `06_build_local_answer.cpp` | `Answerlocal()` |
| 4 | `08_PostMethod.cpp` | `PostRequest()` |
| 5 | `08_PostMethod.cpp` | `HandleFormData()` |
| 6 | `06_build_local_answer.cpp` | `SetStatusLine()` + `SetResponseHeader()` |
| 7 | Various | `sendAnswerToRequest()` |

---

## 🔑 Key Functions in POST

### `PostRequest()` - Content Type Router
```cpp
if (ContentType == "application/x-www-form-urlencoded")
    HandleFormData();          // ✅ Implemented
else if (ContentType == "multipart/form-data")
    return 501;                // 🔜 Future
else if (ContentType == "application/json")
    return 501;                // 🔜 Future
else
    return 415;                // ❌ Not supported
```

### `HandleFormData()` - Main Logic
```cpp
1. Get upload_path from config
2. Parse form data: parseFormData(RequestBody)
3. Decode values: urlDecode()
4. Generate filename: "post_" + timestamp + ".txt"
5. Save to file
6. Set response: StatusCode = "201 Created"
```

### Helper Functions
- **`parseFormData()`** - Split by `&` and `=` → returns map
- **`urlDecode()`** - Convert `%40` → `@`, `+` → space
- **`getCurrentTimestamp()`** - Generate unique filename ID

---

## � Member Variables - Data Journey

```
1. RECEIVE
   RawRequest = "POST /upload HTTP/1.1\r\n...name=Bob&email=bob@..."

2. PARSE
   method = "POST"
   uri = "/upload"
   RequestBody = "name=Bob&email=bob@example.com"

3. PROCESS
   formData["name"] = "Bob"
   formData["email"] = "bob@example.com"

4. SAVE
   File created: www/uploads/post_1732188234.txt

5. RESPOND
   StatusCode = "201 Created"
   AnswerBody = "Data saved successfully!"
   HttpAnswer = "HTTP/1.0 201 Created\r\n..."

6. SEND
   send(socket_fd, HttpAnswer)
```

---

## � GET vs POST Comparison

| Aspect | GET | POST |
|--------|-----|------|
| **Purpose** | Read data FROM server | Send data TO server |
| **Main Action** | `open()` + `read()` file | Parse data + `write()` file |
| **Input** | URI path | Request body |
| **Output** | File contents | Confirmation message |
| **Status Code** | 200 OK | 201 Created |
| **File Operation** | Read existing file | Create new file |

**Key Similarity:** Both set `StatusCode`, `AnswerBody`, `ContentType` the same way!

---

## 🛠️ Helper Functions Explained

### `parseFormData()` - Split form data into map
```
Input:  "name=Bob&email=bob@example.com&age=25"

Process:
  Split by '&'  →  ["name=Bob", "email=bob@example.com", "age=25"]
  Split by '='  →  [key, value] pairs

Output: map["name"] = "Bob"
        map["email"] = "bob@example.com"
        map["age"] = "25"
```

### `urlDecode()` - Convert encoded characters
```
Input                    →  Output
"Bob+Smith"              →  "Bob Smith"           (+ means space)
"test%40example.com"     →  "test@example.com"    (%40 is @)
"hello%20world"          →  "hello world"         (%20 is space)
"50%25+discount"         →  "50% discount"        (%25 is %)
```

**Why needed?** Browsers encode special characters in forms. We decode them server-side.

### `getCurrentTimestamp()` - Generate unique filename
```
Output: "1732188234" (seconds since 1970)

Usage: filename = "post_" + timestamp + ".txt"
       → "post_1732188234.txt"
```

**Why needed?** Prevents overwriting previous submissions.

---

## ⚙️ Config File Setup

```nginx
location /upload {
    method GET POST;                              # Allow POST here
    upload_path /home/user/webserv/www/uploads;   # Where to save files
}
```

**How POST uses this:**
1. Match URI `/upload` with `location.path`
2. Get `upload_path` from matched location
3. Save file: `upload_path + "/post_" + timestamp + ".txt"`

---

## 🧪 Quick Tests

### Test 1: Basic POST
```bash
curl -X POST http://localhost:3333/upload \
     -d "name=Alice&age=30"
```
**Expected:** File created with contents:
```
=== POST Data ===
age: 30
name: Alice
```

### Test 2: Special Characters
```bash
curl -X POST http://localhost:3333/upload \
     -d "email=test%40example.com&name=Bob+Smith"
```
**Expected:** Proper decoding:
```
email: test@example.com
name: Bob Smith
```

### Test 3: Error Case
```bash
curl -X POST http://localhost:3333/upload \
     -H "Content-Type: application/json" \
     -d '{"test":"data"}'
```
**Expected:** `501 Not Implemented`

---

## 📚 Key Takeaways

After implementing POST, you understand:

✅ **Request → Response Flow**
- Socket receives raw bytes → Parse to structured data → Process → Send response

✅ **Data Transformation**
- URL-encoded → Decoded → Saved to file

✅ **Member Variables**
- Functions communicate by setting: `StatusCode`, `AnswerBody`, `ContentType`

✅ **Pattern Consistency**
- POST follows same pattern as GET (easy to maintain)

---

## 🚀 Implementation Checklist

### POST Implementation ✅
- [x] Parse form data (`parseFormData()`)
- [x] URL decode values (`urlDecode()`)
- [x] Save to file with timestamp
- [x] Return `201 Created`
- [x] Handle unsupported types (`501`, `415`)

### Next Steps
- [ ] Implement DELETE method
- [ ] Add CGI support
- [ ] Add security checks (file size limits, path validation)

---

## 💡 Common Issues & Fixes

| Problem | Cause | Solution |
|---------|-------|----------|
| "Cannot create file" | Directory missing | `mkdir -p www/uploads` |
| Special chars not decoded | Forgot `urlDecode()` | Always decode values |
| Semicolon in path | Parser includes `;` | Strip `;` in config parser |
| Wrong location matched | Matches shortest first | Find longest matching path |

---

**🎉 Your POST is ready! The implementation is clean, tested, and follows best practices.**
