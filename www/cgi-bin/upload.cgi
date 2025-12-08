#!/bin/bash
# File upload test CGI

echo "Content-Type: text/html; charset=UTF-8"
echo ""
echo "<html>"
echo "<head>"
echo "<meta charset='UTF-8'>"
echo "<title>File Upload Test</title>"
echo "</head>"
echo "<body style='font-family: Arial, sans-serif; margin: 20px;'>"
echo "<h1>📁 File Upload CGI Test</h1>"

if [ "$REQUEST_METHOD" = "GET" ]; then
    echo "<h2>Upload a File</h2>"
    echo "<form method='POST' action='/cgi-bin/upload.cgi' enctype='multipart/form-data'>"
    echo "<p>Select file: <input type='file' name='upload'></p>"
    echo "<p>Description: <input type='text' name='description' placeholder='File description'></p>"
    echo "<p><input type='submit' value='Upload File'></p>"
    echo "</form>"
    
elif [ "$REQUEST_METHOD" = "POST" ]; then
    echo "<h2>Upload Processing</h2>"
    echo "<p><strong>Content-Type:</strong> $CONTENT_TYPE</p>"
    echo "<p><strong>Content-Length:</strong> $CONTENT_LENGTH bytes</p>"
    
    if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
        echo "<h3>Received Data:</h3>"
        echo "<p>⚠️ This is a demo - file data would normally be processed here</p>"
        echo "<pre style='background: #f5f5f5; padding: 10px; border: 1px solid #ddd; max-height: 200px; overflow-y: scroll;'>"
        # Read limited amount for demo (first 500 chars)
        head -c 500
        if [ "$CONTENT_LENGTH" -gt 500 ]; then
            echo ""
            echo "... (truncated - total size: $CONTENT_LENGTH bytes)"
        fi
        echo "</pre>"
        
        # Create upload directory if it doesn't exist
        mkdir -p ../uploads
        echo "<p>✅ Upload directory ready: ../uploads/</p>"
        echo "<p>📝 In a real implementation, the file would be saved here</p>"
    else
        echo "<p>❌ No data received</p>"
    fi
    
    echo "<p><a href='/cgi-bin/upload.cgi'>← Back to upload form</a></p>"
fi

echo "</body>"
echo "</html>"
