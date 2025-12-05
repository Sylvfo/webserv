
#!/bin/bash

# set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Fixed configuration
TEST_PORT=8080
NGINX_CONFIG_FILE="./docker-stuff/nginx.conf"
WEBSERV_CONFIG_FILE="../config/tester.conf"
TEST_OUTPUT_DIR="./test_results"
NGINX_OUTPUT_DIR="$TEST_OUTPUT_DIR/nginx"
WEBSERV_OUTPUT_DIR="$TEST_OUTPUT_DIR/webserv"
WEBSERV_BINARY=""
DOCKER_COMPOSE_DIR="./docker-stuff"

# Fixed paths
HTML_DIR="../www/origameee"
CGI_DIR="../www/cgi-bin"
ERRORS_DIR="../www/errors"

# Timeout settings
STARTUP_TIMEOUT=60
REQUEST_TIMEOUT=5

# Usage
usage() {
    echo "Usage: $0 -w <webserv_binary> [options]"
    echo ""
    echo "Required:"
    echo "  -w <path>     Path to webserv binary"
    echo ""
    echo "Optional:"
    echo "  -s            Skip webserv tests (nginx only)"
    echo "  -c            Continue on webserv startup failure"
    echo "  -h            Show this help message"
    echo ""
    echo "Example:"
    echo "  $0 -w ../webserv"
    echo "  $0 -w ../webserv -c    # Continue even if webserv fails"
    echo "  $0 -s                  # Only test nginx (no webserv needed)"
    exit 1
}

# Parse arguments
SKIP_WEBSERV=false
CONTINUE_ON_FAIL=false

while getopts "w:sch" opt; do
    case $opt in
        w) WEBSERV_BINARY="$OPTARG" ;;
        s) SKIP_WEBSERV=true ;;
        c) CONTINUE_ON_FAIL=true ;;
        h) usage ;;
        *) usage ;;
    esac
done

# Validate
if [ "$SKIP_WEBSERV" = false ] && [ -z "$WEBSERV_BINARY" ]; then
    usage
fi

if [ "$SKIP_WEBSERV" = false ] && [ ! -f "$WEBSERV_BINARY" ]; then
    echo -e "${RED}Error: webserv binary not found at $WEBSERV_BINARY${NC}"
    exit 1
fi

if [ ! -f "$WEBSERV_CONFIG_FILE" ]; then
    echo -e "${RED}Error: webserv config not found at $WEBSERV_CONFIG_FILE${NC}"
    exit 1
fi

if [ ! -f "$NGINX_CONFIG_FILE" ]; then
    echo -e "${RED}Error: nginx config not found at $NGINX_CONFIG_FILE${NC}"
    exit 1
fi

# Convert to absolute paths
NGINX_CONFIG=$(realpath "$NGINX_CONFIG_FILE")
WEBSERV_CONFIG=$(realpath "$WEBSERV_CONFIG_FILE")
HTML_DIR=$(realpath "$HTML_DIR")
CGI_DIR=$(realpath "$CGI_DIR")
ERRORS_DIR=$(realpath "$ERRORS_DIR")

# Display config
echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║         Webserv Tester - Flexible Mode                    ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BLUE}Configuration:${NC}"
if [ "$SKIP_WEBSERV" = false ]; then
    echo -e "  Webserv binary:   ${WEBSERV_BINARY}"
    echo -e "  Webserv config:   ${WEBSERV_CONFIG}"
fi
echo -e "  Nginx config:     ${NGINX_CONFIG}"
echo -e "  Test port:        ${TEST_PORT}"
echo -e "  Skip webserv:     ${SKIP_WEBSERV}"
echo -e "  Continue on fail: ${CONTINUE_ON_FAIL}"
echo ""

# Cleanup function
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    cd "$DOCKER_COMPOSE_DIR"
    docker-compose down 2>/dev/null || true
    cd ..
    pkill -f "$WEBSERV_BINARY" 2>/dev/null || true
}

