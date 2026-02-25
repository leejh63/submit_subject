FROM debian:12.13-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    mariadb-server \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

COPY tools/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

EXPOSE 3306
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

# 최종 PID 1로 실행될 프로세스
CMD ["mariadbd", "--user=mysql", "--datadir=/var/lib/mysql", "--bind-address=0.0.0.0", "--port=3306"]

