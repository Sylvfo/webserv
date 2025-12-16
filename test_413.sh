#!/bin/bash

# Test script for 413 Payload Too Large error

PORT=8080
HOST="localhost"

echo "==================================="
echo "Test 413 - Payload Too Large"
echo "==================================="

# Test 1: POST with Content-Length exceeding limit (100KB limit, send 150KB declared)
echo -e "\n--- Test 1: Content-Length too large (150KB declared) ---"
# Create a large payload (actually small, but declare 150KB)
curl -v -X POST "http://$HOST:$PORT/" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -H "Content-Length: 150000" \
  --data-raw "test=data" \
  2>&1 | grep -E "(HTTP|413|Payload)"

echo -e "\n--- Test 2: Actually sending too much data (120KB) ---"
# Generate 120KB of data (above 100KB limit)
LARGE_DATA=$(python3 -c "print('x' * 120000)")
curl -v -X POST "http://$HOST:$PORT/" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  --data-raw "data=$LARGE_DATA" \
  2>&1 | grep -E "(HTTP|413|Payload|ERR_)"

echo -e "\n--- Test 3: Normal size data (should work - 50KB) ---"
NORMAL_DATA=$(python3 -c "print('y' * 50000)")
curl -v -X POST "http://$HOST:$PORT/" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  --data-raw "data=$NORMAL_DATA" \
  2>&1 | grep -E "(HTTP|413|200|404)"

echo -e "\n==================================="
echo "Tests completed"
echo "==================================="
