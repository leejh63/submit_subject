#!/bin/sh
set -eu

# ----------------------------
# helpers
# ----------------------------
read_secret() { [ -f "$1" ] && tr -d '\r\n' < "$1" || echo ""; }

log() { printf "[wp-init] %s\n" "$*"; }
die() { printf "[wp-init] ERROR: %s\n" "$*" >&2; exit 1; }

# ----------------------------
# required env
# ----------------------------
: "${MYSQL_DATABASE:?MYSQL_DATABASE not set}"
: "${MYSQL_USER:?MYSQL_USER not set}"
: "${DOMAIN:?DOMAIN not set}"
: "${WP_TITLE:?WP_TITLE not set}"
: "${WP_ADMIN_USER:?WP_ADMIN_USER not set}"
: "${WP_ADMIN_EMAIL:?WP_ADMIN_EMAIL not set}"
: "${WP_USER:?WP_USER not set}"
: "${WP_USER_EMAIL:?WP_USER_EMAIL not set}"

# ----------------------------
# secrets (root reads)
# ----------------------------
DB_PW="$(read_secret /run/secrets/db_password)"
WP_ADMIN_PW="$(read_secret /run/secrets/wp_admin_password)"
WP_USER_PW="$(read_secret /run/secrets/wp_user_password)"

[ -n "$DB_PW" ] || die "missing secret: db_password"
[ -n "$WP_ADMIN_PW" ] || die "missing secret: wp_admin_password"
[ -n "$WP_USER_PW" ] || die "missing secret: wp_user_password"

WEBROOT="/var/www/html"
SRC="/usr/src/wordpress"
DB_HOST="mariadb"
DB_PORT="3306"
URL="https://${DOMAIN}"

# ----------------------------
# 0) seed wp core into empty volume
# ----------------------------
if [ ! -f "${WEBROOT}/wp-config-sample.php" ]; then
  log "webroot is empty -> seeding wp core from ${SRC}"
  [ -f "${SRC}/wp-config-sample.php" ] || die "wp core missing in ${SRC}"
  cp -a "${SRC}/." "${WEBROOT}/"
fi

# ----------------------------
# 1) ownership (avoid re-chown if already correct? keep simple)
# ----------------------------
# NOTE: safe default for a first boot; you can optimize later.
log "ensure ownership: www-data:www-data"
chown -R www-data:www-data "${WEBROOT}" 2>/dev/null || true

# ----------------------------
# 2) wait for DB
# ----------------------------
log "waiting for mariadb (${DB_HOST}:${DB_PORT}) via PHP mysqli..."
i=0
while :; do
  php -r "
    \$m = @new mysqli('${DB_HOST}', '${MYSQL_USER}', '${DB_PW}', '${MYSQL_DATABASE}', ${DB_PORT});
    if (\$m->connect_errno) { exit(1); }
    \$m->close(); exit(0);
  " >/dev/null 2>&1 && break

  i=$((i+1))
  [ "$i" -lt 60 ] || die "mariadb not reachable/auth failed after 60s"
  sleep 1
done
log "mariadb ok"

# ----------------------------
# 3) wp-config.php create (if missing)
# ----------------------------
if [ ! -f "${WEBROOT}/wp-config.php" ]; then
  log "create wp-config.php"
  cd "${WEBROOT}"
  cp wp-config-sample.php wp-config.php
  sed -i "s/database_name_here/${MYSQL_DATABASE}/" wp-config.php
  sed -i "s/username_here/${MYSQL_USER}/" wp-config.php
  sed -i "s/password_here/${DB_PW}/" wp-config.php
  sed -i "s/localhost/${DB_HOST}/" wp-config.php
  chown www-data:www-data wp-config.php 2>/dev/null || true
  chmod 640 wp-config.php 2>/dev/null || true
fi

# ----------------------------
# 4) install WP if not installed
#    IMPORTANT: run wp-cli as www-data, but secrets are root-only -> pass via env
# ----------------------------
if ! su -s /bin/sh www-data -c "wp core is-installed --path='${WEBROOT}' --url='${URL}'" >/dev/null 2>&1; then
  log "wp not installed -> installing"
  su -s /bin/sh www-data -c "
    env APW='${WP_ADMIN_PW}' \
    wp core install \
      --quiet \
      --path='${WEBROOT}' \
      --url='${URL}' \
      --title='${WP_TITLE}' \
      --admin_user='${WP_ADMIN_USER}' \
      --admin_password=\"\$APW\" \
      --admin_email='${WP_ADMIN_EMAIL}' \
      --skip-email
  "
else
  log "wp already installed"
fi

# ----------------------------
# 4.5) comment auto-approve (assignment / dev mode)
# ----------------------------
log "setting comment auto-approve options"

su -s /bin/sh www-data -c "
  wp option update comment_moderation 0 --path='${WEBROOT}' --url='${URL}'
  wp option update comment_whitelist 0 --path='${WEBROOT}' --url='${URL}'
  wp option update comment_previously_approved 0 --path='${WEBROOT}' --url='${URL}'
" >/dev/null 2>&1 || true


# ----------------------------
# 5) create normal user if missing
# ----------------------------
if ! su -s /bin/sh www-data -c "wp user get '${WP_USER}' --field=ID --path='${WEBROOT}' --url='${URL}'" >/dev/null 2>&1; then
  log "creating user: ${WP_USER} (${WP_USER_EMAIL})"
  su -s /bin/sh www-data -c "
    env UPW='${WP_USER_PW}' \
    wp user create \
      '${WP_USER}' '${WP_USER_EMAIL}' \
      --user_pass=\"\$UPW\" \
      --role=subscriber \
      --path='${WEBROOT}' \
      --url='${URL}'
  " || true
else
  log "user already exists: ${WP_USER}"
fi

# ----------------------------
# 6) start php-fpm foreground (PID1)
# ----------------------------
log "starting php-fpm (foreground)"
exec php-fpm8.2 -F
