#!/bin/sh
set -eu

CRT="/etc/nginx/ssl/inception.crt"
KEY="/etc/nginx/ssl/inception.key"

if [ ! -f "$CRT" ] || [ ! -f "$KEY" ]; then
  echo "[nginx] generating self-signed cert..."
  openssl req -x509 -nodes -newkey rsa:2048 \
    -keyout "$KEY" -out "$CRT" -days 365 \
    -subj "/C=KR/ST=Seoul/L=Seoul/O=42/OU=Inception/CN=localhost"
fi

exec "$@"

