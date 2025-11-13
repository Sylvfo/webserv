# cgi-bin/upload.sh
#!/bin/sh
# Writes POST body to a timestamped file in /var/www/html/uploads
UPLOAD_DIR="/var/www/html/uploads"
mkdir -p "$UPLOAD_DIR"
TS=$(date +%s)
FILE="$UPLOAD_DIR/upload-$TS.bin"

# Read exactly CONTENT_LENGTH bytes from stdin to avoid hanging
if [ -n "$CONTENT_LENGTH" ]; then
  dd bs=1 count="$CONTENT_LENGTH" of="$FILE" 2>/dev/null
fi

echo "Status: 201 Created"
echo "Content-Type: text/plain"
echo ""
echo "Saved to: /uploads/$(basename "$FILE")"
