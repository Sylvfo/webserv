# How to Use nginx-webserv Container

Hello and welcome to the nginx-webserv container!

In the folder `docker-stuff` there are all the necessary files to build and run a container
with an nginx server with some placeholder config and config file.

It runs on HTTP 1.0, the same version which will be used in webserv.

---

## Prerequisites

- Docker must be installed

---

## Getting Started

### 1. Navigation
Navigate to the docker-stuff directory:

### 2. Fix Permissions (if needed)
If you run into problems, this could help:
```bash
chmod +x start.sh
```

### 3. Build the Container
```bash
docker build -t nginx-webserv .
```

### 4. Run the Container

**Run in background:**
```bash
docker run -d -p 8080:80 --name custom-name nginx-webserv
```

**Run in foreground:**
```bash
docker run -p 8080:80 --name custom-name nginx-webserv
```

---

## Basic Testing

### Check if container is reachable:
```bash
curl http://localhost:8080/
```

### Check if container is running:
```bash
docker ps -a
```

### Stop the container:
```bash
docker stop container-name
```

---

## Advanced Testing Commands

### Test Static Page
```bash
curl http://localhost:8080/
```

### Test CGI Echo (GET)
```bash
curl http://localhost:8080/cgi-bin/echo
```

### Test CGI Echo (POST)
```bash
curl -X POST -d "name=test&value=123" http://localhost:8080/cgi-bin/echo
```

### Test Directory Listing
```bash
curl http://localhost:8080/docs/
```

### Test 404 Error
```bash
curl http://localhost:8080/nonexistent
```

### Test in Browser
Open: http://localhost:8080

---

## Container Management Commands

### View Container Information
```bash
# List running containers
docker ps

# List all containers (including stopped)
docker ps -a

# View container logs
docker logs webserv-test

# Follow logs in real-time
docker logs -f webserv-test

# View container details
docker inspect webserv-test

# List images
docker images
```

### Execute Commands Inside Container
```bash
# Open bash shell inside running container
docker exec -it webserv-test bash

# Test nginx config inside container
docker exec webserv-test nginx -t
```

---

## Start/Stop/Remove Commands

### Basic Control
```bash
# Stop the container
docker stop webserv-test

# Start a stopped container
docker start webserv-test

# Restart the container
docker restart webserv-test
```

### Remove Containers and Images
```bash
# Remove the container (must be stopped first)
docker rm webserv-test

# Force remove running container
docker rm -f webserv-test

# Remove the image
docker rmi nginx-webserv

# Remove all stopped containers
docker container prune

# Remove unused images
docker image prune
```

---

## Quick Rebuild and Restart

### Rebuild and run in background:
```bash
docker stop webserv-test && docker rm webserv-test && docker build -t nginx-webserv . && docker run -d -p 8080:80 --name webserv-test nginx-webserv
```

### Rebuild and run in foreground:
```bash
docker stop webserv-test && docker rm webserv-test && docker build -t nginx-webserv . && docker run -p 8080:80 --name webserv-test nginx-webserv
```

---

## Troubleshooting

### Check Why Container Exited
```bash
docker logs webserv-test
```

### Check Container Status
```bash
docker ps -a | grep webserv-test
```

### Inspect Container Configuration
```bash
docker inspect webserv-test
```

### Debug Inside Container
```bash
# Get into running container for debugging
docker exec -it webserv-test bash

# Test nginx config
docker exec webserv-test nginx -t
```

---

## Notes

- Replace `custom-name` or `webserv-test` with your preferred container name
- Port 8080 on host maps to port 80 in container
- Use `-d` flag to run in detached/background mode
- Use `docker logs -f` to follow logs in real-time