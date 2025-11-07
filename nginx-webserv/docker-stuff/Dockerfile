# Use Ubuntu slim as base
FROM ubuntu:22.04

# Prevent interactive prompts during install
ENV DEBIAN_FRONTEND=noninteractive

# Install nginx, fcgiwrap, spawn-fcgi, curl for testing
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        nginx \
        fcgiwrap \
        spawn-fcgi \
        curl \
        ca-certificates \
        && rm -rf /var/lib/apt/lists/*

# Create dirs expected by our config
RUN mkdir -p /var/www/html /var/www/cgi-bin /var/www/errors /var/log/nginx

# Copy site files
COPY start.sh /start.sh
COPY html/ /var/www/html/
COPY cgi-bin/ /var/www/cgi-bin/
COPY errors/ /var/www/errors/
COPY nginx.conf /etc/nginx/nginx.conf

# Make start.sh executable
RUN chmod +x /start.sh

# Ensure CGI scripts are executable
RUN chmod +x /var/www/cgi-bin/*.sh

# Expose HTTP
EXPOSE 80

# Run fcgiwrap and nginx in foreground
CMD ["/start.sh"]
