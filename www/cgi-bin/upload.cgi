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

# Read the POST data
POST_DATA=$(cat)

# Extract boundary from Content-Type
BOUNDARY=$(echo "$CONTENT_TYPE" | grep -o 'boundary=[^;]*' | cut -d= -f2)

if [ -z "$BOUNDARY" ]; then
    echo '{"error": "No boundary found", "message": "Invalid multipart data"}'
    exit 1
fi

# Create temporary file for processing
TEMP_FILE="/tmp/upload_$$.tmp"
echo "$POST_DATA" > "$TEMP_FILE"

# Extract filename from the multipart data
FILENAME=$(grep -A 10 "filename=" "$TEMP_FILE" | head -1 | grep -o 'filename="[^"]*"' | cut -d'"' -f2)

if [ -z "$FILENAME" ]; then
    echo '{"error": "No filename found", "message": "Could not extract filename from upload"}'
    rm -f "$TEMP_FILE"
    exit 1
fi

# Sanitize filename (remove path components and dangerous characters)
FILENAME=$(basename "$FILENAME" | tr -d '/../' | sed 's/[^a-zA-Z0-9._-]/_/g')

if [ -z "$FILENAME" ]; then
    FILENAME="uploaded_file_$(date +%s)"
fi

# Extract file data (everything after the first empty line following Content-Type)
awk "
/Content-Type: / { 
    getline; 
    if (\$0 == \"\r\" || \$0 == \"\") {
        while ((getline) > 0) {
            if (\$0 ~ /^--$BOUNDARY/) break;
            print \$0;
        }
        exit;
    }
}" "$TEMP_FILE" > "$UPLOAD_DIR/$FILENAME"

# Check if file was created and has content
if [ -f "$UPLOAD_DIR/$FILENAME" ] && [ -s "$UPLOAD_DIR/$FILENAME" ]; then
    FILE_SIZE=$(stat -f%z "$UPLOAD_DIR/$FILENAME" 2>/dev/null || stat -c%s "$UPLOAD_DIR/$FILENAME" 2>/dev/null)
    echo "{\"success\": true, \"message\": \"File uploaded successfully\", \"filename\": \"$FILENAME\", \"size\": $FILE_SIZE}"
else
    echo '{"error": "Upload failed", "message": "Could not save file"}'
    rm -f "$UPLOAD_DIR/$FILENAME"
fi

# Clean up
rm -f "$TEMP_FILE"
