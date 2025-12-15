#!/bin/bash

# Upload CGI script
echo "Content-Type: application/json; charset=UTF-8"
echo ""

# Set upload directory
UPLOAD_DIR="/home/zarcross/goinfre/webserv/www/beboccas/uploads"

# Create upload directory if it doesn't exist
mkdir -p "$UPLOAD_DIR"

# Check if this is a POST request
if [ "$REQUEST_METHOD" != "POST" ]; then
    echo '{"error": "Method not allowed", "message": "Only POST method is supported"}'
    exit 1
fi

# Check if Content-Type contains multipart/form-data
if [[ "$CONTENT_TYPE" != *"multipart/form-data"* ]]; then
    echo '{"error": "Invalid content type", "message": "Expected multipart/form-data"}'
    exit 1
fi

# Extract boundary from Content-Type
BOUNDARY=$(echo "$CONTENT_TYPE" | grep -o 'boundary=[^;]*' | cut -d= -f2)

if [ -z "$BOUNDARY" ]; then
    echo '{"error": "No boundary found", "message": "Invalid multipart data"}'
    exit 1
fi

# Create temporary file for processing - read directly to file to preserve binary data
TEMP_FILE="/tmp/upload_$$.tmp"
cat > "$TEMP_FILE"

# Extract filename from the multipart data (text-only operation, safe)
FILENAME=$(grep -aoP 'filename="\K[^"]+' "$TEMP_FILE" 2>/dev/null | head -1)

# Fallback if grep -P doesn't work
if [ -z "$FILENAME" ]; then
    FILENAME=$(grep -a "filename=" "$TEMP_FILE" | head -1 | sed 's/.*filename="\([^"]*\)".*/\1/')
fi

if [ -z "$FILENAME" ]; then
    echo '{"error": "No filename found", "message": "Could not extract filename from upload"}'
    rm -f "$TEMP_FILE"
    exit 1
fi

# Sanitize filename (remove path components and dangerous characters)
# Remove any path traversal attempts and sanitize
FILENAME=$(basename "$FILENAME")
FILENAME=$(echo "$FILENAME" | sed 's/\.\.\///g')
FILENAME=$(echo "$FILENAME" | sed 's/[^a-zA-Z0-9._-]/_/g')

if [ -z "$FILENAME" ]; then
    FILENAME="uploaded_file_$(date +%s)"
fi

# Use Python to extract the binary file data safely
FILE_SIZE=$(python3 -c "
import sys
import re

temp_file = '$TEMP_FILE'
boundary = '$BOUNDARY'
output_file = '$UPLOAD_DIR/$FILENAME'

try:
    with open(temp_file, 'rb') as f:
        data = f.read()
    
    # Find the file content between Content-Type and boundary
    pattern = b'Content-Type: [^\r\n]+\r?\n\r?\n'
    match = re.search(pattern, data)
    
    if not match:
        sys.exit(1)
    
    start = match.end()
    
    # Find the ending boundary
    end_boundary = b'\r\n--' + boundary.encode()
    end = data.find(end_boundary, start)
    
    if end == -1:
        end_boundary = b'--' + boundary.encode()
        end = data.find(end_boundary, start)
    
    if end == -1:
        sys.exit(1)
    
    # Extract the file data
    file_data = data[start:end]
    
    # Write to output file
    with open(output_file, 'wb') as f:
        f.write(file_data)
    
    print(len(file_data))
    
except Exception as e:
    sys.exit(1)
" 2>/dev/null)

# Check if file was created and has content
if [ -f "$UPLOAD_DIR/$FILENAME" ] && [ -s "$UPLOAD_DIR/$FILENAME" ]; then
    if [ -z "$FILE_SIZE" ]; then
        FILE_SIZE=$(stat -c%s "$UPLOAD_DIR/$FILENAME" 2>/dev/null || stat -f%z "$UPLOAD_DIR/$FILENAME" 2>/dev/null)
    fi
    echo "{\"success\": true, \"message\": \"File uploaded successfully\", \"filename\": \"$FILENAME\", \"size\": $FILE_SIZE}"
else
    echo '{"error": "Upload failed", "message": "Could not save file"}'
    rm -f "$UPLOAD_DIR/$FILENAME"
fi

# Clean up
rm -f "$TEMP_FILE"
