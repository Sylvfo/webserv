#!/bin/bash

# List files CGI script
echo "Content-Type: application/json; charset=UTF-8"
echo ""

# Set upload directory using DOCUMENT_ROOT or PWD as base
# DOCUMENT_ROOT is typically set by the web server
if [ -n "$DOCUMENT_ROOT" ]; then
    UPLOAD_DIR="$DOCUMENT_ROOT/beboccas/uploads"
else
    # Fallback: assume CGI is in www/cgi-bin, so uploads is at ../beboccas/uploads
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
    UPLOAD_DIR="$SCRIPT_DIR/../beboccas/uploads"
fi

# Check if directory exists
if [ ! -d "$UPLOAD_DIR" ]; then
    echo '{"error": "Upload directory not found"}'
    exit 1
fi

# Use Python to generate properly formatted JSON
python3 << PYTHON_SCRIPT
import json
import os
import sys

upload_dir = '$UPLOAD_DIR'

try:
    files = []
    for filename in os.listdir(upload_dir):
        filepath = os.path.join(upload_dir, filename)
        if os.path.isfile(filepath):
            filesize = os.path.getsize(filepath)
            files.append({
                'name': filename,
                'size': filesize
            })
    
    print(json.dumps(files, ensure_ascii=False, indent=2))
except Exception as e:
    print(json.dumps({'error': str(e)}))
    sys.exit(1)

PYTHON_SCRIPT
