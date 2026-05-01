#!/bin/sh
# cgi-bin/echo.sh
echo "Status: 200 OK"
echo "Content-Type: text/plain"
echo ""
echo "Method: $REQUEST_METHOD"
echo "URI: $REQUEST_URI"
echo "Query: $QUERY_STRING"
echo "Content-Length: $CONTENT_LENGTH"
echo "Content-Type: $CONTENT_TYPE"
echo ""
if [ "$REQUEST_METHOD" = "POST" ]; then
  cat
fi
