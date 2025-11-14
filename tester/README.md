# Webserv Tester 🧪

Automated testing framework that compares your webserv implementation against nginx to ensure HTTP/1.1 compliance.

---

## 📋 Overview

This tester validates your webserv by:
1. Running identical HTTP tests against both **nginx** (reference) and **webserv** (your implementation)
2. Capturing all responses (headers + body)
3. Comparing outputs byte-by-byte
4. Providing clear pass/fail feedback

**Goal:** Ensure your webserv behaves exactly like nginx for HTTP/1.1 requests.

---

## ✅ Prerequisites

- **Docker** & **Docker Compose** installed
- `curl` command-line tool
- Bash shell
- Your compiled **webserv binary**
- Config file: `config/tester.conf`

---

## 🚀 Quick Start

### 1. Setup (one-time)

```bash
# Compile webserv
cd /home/bschmid/Documents/webserv
make
```

# Ensure www/ structure exists
ls www/origameee/     # HTML files
ls www/cgi-bin/       # CGI scripts (echo.sh, upload.sh)
ls www/errors/        # Error pages (404.html, 405.html, etc.)


## Run tester

cd tester

### Standard test (nginx vs webserv)
./webserv-tester.sh -w ../webserv

### Continue even if webserv crashes
./webserv-tester.sh -w ../webserv -c

### Only test nginx (no webserv needed)
./webserv-tester.sh -s


## Debugging


### See what's different
diff test_results/nginx/test2_get_404.headers \
     test_results/webserv/test2_get_404.headers

### Side-by-side comparison
diff -y test_results/nginx/test2_get_404.headers \
        test_results/webserv/test2_get_404.headers


## Options

-w <path>	Required. Path to webserv binary (e.g., webserv)
-c	Continue on webserv startup failure (useful during development)
-s	Skip webserv tests (nginx-only mode)
-h	Show help message
