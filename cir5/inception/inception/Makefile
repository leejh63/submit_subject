NAME := inception

DATA_DIR := /home/jaeholee/data
DB_DIR := $(DATA_DIR)/mariadb
WP_DIR := $(DATA_DIR)/wordpress

# default UIDs/GIDs (Debian 기준 흔한 값)
MARIADB_UID := 100
MARIADB_GID := 101
WP_UID      := 33
WP_GID      := 33

COMPOSE := docker compose -f srcs/docker-compose.yml

all: up

prepare:
	@echo "[prepare] create data directories"
	@sudo mkdir -p "$(DB_DIR)" "$(WP_DIR)"

	@echo "[prepare] set ownership"
	@sudo chown -R $(MARIADB_UID):$(MARIADB_GID) "$(DB_DIR)" || true
	@sudo chown -R $(WP_UID):$(WP_GID) "$(WP_DIR)" || true

	@echo "[prepare] set permissions"
	@sudo chmod -R 750 "$(DB_DIR)" || true
	@sudo chmod -R 755 "$(WP_DIR)" || true
	
	@echo "Default container UID/GID values may differ depending on the base image."
	@echo "If permission issues occur, verify the container user IDs and adjust accordingly."

up: prepare
	$(COMPOSE) up -d --build

down:
	$(COMPOSE) down --remove-orphans

clean:
	$(COMPOSE) down --remove-orphans -v

fclean: clean
	docker rmi -f $$(docker images -q)
	sudo rm -rf /home/jaeholee/data/mariadb/*
	sudo rm -rf /home/jaeholee/data/wordpress/*

re: fclean up

ps:
	$(COMPOSE) ps

logs:
	$(COMPOSE) logs -f --tail=200

.PHONY: all up down clean fclean re ps logs
