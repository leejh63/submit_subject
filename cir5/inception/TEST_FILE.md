## ssh 연결

ssh -p 2222 jaeholee@127.0.0.1

## 호스트 > VM 내부로 폴더 옮기는 작업
호스트에서 진행

scp -p 2222 -r ./폴더명 jaeholee@127.0.0.1:/home/jaeholee

## 호스트 < VM

scp -P 2222 -r jaeholee@127.0.0.1:/home/jaeholee/inception ./폴더명


# ssh의 경우 소문자 p / scp의 경우 대문자 P

## 1. 평가 시작 전 Docker 전체 초기화 (필수)

평가지 요구사항에 따라, 기존 Docker 환경의 영향을 제거하기 위해
모든 컨테이너, 이미지, 볼륨, 네트워크를 초기화한다.
이 과정은 평가자가 프로젝트를 처음 실행하는 환경을 재현하기 위함이다.

docker stop $(docker ps -qa)
docker rm $(docker ps -qa)
docker rmi -f $(docker images -qa)
docker volume rm $(docker volume ls -q)
docker network rm $(docker network ls -q) 2>/dev/null

성공 결과:
- docker ps → 비어 있음
- docker images → 비어 있음
- docker volume ls → 기본 볼륨만 존재
- 에러 없이 종료

실패 결과:
- 일부 리소스 제거 실패
- permission denied 발생


## 2. 프로젝트 디렉터리 구조 확인

과제 요구사항:
- 모든 설정 파일은 srcs/ 디렉터리 하위에 존재해야 한다.
- 서비스(mariadb, wordpress, nginx)마다 Dockerfile이 반드시 존재해야 한다.

ls -la
ls -la srcs
ls -la srcs/requirements
find srcs/requirements -name Dockerfile

성공 결과:
- srcs 디렉터리 존재
- requirements 하위에 mariadb/wordpress/nginx 존재
- 각 디렉터리에 Dockerfile 존재

실패 결과:
- Dockerfile 누락
- 설정 파일이 srcs 외부에 존재


## 3. 금지 항목 사용 여부 검사 (평가지 필수)

과제 및 평가지에서 명시적으로 금지된 항목:
- network: host
- links 또는 --link
- 무한 루프 (tail -f, sleep infinity, while true)
- 백그라운드 실행 (nginx &)

아래 명령을 통해 금지된 설정이나 명령어가 사용되지 않았는지 검사한다.

grep -RIn --exclude-dir=.git \
  -e 'network:\s*host' \
  -e '^\s*links:' \
  -e '\-\-link' \
  -e 'tail -f' \
  -e 'sleep infinity' \
  -e 'while true' \
  -e 'nginx\s*&' \
  srcs Makefile

성공 결과:
- 아무것도 출력되지 않음

실패 결과:
- 위 금지 키워드 중 하나라도 출력됨


## 4. Docker Compose 실행 (Makefile 기준)

과제 요구사항에 따라 docker compose는 Makefile을 통해 실행해야 한다.
이미지는 직접 작성한 Dockerfile을 통해 빌드된다.

make

내부적으로 실행되는 명령:
docker compose -p inception -f srcs/docker-compose.yml up -d --build

성공 결과:
- mariadb, wordpress, nginx 이미지 빌드 완료
- 컨테이너 3개 모두 Up 상태

실패 결과:
- 빌드 실패
- 컨테이너 Exited 상태


## 5. 컨테이너 상태 확인

모든 서비스 컨테이너가 정상적으로 실행 중인지 확인한다.
nginx만 외부에 443 포트를 노출해야 한다.

docker compose ls
docker ps

성공 결과:
- 프로젝트 inception 실행 중
- nginx만 0.0.0.0:443 노출
- mariadb/wordpress 외부 포트 노출 없음

실패 결과:
- 443 외 다른 포트 외부 노출
- 컨테이너 Exited


## 6. Docker 네트워크 확인

bridge 네트워크가 사용되었는지 확인하고,
모든 컨테이너가 동일 네트워크에 연결되어 있는지 검사한다.

docker network ls
docker network inspect inception

성공 결과:
- bridge 네트워크 사용
- mariadb/wordpress/nginx 동일 네트워크 연결
- network_mode: host 없음

실패 결과:
- host 네트워크 사용
- 컨테이너 분리 네트워크


## 7. HTTP 접근 차단 확인 (필수)

과제 요구사항:
- HTTP(포트 80) 접근은 불가능해야 한다.

curl -I http://jaeholee.42.fr

성공 결과:
- Connection refused
- 404 또는 응답 없음
- 포트 80 리슨 없음

실패 결과:
- HTTP 200 OK 반환
- 포트 80 리슨 중


## 8. HTTPS 접근 확인

HTTPS(포트 443)를 통해 WordPress 사이트에 정상적으로 접근 가능한지 확인한다.
-self-signed 인증서이므로 -k 옵션을 사용한다.

curl -kI https://jaeholee.42.fr

