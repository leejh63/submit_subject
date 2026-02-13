FROM debian:12.13-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    php-fpm php-mysql php-cli \
    curl ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# wordpress cli
RUN curl -fsSL https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar \
    -o /usr/local/bin/wp \
 && chmod +x /usr/local/bin/wp

# 워드프레스 코어를 이미지에 포함 (/usr/src/wordpress)
RUN mkdir -p /usr/src/wordpress \
 && wp core download --allow-root --path=/usr/src/wordpress --locale=en_US

COPY conf/www.conf /etc/php/8.2/fpm/pool.d/www.conf
COPY tools/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

WORKDIR /var/www/html
EXPOSE 9000
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["php-fpm8.2", "-F"]
