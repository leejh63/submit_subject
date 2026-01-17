#!/bin/sh
set -eu

: "${MYSQL_DATABASE:?Missing MYSQL_DATABASE}"
: "${MYSQL_USER:?Missing MYSQL_USER}"
: "${MYSQL_PASSWORD:?Missing MYSQL_PASSWORD}"

: "${WP_TITLE:?Missing WP_TITLE}"
: "${WP_ADMIN_USER:?Missing WP_ADMIN_USER}"
: "${WP_ADMIN_PASSWORD:?Missing WP_ADMIN_PASSWORD}"
: "${WP_ADMIN_EMAIL:?Missing WP_ADMIN_EMAIL}"
: "${WP_USER:?Missing WP_USER}"
: "${WP_USER_PASSWORD:?Missing WP_USER_PASSWORD}"
: "${WP_USER_EMAIL:?Missing WP_USER_EMAIL}"
: "${WP_URL:?Missing WP_URL}"

WP_PATH="/var/www/html"
MARKER="${WP_PATH}/.inception_wp_installed"

# wordpress 파일 없으면 다운로드
if [ ! -f "${WP_PATH}/wp-config.php" ]; then
  echo "[wp] downloading core..."
  wp core download --allow-root --path="${WP_PATH}"
fi

# DB 준비될 때까지 대기
echo "[wp] waiting for mariadb..."
i=0
while [ $i -lt 60 ]; do
  if mariadb -hmariadb -u"${MYSQL_USER}" -p"${MYSQL_PASSWORD}" -e "SELECT 1;" "${MYSQL_DATABASE}" >/dev/null 2>&1; then
    break
  fi
  i=$((i+1))
  sleep 1
done

if ! mariadb -hmariadb -u"${MYSQL_USER}" -p"${MYSQL_PASSWORD}" -e "SELECT 1;" "${MYSQL_DATABASE}" >/dev/null 2>&1; then
  echo "[wp] ERROR: mariadb not ready"
  exit 1
fi

# wp-config 생성
if [ ! -f "${WP_PATH}/wp-config.php" ]; then
  echo "[wp] creating wp-config..."
  wp config create --allow-root \
    --path="${WP_PATH}" \
    --dbname="${MYSQL_DATABASE}" \
    --dbuser="${MYSQL_USER}" \
    --dbpass="${MYSQL_PASSWORD}" \
    --dbhost="mariadb:3306"
fi

# 최초 1회만 설치
if [ ! -f "${MARKER}" ]; then
  echo "[wp] installing wordpress..."
  wp core install --allow-root \
    --path="${WP_PATH}" \
    --url="${WP_URL}" \
    --title="${WP_TITLE}" \
    --admin_user="${WP_ADMIN_USER}" \
    --admin_password="${WP_ADMIN_PASSWORD}" \
    --admin_email="${WP_ADMIN_EMAIL}"

  wp user create --allow-root \
    --path="${WP_PATH}" \
    "${WP_USER}" "${WP_USER_EMAIL}" \
    --user_pass="${WP_USER_PASSWORD}"

  touch "${MARKER}"
  echo "[wp] install done"
fi

exec "$@"

