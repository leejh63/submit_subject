FROM debian:bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
    php-fpm php-mysql curl ca-certificates \
    mariadb-client \
    && rm -rf /var/lib/apt/lists/*

# php-fpm: listen을 TCP 9000으로 (nginx가 붙기 쉬움)
RUN sed -i 's|^listen = .*|listen = 9000|' /etc/php/*/fpm/pool.d/www.conf

# wp-cli 설치
RUN curl -fsSL https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar \
    -o /usr/local/bin/wp \
 && chmod +x /usr/local/bin/wp

WORKDIR /var/www/html

COPY tools/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

EXPOSE 9000

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["php-fpm8.2", "-F"]

