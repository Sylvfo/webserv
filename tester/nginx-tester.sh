#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
NGINX_PORT=8080
TEST_OUTPUT_DIR="./test_results_nginx_only"
DOCKER_COMPOSE_DIR="./docker-stuff"

echo -e "${GREEN}Nginx-Only Tester${NC}"
echo -e "${YELLOW}======================================${NC}"
echo ""

# Cleanup function
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    cd "$DOCKER_COMPOSE_DIR"
    docker-compose down 2>/dev/null || true
    cd ..
}

trap cleanup EXIT

# Prepare output directory
rm -rf "$TEST_OUTPUT_DIR"
mkdir -p "$TEST_OUTPUT_DIR"

# Main execution
echo -e "${YELLOW}Starting nginx container...${NC}"
cd "$DOCKER_COMPOSE_DIR"
docker-compose up -d
cd ..
sleep 2

# Wait for nginx to be ready
echo -e "${YELLOW}Waiting for nginx to be ready...${NC}"
for i in {1..10}; do
    if curl -s "http://localhost:$NGINX_PORT" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ nginx is ready${NC}"
        break
    fi
    sleep 1
done

echo ""
echo -e "${YELLOW}Running tests...${NC}"
echo ""

# Test 1: GET index page
echo -n "Test 1: GET index page... "
curl -s -o "$TEST_OUTPUT_DIR/test1_get_index.body" \
     -D "$TEST_OUTPUT_DIR/test1_get_index.headers" \
     "http://localhost:$NGINX_PORT/" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Test 2: GET non-existent page (404)
echo -n "Test 2: GET 404 page... "
curl -s -o "$TEST_OUTPUT_DIR/test2_get_404.body" \
     -D "$TEST_OUTPUT_DIR/test2_get_404.headers" \
     "http://localhost:$NGINX_PORT/nonexistent.html" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Test 3: POST request (should return 405 on /)
echo -n "Test 3: POST to root (405)... "
curl -s -X POST -o "$TEST_OUTPUT_DIR/test3_post_root.body" \
     -D "$TEST_OUTPUT_DIR/test3_post_root.headers" \
     "http://localhost:$NGINX_PORT/" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Test 4: GET with autoindex (/docs/)
echo -n "Test 4: GET autoindex... "
curl -s -o "$TEST_OUTPUT_DIR/test4_get_docs.body" \
     -D "$TEST_OUTPUT_DIR/test4_get_docs.headers" \
     "http://localhost:$NGINX_PORT/docs/" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Test 5: POST to CGI echo
echo -n "Test 5: POST to CGI... "
curl -s -X POST -d "test=data&foo=bar" \
     -o "$TEST_OUTPUT_DIR/test5_post_cgi.body" \
     -D "$TEST_OUTPUT_DIR/test5_post_cgi.headers" \
     "http://localhost:$NGINX_PORT/cgi-bin/echo.sh" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Test 6: GET CGI echo
echo -n "Test 6: GET CGI with params... "
curl -s -o "$TEST_OUTPUT_DIR/test6_get_cgi.body" \
     -D "$TEST_OUTPUT_DIR/test6_get_cgi.headers" \
     "http://localhost:$NGINX_PORT/cgi-bin/echo.sh?param1=value1&param2=value2" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Test 7: HEAD request
echo -n "Test 7: HEAD request... "
curl -s -I -o "$TEST_OUTPUT_DIR/test7_head.headers" \
     "http://localhost:$NGINX_PORT/" 2>/dev/null
echo -e "${GREEN}✓${NC}"

# Show some results
echo ""
echo -e "${YELLOW}Sample outputs:${NC}"
echo ""
echo -e "${GREEN}=== Test 1 Headers ===${NC}"
cat "$TEST_OUTPUT_DIR/test1_get_index.headers"
echo ""
echo -e "${GREEN}=== Test 2 (404) Headers ===${NC}"
cat "$TEST_OUTPUT_DIR/test2_get_404.headers"

echo ""
echo -e "${GREEN}✓ All tests completed! Results saved in $TEST_OUTPUT_DIR${NC}"
echo -e "${YELLOW}Check the files to see nginx responses${NC}"
