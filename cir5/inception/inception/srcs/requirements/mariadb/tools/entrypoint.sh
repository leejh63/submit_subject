#!/bin/sh
set -eu

DB_DIR="/var/lib/mysql"
SOCK_DIR="/run/mysqld"
SOCK="${SOCK_DIR}/mysqld.sock"

log() { printf "[mariadb-init] %s\n" "$*"; }
die() { printf "[mariadb-init] ERROR: %s\n" "$*" >&2; exit 1; }

read_secret() { [ -f "$1" ] && tr -d '\r\n' < "$1" || printf ""; }

# SQL single-quote escape:  foo'bar  ->  foo''bar
sql_escape() { printf "%s" "$1" | sed "s/'/''/g"; }

# --- required env (from .env)
: "${MYSQL_DATABASE:?MYSQL_DATABASE not set}"
: "${MYSQL_USER:?MYSQL_USER not set}"

# --- secrets
ROOT_PW="$(read_secret /run/secrets/db_root_password)"
DB_PW="$(read_secret /run/secrets/db_password)"

[ -n "$ROOT_PW" ] || die "missing secret: db_root_password"
[ -n "$DB_PW" ]   || die "missing secret: db_password"

DB_ESC="$(sql_escape "$MYSQL_DATABASE")"
USER_ESC="$(sql_escape "$MYSQL_USER")"
DBPW_ESC="$(sql_escape "$DB_PW")"
ROOT_ESC="$(sql_escape "$ROOT_PW")"

mkdir -p "$SOCK_DIR"
chown -R mysql:mysql "$SOCK_DIR" 2>/dev/null || true
chown -R mysql:mysql "$DB_DIR" 2>/dev/null || true

# 1) init datadir if empty
if [ ! -d "$DB_DIR/mysql" ]; then
  log "datadir is empty -> initializing"
  mariadb-install-db --user=mysql --datadir="$DB_DIR" >/dev/null
else
  log "datadir exists -> skip init"
fi

# 2) start temp server (socket only)
log "starting temp server (socket only, no TCP)"
mariadbd --user=mysql --datadir="$DB_DIR" --skip-networking --socket="$SOCK" &
TEMP_PID="$!"

# 3) wait ready
log "waiting for temp server ready..."
i=0
while ! mariadb-admin --protocol=socket --socket="$SOCK" ping >/dev/null 2>&1; do
  i=$((i+1))
  if [ "$i" -ge 150 ]; then
    kill "$TEMP_PID" 2>/dev/null || true
    die "temp mariadb not ready"
  fi
  sleep 0.2
done
log "temp server ready"

# 4) decide how to login as root for bootstrap
#    - try no-password first
MYSQL_BASE="mariadb --protocol=socket --socket=$SOCK -uroot"
MYSQLADMIN_BASE="mariadb-admin --protocol=socket --socket=$SOCK -uroot"

if $MYSQL_BASE -e "SELECT 1" >/dev/null 2>&1; then
  log "root login: no password"
  MYSQL_CMD="$MYSQL_BASE"
  MYSQLADMIN_CMD="$MYSQLADMIN_BASE"
else
  # use MYSQL_PWD to avoid shell parsing issues with special chars
  export MYSQL_PWD="$ROOT_PW"
  if $MYSQL_BASE -e "SELECT 1" >/dev/null 2>&1; then
    log "root login: with ROOT_PW"
    MYSQL_CMD="$MYSQL_BASE"
    MYSQLADMIN_CMD="$MYSQLADMIN_BASE"
  else
    # datadir exists + cannot login => mismatch secrets vs existing volume
    if [ -d "$DB_DIR/mysql" ]; then
      die "existing datadir but cannot login as root with provided secret. (volume/password mismatch) -> wipe volume or use correct secret"
    fi
    die "cannot login as root during bootstrap"
  fi
fi

# 5) apply schema/users/grants (idempotent + version-safe)
log "configuring database/users/grants (idempotent, version-safe)"

$MYSQL_CMD <<SQL
-- Create DB
CREATE DATABASE IF NOT EXISTS \`${DB_ESC}\`
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- Remove anonymous users (MariaDB 10.4+ uses mysql.global_priv)
SET @has_gp := (
  SELECT COUNT(*) FROM information_schema.tables
  WHERE table_schema='mysql' AND table_name='global_priv'
);

SET @q := IF(@has_gp > 0,
  'DELETE FROM mysql.global_priv WHERE User=''''',
  'DELETE FROM mysql.user WHERE User='''''
);

PREPARE stmt FROM @q;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- App user for remote access + localhost (avoid host restriction surprises)
CREATE USER IF NOT EXISTS '${USER_ESC}'@'%' IDENTIFIED BY '${DBPW_ESC}';
ALTER USER '${USER_ESC}'@'%' IDENTIFIED BY '${DBPW_ESC}';
GRANT ALL PRIVILEGES ON \`${DB_ESC}\`.* TO '${USER_ESC}'@'%';

CREATE USER IF NOT EXISTS '${USER_ESC}'@'localhost' IDENTIFIED BY '${DBPW_ESC}';
ALTER USER '${USER_ESC}'@'localhost' IDENTIFIED BY '${DBPW_ESC}';
GRANT ALL PRIVILEGES ON \`${DB_ESC}\`.* TO '${USER_ESC}'@'localhost';

FLUSH PRIVILEGES;
SQL

# 6) enforce root password robustly (handle unix_socket/plugin weirdness)
log "enforcing root password"
set +e
$MYSQL_CMD -e "ALTER USER 'root'@'localhost' IDENTIFIED BY '${ROOT_ESC}'; FLUSH PRIVILEGES;" >/dev/null 2>&1
rc=$?
if [ "$rc" -ne 0 ]; then
  # fallback for cases where root uses unix_socket or plugin mismatch
  $MYSQL_CMD -e "ALTER USER 'root'@'localhost' IDENTIFIED VIA mysql_native_password USING PASSWORD('${ROOT_ESC}'); FLUSH PRIVILEGES;" >/dev/null 2>&1
  rc=$?
fi
set -e
[ "$rc" -eq 0 ] || die "failed to set root password (plugin/auth mismatch)"

# after setting password, ensure MYSQL_PWD is correct for shutdown
export MYSQL_PWD="$ROOT_PW"

# 7) stop temp server cleanly
log "stopping temp server"
$MYSQLADMIN_CMD shutdown >/dev/null 2>&1 || true
wait "$TEMP_PID" 2>/dev/null || true

# 8) exec real server (foreground) via CMD
log "starting real server (foreground)"
unset MYSQL_PWD || true
exec "$@"

