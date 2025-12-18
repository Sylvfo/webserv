# Webserv Tester V2

A comparative testing suite that verifies your `webserv` implementation against a reference Nginx server.

---

## 🚀 Quick Start & Setup

### 1. Prerequisites
Before running the tester, ensure you have:
- **Docker** installed and running (for the Nginx reference).
- **docker-compose** installed
- **Curl** installed (for sending requests).
- Your **Webserv** compiled.


2. **Check Configuration:**
   The tester expects your webserv to:
   - Run on **Port 8080**.
   - Use the configuration file `config/webserv.conf`.
   - Serve files from the `tester_v2/www` directory (which is automatically created).

   *Note: The script automatically handles the creation of the `www` folder from `fixtures`.*

### 3. Run the Tester
Navigate to the `tester_v2` directory and run:

```bash
cd tester_v2
./run-tests.sh
```

To clean up generated files (logs, results, temporary www folder):
```bash
./run-tests.sh clean
```

---

## 🔍 How It Works & What It Tests

This tester works on the principle of **Differential Testing**. It runs the same requests against a standard Nginx server and your Webserv, then compares the results.

### The Testing Workflow
1. **Environment Reset:** 
   Before testing each server, the script deletes the `www/` folder and recreates it from `fixtures/`. This ensures a clean state for every run, which is critical for testing stateful methods like POST and DELETE.

2. **Nginx Phase (The "Ground Truth"):**
   - Starts Nginx in a Docker container.
   - Runs a batch of GET requests.
   - Saves headers and bodies to `test_results/nginx/`.

3. **Webserv Phase (Your Server):**
   - Starts your `./webserv` binary.
   - Runs the same GET requests.
   - Runs additional POST and DELETE tests.
   - Saves headers and bodies to `test_results/webserv/`.

4. **Comparison Phase:**
   - **GET Requests:** Compares Status Code, Content-Length, and Body content exactly against Nginx.
   - **POST/DELETE:** Checks for specific success criteria (e.g., Did the file actually get deleted? Did the server return 201 Created?).

### Test Coverage

| Category | Description | What is checked? |
|----------|-------------|------------------|
| **Static Content** | `index.html`, `tiny.html`, images | Exact match with Nginx (Body & Headers). |
| **Error Handling** | `404 Not Found`, `403 Forbidden` | Status code match. |
| **Directory Listing** | Requesting a directory path | Checks if autoindex or default file works. |
| **POST** | Uploading files & CGI | Checks for `201 Created` or `200 OK`. |
| **DELETE** | Deleting files | Checks for `204 No Content` or `200 OK` and verifies file is gone from disk. |

### Directory Structure
- `fixtures/`: The source of truth for test files. Never modified.
- `www/`: The runtime directory. Created/Deleted automatically. **Do not edit manually.**
- `test_results/`: Contains the raw output (headers and bodies) from both servers.
- `docker/`: Configuration for the reference Nginx container.
