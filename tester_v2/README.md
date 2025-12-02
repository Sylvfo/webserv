# Webserv Tester

Compares your webserv against nginx to verify HTTP compliance.

## Usage

```bash
./run-tests.sh          # Run all tests
./run-tests.sh clean    # Remove generated files
```

## What It Tests

| Category | Tests |
|----------|-------|
| **GET** | `index.html`, `tiny.html`, `medium.html`, `large.html`, `test.jpg` |
| **Errors** | `404_not_found.html`, `403_forbidden.html` |
| **Directory** | `uploads/` (autoindex) |
| **POST** | `uploads/new_file.txt`, `cgi-bin/echo.sh` |
| **DELETE** | `test.jpg`, `tiny.html` |

## How It Works

1. Resets `www/` from `fixtures/`
2. Starts nginx → runs GET tests → stops
3. Resets `www/` from `fixtures/`
4. Starts webserv → runs GET/POST/DELETE tests → stops
5. Compares responses (body, status, content-length)

## Output Files

| Path | Contents |
|------|----------|
| `test_results/nginx/` | Nginx responses |
| `test_results/webserv/` | Webserv responses |
| `webserv.log` | Server stdout/stderr |
| `www/` | Runtime test directory |

## Adding Tests

Edit arrays in `run-tests.sh`:
- `TESTS_BASIC` - static files
- `TESTS_ERRORS` - error pages
- `TESTS_POST` - format: `"url|data"`
- `TESTS_DELETE` - files to delete

## Requirements

- Docker (for nginx)
- `fixtures/` directory with test files
