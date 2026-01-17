# **************************************************************************** #
#                               Inception Makefile                             #
# **************************************************************************** #

NAME        := inception
COMPOSE     := docker compose
COMPOSE_FILE:= srcs/docker-compose.yml

SERVICES    := mariadb wordpress nginx

.PHONY: all up down build re ps logs logs-% start stop restart \
        clean fclean prune help

all: up

up:
	$(COMPOSE) -f $(COMPOSE_FILE) up -d

build:
	$(COMPOSE) -f $(COMPOSE_FILE) up -d --build

re: fclean build

down:
	$(COMPOSE) -f $(COMPOSE_FILE) down

start:
	$(COMPOSE) -f $(COMPOSE_FILE) start

stop:
	$(COMPOSE) -f $(COMPOSE_FILE) stop

restart:
	$(COMPOSE) -f $(COMPOSE_FILE) restart

ps:
	$(COMPOSE) -f $(COMPOSE_FILE) ps

logs:
	$(COMPOSE) -f $(COMPOSE_FILE) logs -f --tail=200

logs-%:
	$(COMPOSE) -f $(COMPOSE_FILE) logs -f --tail=200 $*

clean:
	$(COMPOSE) -f $(COMPOSE_FILE) down

# WARNING: deletes volumes (db/wp persistent data)
fclean:
	$(COMPOSE) -f $(COMPOSE_FILE) down -v

# WARNING: removes dangling images/containers/networks/volumes (global docker cleanup)
prune:
	docker system prune -af
	docker volume prune -f

help:
	@echo "Targets:"
	@echo "  make up        - start containers (detached)"
	@echo "  make build     - build images and start"
	@echo "  make down      - stop and remove containers/networks"
	@echo "  make ps        - show compose status"
	@echo "  make logs      - follow logs (tail 200)"
	@echo "  make logs-nginx|logs-wordpress|logs-mariadb - follow one service logs"
	@echo "  make restart   - restart services"
	@echo "  make clean     - same as down (keeps volumes)"
	@echo "  make fclean    - down + delete volumes (DB/WP data reset)"
	@echo "  make prune     - aggressive docker cleanup (global)"

