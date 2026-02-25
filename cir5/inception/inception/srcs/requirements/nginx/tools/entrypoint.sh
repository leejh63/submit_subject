#!/bin/sh
set -eu

CRT="/etc/ssl/certs/inception.crt"
KEY="/etc/ssl/private/inception.key"
CN="jaeholee.42.fr"

mkdir -p /etc/ssl/private

if [ ! -f "$CRT" ] || [ ! -f "$KEY" ]; then
  echo "[nginx] generating self-signed cert..."
  openssl req -x509 -nodes -newkey rsa:2048 \
    -keyout "$KEY" -out "$CRT" -days 365 \
    -subj "/CN=$CN"
fi
echo "[nginx] Done!"
exec "$@"
