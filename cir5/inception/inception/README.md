*This project has been created as part of the 42 curriculum by jaeholee.*

# Inception

## Description
Inception is a system administration and DevOps project whose objective is to design,
build, and deploy a small but complete web infrastructure using Docker and Docker Compose.

The goal of the project is to understand how modern container-based infrastructures work,
how services can be isolated while still communicating with each other, and how data
persistence and security can be ensured in a containerized environment.

The stack is composed of the following services:
- **NGINX** as a reverse proxy, exposing the application securely through HTTPS only
- **WordPress** running with **php-fpm**
- **MariaDB** acting as the database backend for WordPress

Each service runs inside its own dedicated container and is built from scratch using
custom Dockerfiles based on Debian images.

---

## Instructions

### Prerequisites
- A Linux system (the project is evaluated inside a Virtual Machine)
- Docker
- Docker Compose

### Build and Run
From the root of the repository, run:
make

This command:
- Builds all Docker images
- Creates the Docker network and volumes
- Starts all services in detached mode

### Stop the Project
make down

### Restart the Project
make re

---

## Project Description and Design Choices

### Use of Docker
Docker is used to containerize each service in order to:
- Isolate dependencies
- Simplify deployment
- Ensure reproducibility across environments

Docker Compose is used to orchestrate the services, define their relationships, networks,
and volumes, and provide a single entry point for managing the stack.

### Virtual Machines vs Docker
- **Virtual Machines** virtualize an entire operating system, including the kernel.
- **Docker containers** share the host kernel and isolate only the application layer.
- Docker containers are lighter, start faster, and consume fewer resources than virtual machines.

### Secrets vs Environment Variables
- **Environment variables** are easy to use but can expose sensitive information.
- **Docker secrets** provide a more secure way to store and inject credentials at runtime.
- This project uses Docker secrets for all sensitive data such as database and WordPress passwords.

### Docker Network vs Host Network
- **Host networking** removes isolation and can expose services directly to the host system.
- **Docker bridge networks** provide isolation, service discovery, and controlled communication.
- This project uses a dedicated Docker bridge network to allow containers to communicate safely.

### Docker Volumes vs Bind Mounts
- **Bind mounts** map specific host directories directly into containers.
- **Docker volumes** are managed by Docker and integrate better with container lifecycles.
- This project uses **named Docker volumes with bind options**, allowing:
  - Persistent storage under `/home/jaeholee/data`
  - Compatibility with `docker volume inspect`
  - Data persistence across container restarts and system reboots

---

## Resources

### References
- Docker Documentation: https://docs.docker.com/
- Docker Compose Documentation: https://docs.docker.com/compose/
- NGINX Documentation: https://nginx.org/en/docs/
- WordPress Documentation: https://wordpress.org/support/
- MariaDB Documentation: https://mariadb.org/documentation/

### AI Usage
AI tools (ChatGPT) were used during the development of this project to:
- Review Docker and Docker Compose configurations
- Validate infrastructure design choices
- Help structure and verify project documentation

All architectural decisions, configurations, and final implementations were fully
understood, reviewed, and validated by the author.
