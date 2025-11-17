FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        nginx \
        fcgiwrap \
        spawn-fcgi \
        curl \
        ca-certificates \
        && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /var/www/html /var/www/cgi-bin /var/www/errors /var/log/nginx

# Only copy start script (files come from volumes now)
COPY start.sh /start.sh
RUN chmod +x /start.sh

EXPOSE 80

CMD ["/start.sh"]
