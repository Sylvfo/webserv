#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color


# Config
WEBSERV_BINARY="./webserv"
WEBSERV_CONFIG="./config/webserv.conf"

# Directories
RESULTS_DIR="test_results"
NGINX_DIR="$RESULTS_DIR/nginx"
WEBSERV_DIR="$RESULTS_DIR/webserv"

# Cleanup function - runs when script exits
cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up...${NC}"

    # Stop nginx container
    if [ -d "docker" ]; then
        cd docker
        docker-compose down 2>/dev/null
        cd ..
    fi

    # Kill webserv if it's running
    if [ -n "$WEBSERV_PID" ]; then
        kill $WEBSERV_PID 2>/dev/null
        wait $WEBSERV_PID 2>/dev/null
    fi

    # Also kill any webserv process on port 8080 (just in case)
    pkill -f "$WEBSERV_BINARY $WEBSERV_CONFIG" 2>/dev/null

    echo -e "${GREEN}✓ Cleanup complete${NC}"
}

# Register cleanup to run on script exit
trap cleanup EXIT

# Create result directories
mkdir -p "$NGINX_DIR" "$WEBSERV_DIR"

echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  Webserv Tester - Comparing nginx vs webserv           ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

start_nginx() {
    echo -e "${YELLOW}Starting nginx...${NC}"
    cd docker
    docker-compose up -d
    cd ..

    # Wait for nginx to be ready
    sleep 10

    # Check if nginx is responding
    if curl -s http://localhost:8080/tiny.html > /dev/null; then
        echo -e "${GREEN}✓ nginx is ready${NC}"
        return 0
    else
        echo -e "${RED}✗ nginx failed to start${NC}"
        return 1
    fi
}

stop_nginx() {
    echo -e "${YELLOW}Stopping nginx...${NC}"
    cd docker
    docker-compose down
    cd ..
    sleep 5
}

test_nginx() {
    echo -e "${YELLOW}Testing nginx...${NC}"

    # Request tiny.html and save headers + body
    curl -D "$NGINX_DIR/tiny_headers.txt" \
         -o "$NGINX_DIR/tiny_body.txt" \
         -s \
         http://localhost:8080/tiny.html

    echo -e "${GREEN}✓ Saved nginx response${NC}"
}


start_webserv() {
    echo -e "${YELLOW}Starting webserv...${NC}"

    # Start webserv in background, redirect output to log
	cd ..
    $WEBSERV_BINARY $WEBSERV_CONFIG > webserv.log 2>&1 &
    WEBSERV_PID=$!
	cd tester_v2

    # Just wait for it to start
    sleep 2

    # Check if process is still alive (don't send request yet)
    if kill -0 $WEBSERV_PID 2>/dev/null; then
        echo -e "${GREEN}✓ webserv started (PID: $WEBSERV_PID)${NC}"
        return 0
    else
        echo -e "${RED}✗ webserv process died immediately${NC}"
        echo -e "${YELLOW}Check webserv.log for errors${NC}"
        return 1
    fi
}

stop_webserv() {
    echo -e "${YELLOW}Stopping webserv...${NC}"

    # Kill the webserv process
    if [ -n "$WEBSERV_PID" ]; then
        kill $WEBSERV_PID 2>/dev/null
        wait $WEBSERV_PID 2>/dev/null
        echo -e "${GREEN}✓ webserv stopped${NC}"
    fi

    sleep 3
}


test_webserv() {
    echo -e "${YELLOW}Testing webserv...${NC}"

    # Add a small delay and use HTTP/1.0 explicitly
    curl -D "$WEBSERV_DIR/tiny_headers.txt" \
         -o "$WEBSERV_DIR/tiny_body.txt" \
         -s \
         --http1.0 \
         http://localhost:8080/tiny.html

    # Wait a bit before checking
    sleep 1

    # Check if webserv is still alive
    if ! kill -0 $WEBSERV_PID 2>/dev/null; then
        echo -e "${RED}⚠ webserv died after request${NC}"
    else
        echo -e "${GREEN}✓ Saved webserv response${NC}"
    fi
}

