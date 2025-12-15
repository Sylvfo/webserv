#!/bin/bash
# Error testing CGI script

ERROR_TYPE="${QUERY_STRING:-normal}"

case "$ERROR_TYPE" in
    "404")
        echo "Status: 404 Not Found"
        echo "Content-Type: text/html; charset=UTF-8"
        echo ""
        echo "<html><head><meta charset='UTF-8'><title>404 Error Test</title></head>"
        echo "<body><h1>404 - Not Found</h1><p>This is a simulated 404 error from CGI</p></body></html>"
        ;;
    "500")
        echo "Status: 500 Internal Server Error"
        echo "Content-Type: text/html; charset=UTF-8"
        echo ""
        echo "<html><head><meta charset='UTF-8'><title>500 Error Test</title></head>"
        echo "<body><h1>500 - Internal Server Error</h1><p>This is a simulated 500 error from CGI</p></body></html>"
        ;;
    "redirect")
        echo "Status: 302 Found"
        echo "Location: /cgi-bin/test.cgi"
        echo "Content-Type: text/html; charset=UTF-8"
        echo ""
        echo "<html><head><meta charset='UTF-8'><title>Redirecting...</title></head>"
        echo "<body><p>Redirecting to test.cgi...</p></body></html>"
        ;;
    "crash")
        echo "Content-Type: text/html; charset=UTF-8"
        echo ""
        echo "<h1>About to crash...</h1>"
        # Force exit with error code
        exit 1
        ;;
    *)
        echo "Content-Type: text/html; charset=UTF-8"
        echo ""
        echo "<html>"
        echo "<head>"
        echo "<meta charset='UTF-8'>"
        echo "<title>Error Testing CGI</title>"
        echo "</head>"
        echo "<body style='font-family: Arial, sans-serif; margin: 20px;'>"
        echo "<h1>🚨 Error Testing CGI</h1>"
        echo "<p>Test different error conditions:</p>"
        echo "<ul>"
        echo "<li><a href='/cgi-bin/error_test.cgi?404'>Test 404 Not Found</a></li>"
        echo "<li><a href='/cgi-bin/error_test.cgi?500'>Test 500 Internal Server Error</a></li>"
        echo "<li><a href='/cgi-bin/error_test.cgi?redirect'>Test 302 Redirect</a></li>"
        echo "<li><a href='/cgi-bin/error_test.cgi?crash'>Test Script Crash</a></li>"
        echo "</ul>"
        echo "<p><em>These tests help verify how your webserver handles different CGI error conditions.</em></p>"
        echo "</body>"
        echo "</html>"
        ;;
esac
