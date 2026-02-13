#!/bin/sh
set -eu

DB_DIR="/var/lib/mysql"
SOCK_DIR="/run/mysqld"
SOCK="${SOCK_DIR}/mysqld.sock"

read_secret() { [ -f "$1" ] && tr -d '\r\n' < "$1" || echo ""; }

ROOT_PW="$(read_secret /run/secrets/db_root_password)"
DB_PW="$(read_secret /run/secrets/db_password)"

: "${MYSQL_DATABASE:?MYSQL_DATABASE not set}"
: "${MYSQL_USER:?MYSQL_USER not set}"

[ -n "$ROOT_PW" ] || { echo "ERROR: missing db_root_password"; exit 1; }
[ -n "$DB_PW" ]   || { echo "ERROR: missing db_password"; exit 1; }

mkdir -p "$SOCK_DIR"
chown -R mysql:mysql "$SOCK_DIR" "$DB_DIR" 2>/dev/null || true

echo "[step] 1) datadir init (if needed)"
if [ ! -d "$DB_DIR/mysql" ]; then
  echo "  - init datadir..."
  chown -R mysql:mysql "$DB_DIR"
  mariadb-install-db --user=mysql --datadir="$DB_DIR" >/dev/null
else
  echo "  - datadir exists, skip"
fi

echo "[step] 2) start temp server (socket only, no TCP)"
mariadbd --user=mysql --datadir="$DB_DIR" --skip-networking --socket="$SOCK" &
TEMP_PID="$!"

echo "[step] 3) wait until ready"
i=0
while ! mariadb-admin --protocol=socket --socket="$SOCK" ping >/dev/null 2>&1; do
  i=$((i+1))
  [ "$i" -lt 150 ] || { echo "ERROR: temp mariadb not ready"; kill "$TEMP_PID" 2>/dev/null || true; exit 1; }
  sleep 0.2
done

echo "[step] 4) configure db/user/grants/root pw (idempotent)"
# root가 무비번인지/유비번인지 상황에 맞춰 접속 커맨드를 선택
if mariadb --protocol=socket --socket="$SOCK" -uroot -e "SELECT 1" >/dev/null 2>&1; then
  MYSQL_CMD="mariadb --protocol=socket --socket=$SOCK -uroot"
else
  MYSQL_CMD="mariadb --protocol=socket --socket=$SOCK -uroot -p$ROOT_PW"
fi

$MYSQL_CMD <<SQL
CREATE DATABASE IF NOT EXISTS \`${MYSQL_DATABASE}\`
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

DELETE FROM mysql.user WHERE User='';

CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${DB_PW}';
ALTER USER '${MYSQL_USER}'@'%' IDENTIFIED BY '${DB_PW}';
GRANT ALL PRIVILEGES ON \`${MYSQL_DATABASE}\`.* TO '${MYSQL_USER}'@'%';

ALTER USER 'root'@'localhost' IDENTIFIED BY '${ROOT_PW}';
FLUSH PRIVILEGES;
SQL

echo "[step] 5) stop temp server"
mariadb-admin --protocol=socket --socket="$SOCK" -uroot shutdown >/dev/null 2>&1 \
  || mariadb-admin --protocol=socket --socket="$SOCK" -uroot -p"$ROOT_PW" shutdown >/dev/null 2>&1 \
  || true

echo "[step] 6) start real server (TCP 3306) in foreground"
echo "  - listen: 0.0.0.0:3306"
exec mariadbd --user=mysql --datadir="$DB_DIR" --bind-address=0.0.0.0 --port=3306