compare_responses() {
    echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║  Comparing nginx vs webserv responses                  ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""

    local test_name="$1"  # e.g., "tiny.html"
    local passed=0
    local failed=0

	# Extract status codes
    # Format: "HTTP/1.1 200 OK" or "HTTP/1.0 200 OK"
    nginx_status=$(head -1 "$NGINX_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')
	webserv_status=$(head -1 "$WEBSERV_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')

    echo -n "Status code... "
    if [ "$nginx_status" == "$webserv_status" ]; then
        echo -e "${GREEN}✓ MATCH${NC} ($nginx_status)"
        ((passed++))
    else
        echo -e "${RED}✗ DIFFER${NC} (nginx: ${nginx_status}, webserv: ${webserv_status})"
        ((failed++))
    fi

	    # Extract Content-Type
	nginx_ctype=$(grep -i "^Content-Type:" "$NGINX_DIR/${test_name}_headers.txt" | cut -d' ' -f2- | tr -d '\r')
	webserv_ctype=$(grep -i "^Content-Type:" "$WEBSERV_DIR/${test_name}_headers.txt" | cut -d' ' -f2- | tr -d '\r')

    echo -n "Content-Type... "
    if [ "$nginx_ctype" == "$webserv_ctype" ]; then
        echo -e "${GREEN}✓ MATCH${NC} ($nginx_ctype)"
        ((passed++))
    else
        echo -e "${RED}✗ DIFFER${NC} (nginx: ${nginx_ctype}, webserv: ${webserv_ctype})"
        ((failed++))
    fi

	    # Extract Content-Length
	nginx_clen=$(grep -i "^Content-Length:" "$NGINX_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')
	webserv_clen=$(grep -i "^Content-Length:" "$WEBSERV_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')

    echo -n "Content-Length... "
    if [ "$nginx_clen" == "$webserv_clen" ]; then
        echo -e "${GREEN}✓ MATCH${NC} ($nginx_clen bytes)"
        ((passed++))
    else
        echo -e "${RED}✗ DIFFER${NC} (nginx: ${nginx_clen}, webserv: ${webserv_clen})"
        ((failed++))
    fi

    # Compare bodies (easiest - exact match)
    echo -n "Body content... "
    if diff -q "$NGINX_DIR/${test_name}_body.txt" "$WEBSERV_DIR/${test_name}_body.txt" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ MATCH${NC}"
        ((passed++))
    else
        echo -e "${RED}✗ DIFFER${NC}"
        echo -e "${YELLOW}  Run: diff $NGINX_DIR/${test_name}_body.txt $WEBSERV_DIR/${test_name}_body.txt${NC}"
        ((failed++))
    fi

	    # Print summary
    echo ""
    echo -e "${BLUE}Results:${NC}"
    echo -e "  ${GREEN}Passed: $passed${NC}"
    echo -e "  ${RED}Failed: $failed${NC}"
    echo ""

    if [ $failed -eq 0 ]; then
        echo -e "${GREEN}✓ All checks passed!${NC}"
        return 0
    else
        echo -e "${RED}✗ Some checks failed${NC}"
        return 1
    fi
}

# ============================================================================
# MAIN EXECUTION
# ============================================================================

# Clean up old results
rm -rf "$RESULTS_DIR"
mkdir -p "$NGINX_DIR" "$WEBSERV_DIR"

# Test nginx
if start_nginx; then
    test_nginx
    stop_nginx
else
    echo -e "${RED}Failed to start nginx. Exiting.${NC}"
    exit 1
fi

echo ""

# Test webserv
if start_webserv; then
    test_webserv
    stop_webserv
else
    echo -e "${RED}Failed to start webserv. Exiting.${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}✓ All tests complete!${NC}"
echo -e "${BLUE}Results saved in: $RESULTS_DIR${NC}"
echo ""
echo "Check the files:"
echo "  ls -la $NGINX_DIR/"
echo "  ls -la $WEBSERV_DIR/"

echo ""
echo -e "${GREEN}✓ All tests complete!${NC}"
echo -e "${BLUE}Results saved in: $RESULTS_DIR${NC}"
echo ""

# Compare the responses
compare_responses "tiny"