trap cleanup EXIT

# Prepare output
rm -rf "$TEST_OUTPUT_DIR"
mkdir -p "$NGINX_OUTPUT_DIR" "$WEBSERV_OUTPUT_DIR"

# Wait for server with timeout
wait_for_server() {
    local port=$1
    local name=$2
    local timeout=$3

    echo -e "${YELLOW}Waiting for $name to be ready (timeout: ${timeout}s)...${NC}"

    for i in $(seq 1 $timeout); do
        if curl -s --max-time 2 "http://localhost:$port" > /dev/null 2>&1; then
            echo -e "${GREEN}✓ $name is ready${NC}"
            return 0
        fi
        sleep 1
    done

    echo -e "${RED}✗ $name failed to start within ${timeout}s${NC}"
    return 1
}

# Run tests (with error handling)
run_tests() {
    local port=$1
    local output_dir=$2
    local server_name=$3

    echo -e "${CYAN}Running tests against $server_name on port $port...${NC}"

    local tests_run=0
    local tests_failed=0

    # Test 1: GET index
    echo -n "  Test 1: GET index page... "
    if curl -s --max-time $REQUEST_TIMEOUT -o "$output_dir/test1_get_index.body" \
         -D "$output_dir/test1_get_index.headers" \
         "http://localhost:$port/" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    # Test 2: GET 404
    echo -n "  Test 2: GET 404 page... "
    if curl -s --max-time $REQUEST_TIMEOUT -o "$output_dir/test2_get_404.body" \
         -D "$output_dir/test2_get_404.headers" \
         "http://localhost:$port/nonexistent.html" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    # Test 3: POST root (405)
    echo -n "  Test 3: POST to root (405)... "
    if curl -s --max-time $REQUEST_TIMEOUT -X POST -o "$output_dir/test3_post_root.body" \
         -D "$output_dir/test3_post_root.headers" \
         "http://localhost:$port/" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    # Test 4: GET autoindex
    echo -n "  Test 4: GET autoindex... "
    if curl -s --max-time $REQUEST_TIMEOUT -o "$output_dir/test4_get_docs.body" \
         -D "$output_dir/test4_get_docs.headers" \
         "http://localhost:$port/docs/" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    # Test 5: POST CGI
    echo -n "  Test 5: POST to CGI... "
    if curl -s --max-time $REQUEST_TIMEOUT -X POST -d "test=data&foo=bar" \
         -o "$output_dir/test5_post_cgi.body" \
         -D "$output_dir/test5_post_cgi.headers" \
         "http://localhost:$port/cgi-bin/echo.sh" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    # Test 6: GET CGI
    echo -n "  Test 6: GET CGI with params... "
    if curl -s --max-time $REQUEST_TIMEOUT -o "$output_dir/test6_get_cgi.body" \
         -D "$output_dir/test6_get_cgi.headers" \
         "http://localhost:$port/cgi-bin/echo.sh?param1=value1&param2=value2" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    # Test 7: HEAD
    echo -n "  Test 7: HEAD request... "
    if curl -s --max-time $REQUEST_TIMEOUT -I -o "$output_dir/test7_head.headers" \
         "http://localhost:$port/" 2>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((tests_run++))
    else
        echo -e "${RED}✗ (timeout/error)${NC}"
        ((tests_failed++))
    fi

    echo -e "${GREEN}✓ Completed $tests_run/7 tests for $server_name${NC}"
    if [ $tests_failed -gt 0 ]; then
        echo -e "${YELLOW}⚠ $tests_failed tests timed out or failed${NC}"
    fi
}

