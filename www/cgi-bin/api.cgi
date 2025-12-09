#!/bin/bash

# Simple API CGI script for testing
echo "Content-Type: application/json; charset=UTF-8"
echo ""

# Handle different HTTP methods
case "$REQUEST_METHOD" in
    "GET")
        echo '{"message": "GET request successful", "timestamp": "'$(date -Iseconds)'", "method": "GET"}'
        ;;
    "POST")
        # Read POST data
        POST_DATA=$(cat)
        echo "{\"message\": \"POST request successful\", \"timestamp\": \"$(date -Iseconds)\", \"method\": \"POST\", \"received_data\": \"$POST_DATA\"}"
        ;;
    "DELETE")
        echo '{"message": "DELETE request successful", "timestamp": "'$(date -Iseconds)'", "method": "DELETE"}'
        ;;
    *)
        echo '{"error": "Method not supported", "method": "'$REQUEST_METHOD'"}'
        ;;
esac
