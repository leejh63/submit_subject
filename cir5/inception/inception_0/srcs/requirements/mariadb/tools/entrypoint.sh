#!/bin/sh
set -eu

# ---- required env ----
: "${MYSQL_DATABASE:?Missing MYSQL_DATABASE}"
: "${MYSQL_USER:?Missing MYSQL_USER}"
: "${MYSQL_PASSWORD:?Missing MYSQL_PASSWORD}"
: "${MYSQL_ROOT_PASSWORD:?Missing MYSQL_ROOT_PASSWORD}"

# ---- paths ----
DATADIR="/var/lib/mysql"
SOCKET="/run/mysqld/mysqld.sock"
PIDDIR="/run/mysqld"
INIT_MARKER="${DATADIR}/.inception_initialized"

# ---- runtime dirs + perms ----
mkdir -p "${PIDDIR}"
chown -R mysql:mysql "${PIDDIR}" "${DATADIR}"

# ---- if fully initialized before, just run server ----
if [ -f "${INIT_MARKER}" ]; then
  exec su -s /bin/sh -c "exec mariadbd --datadir=${DATADIR} --bind-address=0.0.0.0" mysql
fi

echo "[mariadb] init: start"

# ---- base DB init (creates system tables) ----
mariadb-install-db --user=mysql --datadir="${DATADIR}" > /dev/null
echo "[mariadb] init: install-db done"

# ---- start temp server (socket only) ----
su -s /bin/sh -c "mariadbd --user=mysql --datadir=${DATADIR} --skip-networking --socket=${SOCKET}" mysql &
pid="$!"
echo "[mariadb] init: temp mariadbd pid=${pid}"

# ---- wait for temp server ----
i=0
while [ $i -lt 30 ]; do
  if mariadb-admin --socket="${SOCKET}" ping > /dev/null 2>&1; then
    echo "[mariadb] init: temp server is up"
    break
  fi
  i=$((i+1))
  sleep 1
done

if ! mariadb-admin --socket="${SOCKET}" ping > /dev/null 2>&1; then
  echo "[mariadb] ERROR: temp server did not start"
  # kill temp server if it exists
  kill "${pid}" 2>/dev/null || true
  exit 1
fi

# ---- apply init SQL ----
echo "[mariadb] init: applying sql"
mariadb --protocol=socket --socket="${SOCKET}" -uroot <<-SQL
  -- root password
  ALTER USER 'root'@'localhost' IDENTIFIED BY '${MYSQL_ROOT_PASSWORD}';

  -- app DB + user
  CREATE DATABASE IF NOT EXISTS \`${MYSQL_DATABASE}\`;
  CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${MYSQL_PASSWORD}';
  GRANT ALL PRIVILEGES ON \`${MYSQL_DATABASE}\`.* TO '${MYSQL_USER}'@'%';
  FLUSH PRIVILEGES;
SQL
echo "[mariadb] init: sql applied"

# ---- shutdown temp server ----
mariadb-admin --protocol=socket --socket="${SOCKET}" -uroot -p"${MYSQL_ROOT_PASSWORD}" shutdown
wait "${pid}" 2>/dev/null || true
echo "[mariadb] init: temp server shutdown"

# ---- mark init done ONLY after success ----
touch "${INIT_MARKER}"
chown mysql:mysql "${INIT_MARKER}"
echo "[mariadb] init: done"

# ---- run real server (network enabled) ----
exec su -s /bin/sh -c "exec mariadbd --datadir=${DATADIR} --bind-address=0.0.0.0" mysql

