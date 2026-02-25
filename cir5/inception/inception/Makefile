NAME := inception
COMPOSE := docker compose -p $(NAME) -f srcs/docker-compose.yml

DATA_DIR := /home/jaeholee/data
DB_DIR   := $(DATA_DIR)/mariadb
WP_DIR   := $(DATA_DIR)/wordpress

SECRETS_DIR := $(CURDIR)/secrets
SECRET_FILES := \
	$(SECRETS_DIR)/db_root_password.txt \
	$(SECRETS_DIR)/db_password.txt \
	$(SECRETS_DIR)/wp_admin_password.txt \
	$(SECRETS_DIR)/wp_user_password.txt

MARIADB_UID := 100
MARIADB_GID := 101
WP_UID      := 33
WP_GID      := 33

all: up

check:
	@set -e; \
	echo "[check] validate secrets + data paths"; \
	test -d "$(SECRETS_DIR)" || (echo "ERROR: missing dir: $(SECRETS_DIR)"; exit 1); \
	for f in $(SECRET_FILES); do \
		test -f $$f || (echo "ERROR: missing secret file: $$f"; exit 1); \
	done; \
	echo "[check] enforce permissions (600)"; \
	for f in $(SECRET_FILES); do \
		sudo chmod 600 $$f; \
	done; \
	echo "[check] OK"

prepare:
	@set -e; \
	echo "[prepare] create data directories"; \
	sudo mkdir -p "$(DB_DIR)" "$(WP_DIR)"; \
	echo "[prepare] set ownership"; \
	sudo chown -R $(MARIADB_UID):$(MARIADB_GID) "$(DB_DIR)" || true; \
	sudo chown -R $(WP_UID):$(WP_GID) "$(WP_DIR)" || true; \
	echo "[prepare] set permissions"; \
	sudo chmod -R 750 "$(DB_DIR)" || true; \
	sudo chmod -R 755 "$(WP_DIR)" || true

up: check prepare
	$(COMPOSE) up -d --build
	$(COMPOSE) logs -f --tail=200

down:
	$(COMPOSE) down --remove-orphans

clean:
	$(COMPOSE) down --remove-orphans -v

fclean: clean
	$(COMPOSE) down --remove-orphans --rmi local -v || true
	sudo rm -rf "$(DB_DIR)"/* "$(WP_DIR)"/*

re: fclean up

ps:
	$(COMPOSE) ps

logs:
	$(COMPOSE) logs -f --tail=200

.PHONY: all check prepare up safe-up down clean fclean re ps logs

