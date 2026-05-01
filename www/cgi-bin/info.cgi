#!/bin/bash
# Simple info CGI script

echo "Content-Type: text/html; charset=UTF-8"
echo ""
echo "<html>"
echo "<head>"
echo "<meta charset='UTF-8'>"
echo "<title>CGI Environment Info</title>"
echo "</head>"
echo "<body style='font-family: Arial, sans-serif; margin: 20px;'>"
echo "<h1>🚀 CGI Environment Information</h1>"
echo "<h2>📋 Request Details</h2>"
echo "<table border='1' style='border-collapse: collapse; width: 100%;'>"
echo "<tr><td><strong>Request Method</strong></td><td>$REQUEST_METHOD</td></tr>"
echo "<tr><td><strong>Request URI</strong></td><td>$REQUEST_URI</td></tr>"
echo "<tr><td><strong>Query String</strong></td><td>$QUERY_STRING</td></tr>"
echo "<tr><td><strong>Script Name</strong></td><td>$SCRIPT_NAME</td></tr>"
echo "<tr><td><strong>HTTP Host</strong></td><td>$HTTP_HOST</td></tr>"
echo "<tr><td><strong>HTTP User Agent</strong></td><td>$HTTP_USER_AGENT</td></tr>"
echo "<tr><td><strong>Content Type</strong></td><td>$CONTENT_TYPE</td></tr>"
echo "<tr><td><strong>Content Length</strong></td><td>$CONTENT_LENGTH</td></tr>"
echo "</table>"

echo "<h2>🌐 All Environment Variables</h2>"
echo "<pre style='background: #f5f5f5; padding: 10px; border: 1px solid #ddd;'>"
env | sort
echo "</pre>"

echo "<h2>⏰ Current Time</h2>"
echo "<p>Server time: $(date)</p>"

echo "</body>"
echo "</html>"
