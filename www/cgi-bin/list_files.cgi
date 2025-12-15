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

# Use Python to generate properly formatted JSON
python3 << 'PYTHON_SCRIPT'
import json
import os
import sys

upload_dir = '/home/zarcross/goinfre/webserv/www/beboccas/uploads'

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
