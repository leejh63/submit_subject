FROM debian:bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
    mariadb-server \
    && rm -rf /var/lib/apt/lists/*

COPY tools/entrypoint.sh /usr/local/bin/entrypoint.sh
COPY tools/50-server.cnf /etc/mysql/mariadb.conf.d/50-server.cnf
RUN chmod +x /usr/local/bin/entrypoint.sh

EXPOSE 3306

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["mariadbd"]
