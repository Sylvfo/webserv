#!/bin/bash

# Delete file CGI script
echo "Content-Type: application/json; charset=UTF-8"
echo ""

# Set upload directory
UPLOAD_DIR="/home/zarcross/goinfre/webserv/www/beboccas/uploads"

# Check if this is a POST request
if [ "$REQUEST_METHOD" != "POST" ]; then
    echo '{"error": "Method not allowed", "message": "Only POST method is supported"}'
    exit 1
fi

# Read POST data
POST_DATA=$(cat)

# Extract filename from POST data (format: filename=somefile.txt)
FILENAME=$(echo "$POST_DATA" | grep -o 'filename=[^&]*' | cut -d= -f2 | sed 's/%20/ /g' | sed 's/+/ /g')

# URL decode filename (basic decoding)
FILENAME=$(echo "$FILENAME" | sed 's/%2F/\//g' | sed 's/%2E/./g' | sed 's/%2D/-/g' | sed 's/%5F/_/g')

if [ -z "$FILENAME" ]; then
    echo '{"error": "No filename provided", "message": "filename parameter is required"}'
    exit 1
fi

# Sanitize filename (remove path components and ensure it's just a filename)
FILENAME=$(basename "$FILENAME")

# Check if file exists
FULL_PATH="$UPLOAD_DIR/$FILENAME"

if [ ! -f "$FULL_PATH" ]; then
    echo "{\"error\": \"File not found\", \"message\": \"File '$FILENAME' does not exist\"}"
    exit 1
fi

# Attempt to delete the file
if rm "$FULL_PATH" 2>/dev/null; then
    echo "{\"success\": true, \"message\": \"File '$FILENAME' deleted successfully\"}"
else
    echo "{\"error\": \"Deletion failed\", \"message\": \"Could not delete file '$FILENAME'\"}"
fi
