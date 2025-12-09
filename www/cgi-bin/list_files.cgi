#!/bin/bash

# List files CGI script
echo "Content-Type: application/json; charset=UTF-8"
echo ""

# Set upload directory
UPLOAD_DIR="/home/zarcross/goinfre/webserv/www/beboccas/uploads"

# Check if directory exists
if [ ! -d "$UPLOAD_DIR" ]; then
    echo '{"error": "Upload directory not found"}'
    exit 1
fi

# Start JSON array
echo "["

first=true
for file in "$UPLOAD_DIR"/*; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        # Get file size (try macOS stat first, then Linux stat)
        filesize=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
        
        if [ "$first" = true ]; then
            first=false
        else
            echo ","
        fi
        
        echo "  {\"name\": \"$filename\", \"size\": $filesize}"
    fi
done

# End JSON array
echo "]"
