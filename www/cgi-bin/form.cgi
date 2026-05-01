#!/bin/bash
# Form handling CGI script

echo "Content-Type: text/html; charset=UTF-8"
echo ""
echo "<html>"
echo "<head>"
echo "<meta charset='UTF-8'>"
echo "<title>CGI Form Test</title>"
echo "</head>"
echo "<body style='font-family: Arial, sans-serif; margin: 20px;'>"
echo "<h1>📝 CGI Form Handler</h1>"

if [ "$REQUEST_METHOD" = "GET" ]; then
    echo "<h2>GET Request - Query Parameters</h2>"
    if [ -n "$QUERY_STRING" ]; then
        echo "<p><strong>Query String:</strong> $QUERY_STRING</p>"
        # Parse query string parameters
        IFS='&' read -ra PARAMS <<< "$QUERY_STRING"
        echo "<h3>Parsed Parameters:</h3>"
        echo "<ul>"
        for param in "${PARAMS[@]}"; do
            IFS='=' read -ra PAIR <<< "$param"
            name="${PAIR[0]}"
            value="${PAIR[1]}"
            # URL decode (basic)
            value=$(echo "$value" | sed 's/+/ /g' | sed 's/%20/ /g')
            echo "<li><strong>$name:</strong> $value</li>"
        done
        echo "</ul>"
    else
        echo "<p>No parameters provided</p>"
    fi
    
    echo "<h2>Test Form</h2>"
    echo "<form method='GET' action='/cgi-bin/form.cgi'>"
    echo "<p>Name: <input type='text' name='name' placeholder='Enter your name'></p>"
    echo "<p>Email: <input type='email' name='email' placeholder='Enter your email'></p>"
    echo "<p>Message: <textarea name='message' placeholder='Enter your message'></textarea></p>"
    echo "<p><input type='submit' value='Submit via GET'></p>"
    echo "</form>"
    
    echo "<form method='POST' action='/cgi-bin/form.cgi'>"
    echo "<p>Name: <input type='text' name='name' placeholder='Enter your name'></p>"
    echo "<p>Email: <input type='email' name='email' placeholder='Enter your email'></p>"
    echo "<p>Message: <textarea name='message' placeholder='Enter your message'></textarea></p>"
    echo "<p><input type='submit' value='Submit via POST'></p>"
    echo "</form>"
    
elif [ "$REQUEST_METHOD" = "POST" ]; then
    echo "<h2>POST Request - Form Data</h2>"
    echo "<p><strong>Content-Type:</strong> $CONTENT_TYPE</p>"
    echo "<p><strong>Content-Length:</strong> $CONTENT_LENGTH</p>"
    
    if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
        echo "<h3>POST Data Received:</h3>"
        echo "<pre style='background: #f5f5f5; padding: 10px; border: 1px solid #ddd;'>"
        # Read POST data from stdin
        read -n $CONTENT_LENGTH post_data
        echo "Raw data: $post_data"
        echo "</pre>"
        
        # Parse POST data
        IFS='&' read -ra PARAMS <<< "$post_data"
        echo "<h3>Parsed Form Fields:</h3>"
        echo "<ul>"
        for param in "${PARAMS[@]}"; do
            IFS='=' read -ra PAIR <<< "$param"
            name="${PAIR[0]}"
            value="${PAIR[1]}"
            # URL decode (basic)
            value=$(echo "$value" | sed 's/+/ /g' | sed 's/%20/ /g')
            echo "<li><strong>$name:</strong> $value</li>"
        done
        echo "</ul>"
    else
        echo "<p>No POST data received</p>"
    fi
    
    echo "<p><a href='/cgi-bin/form.cgi'>← Back to form</a></p>"
fi

echo "</body>"
echo "</html>"