# Compare outputs (with missing file handling)
compare_outputs() {
    echo ""
    echo -e "${YELLOW}Comparing outputs...${NC}"
    echo ""

    local passed=0
    local failed=0
    local missing=0

    for nginx_file in "$NGINX_OUTPUT_DIR"/*.body "$NGINX_OUTPUT_DIR"/*.headers; do
        [ -f "$nginx_file" ] || continue

        filename=$(basename "$nginx_file")
        webserv_file="$WEBSERV_OUTPUT_DIR/$filename"

        if [ ! -f "$webserv_file" ]; then
            echo -e "${YELLOW}⊘ SKIP: Missing webserv output for $filename${NC}"
            ((missing++))
            continue
        fi

        if diff -q "$nginx_file" "$webserv_file" > /dev/null 2>&1; then
            echo -e "${GREEN}✓ PASS: $filename${NC}"
            ((passed++))
        else
            echo -e "${RED}✗ FAIL: $filename${NC}"
            ((failed++))
        fi
    done

    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║           Test Results                 ║${NC}"
    echo -e "${GREEN}╠════════════════════════════════════════╣${NC}"
    printf "${GREEN}║${NC} Passed:  %-29s ${GREEN}║${NC}\n" "${GREEN}$passed${NC}"
    printf "${GREEN}║${NC} Failed:  %-29s ${GREEN}║${NC}\n" "${RED}$failed${NC}"
    printf "${GREEN}║${NC} Missing: %-29s ${GREEN}║${NC}\n" "${YELLOW}$missing${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
}

# Main execution
echo -e "${YELLOW}[1/6] Starting nginx container...${NC}"
cd "$DOCKER_COMPOSE_DIR"
export NGINX_CONFIG
export WEBSERV_HTML_DIR="$HTML_DIR"
export WEBSERV_CGI_DIR="$CGI_DIR"
export WEBSERV_ERRORS_DIR="$ERRORS_DIR"
docker-compose up -d
cd ..
sleep 2

if ! wait_for_server "$TEST_PORT" "nginx" "$STARTUP_TIMEOUT"; then
    echo -e "${RED}Fatal: nginx failed to start${NC}"
    exit 1
fi

echo -e "${YELLOW}[2/6] Testing nginx...${NC}"
run_tests "$TEST_PORT" "$NGINX_OUTPUT_DIR" "nginx"

echo -e "${YELLOW}[3/6] Stopping nginx...${NC}"
cd "$DOCKER_COMPOSE_DIR"
docker-compose down
cd ..
sleep 2

# Skip webserv if requested
if [ "$SKIP_WEBSERV" = true ]; then
    echo -e "${CYAN}Skipping webserv tests (nginx-only mode)${NC}"
    exit 0
fi

echo -e "${YELLOW}[4/6] Starting webserv...${NC}"
"$WEBSERV_BINARY" "$WEBSERV_CONFIG" &
WEBSERV_PID=$!
sleep 2

if ! wait_for_server "$TEST_PORT" "webserv" "$STARTUP_TIMEOUT"; then
    if [ "$CONTINUE_ON_FAIL" = true ]; then
        echo -e "${YELLOW}⚠ Continuing despite webserv startup failure${NC}"
        echo -e "${CYAN}Only nginx results available${NC}"
        exit 0
    else
        echo -e "${RED}Fatal: webserv failed to start${NC}"
        echo -e "${YELLOW}Tip: Use -c flag to continue on failure${NC}"
        exit 1
    fi
fi

echo -e "${YELLOW}[5/6] Testing webserv...${NC}"
run_tests "$TEST_PORT" "$WEBSERV_OUTPUT_DIR" "webserv"

echo -e "${YELLOW}Stopping webserv...${NC}"
kill $WEBSERV_PID 2>/dev/null || true
sleep 1

echo -e "${YELLOW}[6/6] Comparing results...${NC}"
compare_outputs

echo ""
echo -e "${GREEN}✓ Testing complete! Results in $TEST_OUTPUT_DIR${NC}"
echo -e "${BLUE}To debug differences:${NC}"
echo -e "  diff test_results/nginx/<file> test_results/webserv/<file>"
