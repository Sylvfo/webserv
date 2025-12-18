#!/bin/bash
# Cookie handling CGI script

echo "Content-Type: text/html; charset=UTF-8"

# Check for existing cookie
COOKIE_VALUE=""
if [ -n "$HTTP_COOKIE" ]; then
    # Extract visitor count from cookie (simple parsing)
    COOKIE_VALUE=$(echo "$HTTP_COOKIE" | grep -o 'visitor_count=[0-9]*' | cut -d'=' -f2)
fi

# Increment visitor count
if [ -z "$COOKIE_VALUE" ]; then
    NEW_COUNT=1
else
    NEW_COUNT=$((COOKIE_VALUE + 1))
fi

# Set new cookie
echo "Set-Cookie: visitor_count=$NEW_COUNT; path=/; Max-Age=86400"
echo ""

echo "<html>"
echo "<head>"
echo "<meta charset='UTF-8'>"
echo "<title>Cookie Test CGI</title>"
echo "</head>"
echo "<body style='font-family: Arial, sans-serif; margin: 20px;'>"
echo "<h1>🍪 Cookie Handling Test</h1>"

echo "<div style='background: #e7f3ff; border: 1px solid #bee5eb; padding: 15px; margin: 10px 0; border-radius: 5px;'>"
echo "<h2>👤 Visitor Information</h2>"
echo "<p><strong>Visit Count:</strong> $NEW_COUNT</p>"
echo "<p><strong>Previous Count:</strong> ${COOKIE_VALUE:-'First visit'}</p>"
echo "<p><strong>Cookie Header:</strong> ${HTTP_COOKIE:-'No cookies'}</p>"
echo "</div>"

echo "<h2>🔄 Actions</h2>"
echo "<p><a href='/cgi-bin/cookie.cgi'>🔄 Refresh Page (increment counter)</a></p>"
echo "<p><a href='/cgi-bin/cookie.cgi' onclick=\"document.cookie='visitor_count=; expires=Thu, 01 Jan 1970 00:00:00 GMT; path=/'; return true;\">🗑️ Clear Cookie & Refresh</a></p>"

echo "<h2>📊 Cookie Details</h2>"
echo "<table border='1' style='border-collapse: collapse; width: 100%;'>"
echo "<tr><td><strong>Cookie Name</strong></td><td>visitor_count</td></tr>"
echo "<tr><td><strong>Current Value</strong></td><td>$NEW_COUNT</td></tr>"
echo "<tr><td><strong>path</strong></td><td>/</td></tr>"
echo "<tr><td><strong>Max Age</strong></td><td>86400 seconds (24 hours)</td></tr>"
echo "</table>"

echo "<h2>🧪 Test Instructions</h2>"
echo "<ol>"
echo "<li>Refresh this page multiple times to see the counter increment</li>"
echo "<li>Close your browser and come back - the count should persist</li>"
echo "<li>Click 'Clear Cookie' to reset the counter</li>"
echo "<li>Check your browser's developer tools to see the cookie being set</li>"
echo "</ol>"

echo "</body>"
echo "</html>"
