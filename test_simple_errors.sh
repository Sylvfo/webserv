#!/bin/bash

# Script simple pour tester les codes d'erreur HTTP
# Usage: ./test_simple_errors.sh [port]

PORT=${1:-2222}
HOST="localhost"

echo "Testing error codes on http://${HOST}:${PORT}"
echo "=============================================="
echo ""

# Test 404
echo "Test 1: 404 Not Found"
curl -s -o /dev/null -w "Status: %{http_code}\n" "http://${HOST}:${PORT}/nonexistent"
echo ""

# Test 404 with body display
echo "Test 2: 404 Not Found (with HTML body)"
curl -s "http://${HOST}:${PORT}/missing.html"
echo ""
echo ""

# Test 405 (POST on root if not allowed)
echo "Test 3: 405 Method Not Allowed"
curl -s -o /dev/null -w "Status: %{http_code}\n" -X POST "http://${HOST}:${PORT}/"
echo ""

# Test 413 (large body)
echo "Test 4: 413 Payload Too Large"
dd if=/dev/zero bs=1M count=10 2>/dev/null | curl -s -o /dev/null -w "Status: %{http_code}\n" -X POST -d @- "http://${HOST}:${PORT}/upload"
echo ""

# Test 414 (long URI)
echo "Test 5: 414 URI Too Long"
LONG_URI=$(python3 -c "print('A' * 10000)")
curl -s -o /dev/null -w "Status: %{http_code}\n" "http://${HOST}:${PORT}/${LONG_URI}"
echo ""

echo "=============================================="
echo "Tests completed!"
echo ""
echo "To see the HTML error pages in your browser:"
echo "  - http://${HOST}:${PORT}/nonexistent (404)"
echo "  - http://${HOST}:${PORT}/forbidden (403)"
