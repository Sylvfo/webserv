#!/bin/bash
# Start fcgiwrap
spawn-fcgi -s /var/run/fcgiwrap.socket -u www-data -g www-data /usr/sbin/fcgiwrap

# Replace shell with nginx (exec makes nginx PID 1)
exec nginx -g 'daemon off;'
