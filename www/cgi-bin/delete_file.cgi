#!/bin/bash

# Delete file CGI script
echo "Content-Type: application/json; charset=UTF-8"
echo ""

# Set upload directory using DOCUMENT_ROOT or relative path
if [ -n "$DOCUMENT_ROOT" ]; then
    UPLOAD_DIR="$DOCUMENT_ROOT/beboccas/uploads"
else
    # Fallback: assume CGI is in www/cgi-bin, so uploads is at ../beboccas/uploads
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    UPLOAD_DIR="$SCRIPT_DIR/../beboccas/uploads"
fi

# Check if this is a POST request
if [ "$REQUEST_METHOD" != "POST" ]; then
    echo '{"error": "Method not allowed", "message": "Only POST method is supported"}'
    exit 1
fi

# Read POST data
POST_DATA=$(cat)

# Extract filename from POST data (format: filename=somefile.txt)
FILENAME_ENCODED=$(echo "$POST_DATA" | grep -o 'filename=[^&]*' | cut -d= -f2)

if [ -z "$FILENAME_ENCODED" ]; then
    echo '{"error": "No filename provided", "message": "filename parameter is required"}'
    exit 1
fi

# URL decode filename using Python for proper UTF-8 handling
FILENAME=$(python3 -c "
import sys
try:
    from urllib.parse import unquote_plus
    print(unquote_plus('$FILENAME_ENCODED'))
except Exception as e:
    sys.exit(1)
")

if [ -z "$FILENAME" ]; then
    echo '{"error": "Failed to decode filename", "message": "Could not decode the filename"}'
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
