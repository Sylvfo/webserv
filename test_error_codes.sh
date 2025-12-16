#!/bin/bash

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PORT=2222
HOST="localhost"
BASE_URL="http://${HOST}:${PORT}"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   WEBSERV ERROR CODES TEST SUITE${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Fonction pour tester un code d'erreur
test_error() {
    local test_name="$1"
    local url="$2"
    local expected_code="$3"
    local method="${4:-GET}"
    local data="$5"
    
    echo -e "${YELLOW}Testing: ${test_name}${NC}"
    
    if [ "$method" = "POST" ]; then
        response=$(curl -s -w "\n%{http_code}" -X POST -d "$data" "${url}" 2>/dev/null)
    elif [ "$method" = "DELETE" ]; then
        response=$(curl -s -w "\n%{http_code}" -X DELETE "${url}" 2>/dev/null)
    else
        response=$(curl -s -w "\n%{http_code}" "${url}" 2>/dev/null)
    fi
    
    http_code=$(echo "$response" | tail -n1)
    body=$(echo "$response" | head -n-1)
    
    if [ "$http_code" = "$expected_code" ]; then
        echo -e "${GREEN}✓ PASS${NC} - Got ${http_code} as expected"
        if echo "$body" | grep -q "<title>${expected_code}"; then
            echo -e "${GREEN}✓ PASS${NC} - HTML page contains correct error code"
        else
            echo -e "${RED}✗ FAIL${NC} - HTML page doesn't contain error code ${expected_code}"
        fi
    else
        echo -e "${RED}✗ FAIL${NC} - Expected ${expected_code}, got ${http_code}"
    fi
    echo ""
}

# Vérifier que le serveur est accessible
echo -e "${BLUE}Checking if server is running...${NC}"
if ! curl -s "${BASE_URL}" > /dev/null 2>&1; then
    echo -e "${RED}✗ Server is not responding on ${BASE_URL}${NC}"
    echo -e "${YELLOW}Please start the server with: ./webserv config/beboccas.conf${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Server is running${NC}\n"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   400 - Bad Request Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 400 - Bad Request (malformed request)
echo -e "${YELLOW}Testing: 400 Bad Request (manual telnet test)${NC}"
echo -e "To test manually, run:"
echo -e "  echo -e 'INVALID REQUEST\\r\\n\\r\\n' | nc localhost ${PORT}"
echo ""

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   403 - Forbidden Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 403 - Forbidden (si vous avez configuré des restrictions)
test_error "403 Forbidden - Access denied" \
    "${BASE_URL}/forbidden_path" \
    "403"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   404 - Not Found Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

test_error "404 Not Found - Non-existent file" \
    "${BASE_URL}/nonexistent.html" \
    "404"

test_error "404 Not Found - Non-existent path" \
    "${BASE_URL}/this/path/does/not/exist" \
    "404"

test_error "404 Not Found - Missing file in valid directory" \
    "${BASE_URL}/missing_file_12345.txt" \
    "404"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   405 - Method Not Allowed Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 405 - Method Not Allowed (si POST n'est pas autorisé quelque part)
test_error "405 Method Not Allowed - POST on GET-only location" \
    "${BASE_URL}/" \
    "405" \
    "POST" \
    "test=data"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   413 - Payload Too Large Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 413 - Payload Too Large
echo -e "${YELLOW}Testing: 413 Payload Too Large${NC}"
echo -e "Generating large payload (5MB)..."
large_data=$(head -c 5000000 < /dev/zero | tr '\0' 'A')
test_error "413 Payload Too Large - 5MB POST" \
    "${BASE_URL}/upload" \
    "413" \
    "POST" \
    "$large_data"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   414 - URI Too Long Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 414 - URI Too Long
long_uri=$(printf 'A%.0s' {1..10000})
test_error "414 URI Too Long - 10000 chars URI" \
    "${BASE_URL}/${long_uri}" \
    "414"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   500 - Internal Server Error Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 500 - Internal Server Error (CGI error if configured)
test_error "500 Internal Server Error - CGI error" \
    "${BASE_URL}/cgi-bin/broken_script.sh" \
    "500"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   501 - Not Implemented Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 501 - Not Implemented (méthode non supportée)
echo -e "${YELLOW}Testing: 501 Not Implemented (manual test)${NC}"
echo -e "To test manually, run:"
echo -e "  curl -X TRACE ${BASE_URL}/"
echo ""

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   502 - Bad Gateway Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

# Test 502 - Bad Gateway (CGI timeout or error)
test_error "502 Bad Gateway - CGI timeout" \
    "${BASE_URL}/cgi-bin/timeout_script.sh" \
    "502"

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   503 - Service Unavailable Tests${NC}"
echo -e "${BLUE}================================================${NC}\n"

echo -e "${YELLOW}Testing: 503 Service Unavailable${NC}"
echo -e "This typically requires the server to be overloaded"
echo -e "Manual test: Send many concurrent requests"
echo ""

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   Summary${NC}"
echo -e "${BLUE}================================================${NC}\n"

echo -e "Test suite completed!"
echo -e "\nTo view error pages in a browser, visit:"
echo -e "  - ${BASE_URL}/nonexistent.html (404)"
echo -e "  - ${BASE_URL}/forbidden (403)"
echo -e "\nTo test custom error pages:"
echo -e "  1. Add to config: error_page 404 errors/custom404.html"
echo -e "  2. Create the file: www/errors/custom404.html"
echo -e "  3. Restart the server and rerun tests"