성공 결과:
- HTTP/1.1 200 OK
- Server: nginx 표시

실패 결과:
- 502 Bad Gateway
- SSL handshake 실패
- Connection refused


## 9. TLS 버전 검증 (필수)

과제 요구사항:
- TLS v1.2, TLS v1.3 허용
- TLS v1.1 이하 차단

TLS 1.2 확인:
echo | openssl s_client -connect jaeholee.42.fr:443 -tls1_2 | grep -E 'Protocol|Cipher'

TLS 1.3 확인:
echo | openssl s_client -connect jaeholee.42.fr:443 -tls1_3 | grep -E 'Protocol|Cipher'

TLS 1.1 차단 확인:
echo | openssl s_client -connect jaeholee.42.fr:443 -tls1_1

성공 결과:
- TLSv1.2 연결 성공
- TLSv1.3 연결 성공
- TLSv1.1 handshake 실패

실패 결과:
- TLSv1.1 연결 성공
- TLSv1.2/1.3 실패


## 10. WordPress 사용자 계정 확인

관리자 계정 이름에 admin/Admin 이 포함되지 않아야 하며,
일반 사용자 계정이 존재해야 한다.

docker exec -it wordpress sh -lc 'wp --allow-root user list'

성공 결과:
- 관리자 계정 이름에 admin 포함되지 않음
- 일반 사용자(editor 등) 존재

실패 결과:
- admin / Admin 계정 존재
- 일반 사용자 없음


## 11. WordPress 페이지 수정 및 반영 확인

페이지 목록 조회:
docker exec -it wordpress sh -lc 'wp --allow-root post list --post_type=page --fields=ID,post_title,post_status'

페이지 제목 수정:
docker exec -it wordpress sh -lc '
PID=$(wp --allow-root post list --post_type=page --fields=ID --format=csv | tail -n +2 | head -n 1)
wp --allow-root post update "$PID" --post_title="Inception Page Updated"
'

웹 페이지 반영 확인:
curl -ks https://jaeholee.42.fr | grep "Inception Page Updated"

성공 결과:
- 수정한 제목이 HTML에 출력됨

실패 결과:
- 제목 반영되지 않음
- 502 오류


## 12. 댓글 작성 기능 테스트

WordPress 댓글 기능이 정상 동작하는지 확인한다.

docker exec -it srcs-wordpress-1 sh -lc '
POST_ID=$(wp --allow-root post list --post_type=post --fields=ID --format=csv | tail -n +2 | head -n 1)
wp --allow-root comment create \
  --comment_post_ID="$POST_ID" \
  --comment_author="editor" \
  --comment_author_email="editor@jaeholee.42.fr" \
  --comment_content="Incsssseption evassssluation cosssssmment"
'

댓글 목록 확인:
docker exec -it srcs-wordpress-1 sh -lc 'wp --allow-root comment list --number=5'

성공 결과:
- comment_ID 생성됨
- comment_content 출력됨

실패 결과:
- 댓글 생성 실패
- DB 오류


## 13. MariaDB 데이터 확인

MariaDB에 정상적으로 접속 가능한지,
WordPress 데이터베이스와 테이블이 존재하는지 확인한다.

docker exec -it srcs-mariadb-1 sh -lc '
mariadb -uroot -p"$(cat /run/secrets/db_root_password)" -e "
SHOW DATABASES;
USE wordpress;
SHOW TABLES;
SELECT User,Host FROM mysql.user;
"
'

성공 결과:
- wordpress DB 존재
- wp_posts, wp_comments 등 테이블 존재
- wpuser@% 존재

실패 결과:
- DB 없음
- 테이블 없음
- 권한 오류


## 14. 볼륨 및 데이터 영속성 확인

Docker 볼륨이 호스트 디렉터리에 바인드되어 있는지 확인한다.

docker volume ls
docker volume inspect srcs_mariadb_data
docker volume inspect srcs_wordpress_data

성공 결과:
- Mountpoint가 /home/jaeholee/data/... 경로로 설정됨

실패 결과:
- anonymous volume 사용
- bind mount 없음


## 15. 재부팅 후 데이터 유지(Persistence) 확인

시스템 재부팅 후에도 WordPress 페이지와 댓글이 유지되는지 확인한다.

sudo systemctl reboot --force --force


sudo reboot

재부팅 후:

docker exec -it wordpress sh -lc 'wp --allow-root post get <PAGE_ID> --field=post_title'
docker exec -it wordpress sh -lc 'wp --allow-root comment list --number=2'

성공 결과:
- 수정한 페이지 제목 유지
- 댓글 데이터 유지

실패 결과:
- 데이터 초기화됨


## 16. 종료 및 정리

컨테이너 중지:
make down

완전 정리:
make fclean

성공 결과:
- 컨테이너 중지 완료
- 데이터 디렉터리 비워짐

실패 결과:
- 컨테이너 남아 있음
- 데이터 남아 있음

