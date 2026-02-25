정확히 말하면 이렇게 정리하면 된다:

Dockerfile / 앱 설정 → “컨테이너 내부에서 실제로 리슨하는 포트”
docker-compose.yml → “호스트와 연결할 포트 매핑”

하지만 조금 더 정확하게 구조를 이해해야 헷갈리지 않는다.

1️⃣ 내부 포트는 누가 결정하나?

내부 포트는 애플리케이션이 결정한다.

예시:

MariaDB
mariadbd --port=3306
Nginx
listen 443;
PHP-FPM
listen = 9000

이게 “진짜 내부 포트”다.

Dockerfile의 EXPOSE 3306 이런 건:

문서적 메타데이터일 뿐
실제 포트를 바꾸지 않는다.

2️⃣ docker-compose의 ports는 무엇인가?
ports:
  - "443:443"

의미:

호스트 443 → 컨테이너 443

왼쪽 = 호스트
오른쪽 = 컨테이너 내부

3️⃣ 핵심 구조 그림
[ Host ]
  443
   ↓
[ nginx container ]
  443 (실제 nginx listen 포트)

----------------------------------

[ wordpress container ]
  9000 (php-fpm)

[ mariadb container ]
  3306 (mariadbd)

wordpress ↔ mariadb는 같은 네트워크 내부 통신이라
ports: 없어도 된다.

4️⃣ 그래서 네 질문에 대한 정확한 답

도커 컴포즈는 외부 포트
도커파일/스크립트는 내부 포트?

✔ 거의 맞다.
✔ 더 정확히는:

앱 설정 / 실행 커맨드가 내부 포트 결정

Dockerfile은 그 실행 환경을 만드는 역할

Compose는 외부 연결과 네트워크 구조를 정하는 역할

5️⃣ 예시로 완전 정리
🔵 내부 포트를 3307로 바꾸고 싶다
1️⃣ MariaDB 내부 설정 변경
CMD ["mariadbd", "--port=3307"]
2️⃣ WordPress DB_PORT 변경
environment:
  DB_PORT: 3307

Compose에 ports 없어도 내부 통신은 정상 동작.

🔵 외부에서 8443으로 접속하고 싶다
ports:
  - "8443:443"

→ nginx는 그대로 443 리슨
→ 호스트는 8443으로 접속

6️⃣ 흔히 생기는 오해

❌ EXPOSE 바꾸면 포트 바뀐다
→ 아님

❌ compose에서 ports 오른쪽 숫자 바꾸면 내부 포트 바뀐다
→ 아님

❌ 내부 포트 바꾸려면 compose만 수정하면 된다
→ 아님, 앱 설정도 수정해야 함

7️⃣ 한 줄 핵심

내부 포트는 "프로그램이 결정",
외부 포트는 "compose가 매핑".




# DEV_DOC — Inception (jaeholee)

## 0. 초기 설정 (최초 1회 실행)

프로젝트 실행 전에 secrets 파일과 환경 변수 파일을 먼저 생성해야 한다.
또한 도커를 완전히 초기화 한다.

docker compose -p inception down -v
docker stop $(docker ps -qa) 2>/dev/null || true
docker rm $(docker ps -qa) 2>/dev/null || true
docker volume rm $(docker volume ls -q) 2>/dev/null || true
docker system prune -af


### /etc/hosts 설정

VM의 IP 주소를 확인한다:

ip addr | grep inet

예시 출력:
10.0.2.15

hosts 파일을 수정한다:

sudo nano /etc/hosts

다음 내용을 추가한다 (VM IP로 변경):

10.0.2.15 jaeholee.42.fr

저장 후 종료.

정상 등록 확인:

ping jaeholee.42.fr


### secrets 디렉토리 및 비밀번호 파일 생성

mkdir -p secrets
touch secrets/db_root_password.txt
touch secrets/db_password.txt
touch secrets/wp_admin_password.txt
touch secrets/wp_user_password.txt

각 파일에 안전한 비밀번호를 입력한다.

예시:

echo "strong_root_password" > secrets/db_root_password.txt
echo "strong_db_password" > secrets/db_password.txt
echo "strong_admin_password" > secrets/wp_admin_password.txt
echo "strong_user_password" > secrets/wp_user_password.txt

파일 권한 제한:

sudo chmod 600 secrets/*.txt

mkdir -p secrets
touch secrets/db_root_password.txt
touch secrets/db_password.txt
touch secrets/wp_admin_password.txt
touch secrets/wp_user_password.txt
echo "strong_root_password" > secrets/db_root_password.txt
echo "strong_db_password" > secrets/db_password.txt
echo "strong_admin_password" > secrets/wp_admin_password.txt
echo "strong_user_password" > secrets/wp_user_password.txt


secrets 디렉토리는 Git에 포함되지 않는다.

---

### .env 파일 설정

다음 변수명을 반드시 포함해야 한다.
LOGIN
DOMAIN
MYSQL_DATABASE
MYSQL_USER
WP_TITLE
WP_ADMIN_USER
WP_ADMIN_EMAIL
WP_USER
WP_USER_EMAIL

예시:

LOGIN=jaeholee
DOMAIN=jaeholee.42.fr

MYSQL_DATABASE=wordpress
MYSQL_USER=wpuser

WP_TITLE=Inception

WP_ADMIN_USER=mario
WP_ADMIN_EMAIL=mario@jaeholee.42.fr

WP_USER=editor
WP_USER_EMAIL=editor@jaeholee.42.fr


권한 설정:

chmod 600 srcs/.env

주의:
비밀번호는 .env 파일에 작성하지 않는다.


### 영구 데이터 디렉토리

make시 아래 폴더들이 존재 하지 않을 경우 폴더를 생성한다.

프로젝트는 다음 경로를 사용한다:

/home/jaeholee/data
/home/jaeholee/data/mariadb
/home/jaeholee/data/wordpress

권한 및 소유자 확인:

ls -ld /home/jaeholee/data
ls -ld /home/jaeholee/data/mariadb
ls -ld /home/jaeholee/data/wordpress

stat /home/jaeholee/data
stat /home/jaeholee/data/mariadb
stat /home/jaeholee/data/wordpress

참고:
기본 컨테이너 UID/GID 값은 베이스 이미지에 따라 달라질 수 있다.
권한 문제가 발생할 경우, 컨테이너 내부 사용자 ID를 확인 후 조정한다.


---

## 3. 프로젝트 구조

루트 디렉토리:

- Makefile
- README.md
- USER_DOC.md
- DEV_DOC.md
- secrets/
- srcs/

srcs/ 내부:

- docker-compose.yml
- .env
- requirements/

requirements/ 내부:

- mariadb/
- wordpress/
- nginx/

각 서비스는 Debian 기반의 커스텀 Dockerfile로 빌드된다.


---

## 4. 빌드 및 실행

빌드 및 실행:

make

실행되는 명령:

docker compose -f srcs/docker-compose.yml up -d --build

중지:

make down

볼륨 포함 초기화:

make clean

데이터 폴더 까지 삭제:

make fclean

재빌드:

make re

docker compose ps 실행:

make ps

docker compose logs -f --tail=200 실행:

make logs


참고:
빌드 완료후 make logs 를 통해 모든 스크립트가 끝났는지 확인필요, 완전 작동까지 약간의 시간 딜레이 존재함

---

## 5. 컨테이너 관리

상태 확인:

docker compose -f srcs/docker-compose.yml ps

로그 확인:

docker compose -f srcs/docker-compose.yml logs --tail=200

컨테이너 내부 접속:

docker compose -f srcs/docker-compose.yml exec 컨테이너 이름 -it sh -lc


---

## 6. 서비스 아키텍처

### NGINX
- 443 포트만 외부 노출
- TLS 1.2 / 1.3 활성화
- Reverse Proxy 역할
- wordpress:9000 으로 fastcgi 전달

### WordPress
- php-fpm 기반 실행
- Docker 내부 네트워크로 MariaDB 연결
- 최초 실행 시 wp-cli로 자동 설치
- 관리자 계정 1개 및 일반 사용자 계정 1개 생성

### MariaDB
- 최초 실행 시 데이터베이스 초기화
- Docker secrets를 사용해 DB 및 사용자 생성
- 포그라운드(PID 1) 실행


---

## 7. 네트워크 구성

- 전용 Docker bridge 네트워크 사용
- host 네트워크 미사용
- 서비스 이름 기반 내부 통신:
  - mariadb
  - wordpress


---

## 8. 데이터 지속성

bind 옵션이 적용된 named volume 사용

호스트 경로:

/home/jaeholee/data/mariadb
/home/jaeholee/data/wordpress

컨테이너 삭제 또는 VM 재부팅 후에도 데이터 유지.


---

## 9. 보안 설계

- HTTPS(443)만 외부 노출
- network: host 및 links 미사용
- :latest 태그 미사용
- 서비스 간 격리
- 비밀번호 하드코딩 금지
- Docker secrets 사용


# INCEPTION – WordPress & MariaDB 로그인 / 검증 / 데이터 조회 정리

---

## 1. 컨테이너 상태 확인

docker compose -p inception ps

모든 서비스 상태가 `Up` 이어야 한다.

---

## 2. WordPress 웹 로그인

접속 주소:

https://<DOMAIN>/wp-admin

예:

https://jaeholee.42.fr/wp-admin

관리자 계정 비밀번호 확인:

docker exec -it inception-wordpress-1 cat /run/secrets/wp_admin_password

아이디는 `.env` 파일의 `WP_ADMIN_USER` 값이다.

---

## 3. WordPress 컨테이너 내부 접근

docker exec -it inception-wordpress-1 sh -lc ''

환경 변수 확인:

env | egrep "DOMAIN|MYSQL|WP_"

---

## 4. WP-CLI 검증 명령어

docker exec -it inception-wordpress-1 sh -lc '<명령>'

유저 목록 확인:

wp --allow-root user list

특정 유저 정보 확인:

wp --allow-root user get <username>

비밀번호 재설정 (테스트용):

wp --allow-root user update <username> --user_pass=test123

게시글 목록 확인:

wp --allow-root post list

댓글 목록 확인:

wp --allow-root comment list

기본 사이트 접근 변경

su -s /bin/sh www-data -c "
wp option update siteurl https://jaeholee.42.fr:8443 --path=/var/www/html
wp option update home    https://jaeholee.42.fr:8443 --path=/var/www/html
"


---

## 5. MariaDB 로그인 방법

컨테이너 진입:

docker exec -it inception-mariadb-1 sh -lc ''

root 로그인:

mariadb -uroot -p"$(cat /run/secrets/db_root_password)"

일반 사용자 로그인:

mariadb -u$MYSQL_USER -p"$(cat /run/secrets/db_password)"

---

## 6. 데이터베이스 기본 검증

데이터베이스 목록 확인:

SHOW DATABASES;

WordPress 데이터베이스 선택:

USE wordpress;

테이블 목록 확인:

SHOW TABLES;

정상 테이블 예시:

wp_users
wp_posts
wp_comments
wp_options

---

## 7. 내부 데이터 조회 SQL

docker exec -it inception-mariadb-1 sh -lc 'mariadb -uroot -p"$(cat /run/secrets/db_root_password)" -e "<명령>"'

유저 조회:

SELECT ID, user_login, user_email FROM wp_users;

게시글 조회:

SELECT ID, post_title, post_status FROM wp_posts;

댓글 조회:

SELECT comment_ID, comment_content FROM wp_comments;

사이트 URL 확인:

SELECT option_name, option_value
FROM wp_options
WHERE option_name IN ('siteurl','home');


---

## 9. TLS 동작 확인

docker exec -it inception-nginx-1 sh -lc ' \
openssl s_client -connect 127.0.0.1:443 -servername jaeholee.42.fr'

docker exec -it inception-nginx-1 sh -lc ' \
openssl s_client -connect 127.0.0.1:443 -servername jaeholee.42.fr -tls1_1'


---


## 10. 도커 컴포즈 재시작

docker compose -p inception up -d --build --force-recreate
 
---

## nginx - wordpress 연결 포트 변경의 경우 크게 어렵지 않다 그냥 nginx conf 파일, wordpress conf 파일만 9000에서 변경하면 적용된다
## wordpress - mariadb 는 조금 다르다
## mariadb 쪽은 Dockerfile 에 CMD 부분의 포트를 변경하면 끝난다 하지만
## wordpress 쪽은 조금 다르다. sh 파일내에 DB_PORT="3306" 이것과 sed -i "s/localhost/${DB_HOST}/" wp-config.php 이걸 수정해야한다
## DB_PORT=원하는 숫자 , sed -i "s/localhost/${DB_HOST}:${DB_PORT}/" wp-config.php
## 여기서 중요한점은 이미 워드프레스데이터가 존재할경우에는 data폴더 위치로 이동한 후 wp-config.php 파일내에 있는
## define( DB_HOST, maria); 이걸 define( DB_HOST, maria:4000); 이걸로 변경해야한다.


1. Docker Compose는 무엇을 관리하는가?
역할 분리
Dockerfile

이미지 빌드 정의

파일시스템 구성

기본 실행 커맨드 정의

docker compose

여러 컨테이너 오케스트레이션

네트워크 생성

볼륨 연결

포트 매핑

의존성 관리

secrets / env 관리

Compose에서 Dockerfile 사용 예시
services:
  wordpress:
    build:
      context: ./requirements/wordpress
      dockerfile: Dockerfile
    image: wordpress:inception

build → Dockerfile 빌드

image → 결과 이미지 이름

2. 컨테이너 간 네트워크 통신 원리
기본 동작

Compose는 프로젝트별 bridge 네트워크를 생성한다.

같은 compose 파일의 서비스들은 자동으로 같은 네트워크에 연결된다.

내부 통신 방식

서비스명 = 내부 DNS 이름

예: wordpress → mariadb:3306 접속 가능

외부 공개 없이도 내부 통신 가능.

ports vs expose

ports → 호스트와 연결

내부 통신만 필요하면 publish 불필요

DB는 보통 외부에 열지 않음

3. 커널 공유인데 PID / 포트는 왜 독립적인가?

Docker는 VM이 아니다.

커널은 하나지만 namespace로 격리한다.

4. PID 독립 원리
PID namespace

Docker는 컨테이너 생성 시:

새로운 PID namespace 생성

그 안에서 프로세스 실행

실제 현상

호스트:

PID 10234 -> nginx
PID 10250 -> mariadb

컨테이너 내부:

PID 1 -> nginx
PID 7 -> worker

같은 프로세스지만 보이는 PID가 다름.

핵심

실제 PID는 커널 전역에 존재

namespace로 보이는 PID만 다름

5. 포트 독립 원리
Network namespace

컨테이너마다:

IP 주소

포트 테이블

라우팅 테이블

네트워크 인터페이스

가 분리됨.

예시

컨테이너 A:

IP: 172.18.0.2
포트 80 사용

컨테이너 B:

IP: 172.18.0.3
포트 80 사용

충돌 없음.

왜냐하면 포트는 (IP, PORT) 조합으로 구분됨.

충돌이 나는 경우
ports:
  - "80:80"

이 경우:

호스트 80 → 컨테이너 80

호스트 80은 하나뿐이라 충돌 발생 가능.

6. VM과 Docker 차이
항목	VM	Docker
커널	여러 개	하나
격리 방식	하이퍼바이저	namespace
리소스 제한	VM 단위	cgroup
오버헤드	큼	작음
7. PID 1이 특별한 이유

PID 1은 단순히 번호 1이 아니다.

컨테이너 내부에서 init 역할을 한다.

8. PID 1의 특징
1. 고아 프로세스 수거 의무

부모가 죽으면 자식은 PID 1의 자식이 된다.

PID 1은 반드시 wait()로 수거해야 한다.

안 하면 zombie 누적 발생.

2. Signal 처리 특수성

일반 프로세스는 SIGTERM 기본 처리 존재

PID 1은 signal handler 없으면 무시하는 경우 발생

Docker stop이 안 먹는 이유가 여기서 발생 가능.

3. 컨테이너 생명주기 = PID 1 생명주기

PID 1이 종료되면 컨테이너 종료.

9. 잘못된 Docker 실행 예
❌ 백그라운드 실행
CMD ["sh", "-c", "nginx &"]

sh가 PID 1

nginx는 자식

sh 종료 → 컨테이너 종료

❌ daemon 모드 실행

daemon 모드는 PID 1이 즉시 종료될 위험 존재.

✅ 권장 방식
CMD ["nginx", "-g", "daemon off;"]

nginx가 PID 1

포그라운드 실행

signal 직접 처리

10. Self-Signed 인증서 경고 문제
경고가 뜨는 이유

브라우저는:

신뢰된 CA 목록을 가지고 있음

자가 서명은 그 목록에 없음

따라서 경고 발생.

경고 없애는 방법
1. 자체 CA 생성 후 클라이언트에 등록

CA 생성

서버 인증서 서명

클라이언트 trust store에 CA 등록

→ 해당 클라이언트에서 경고 제거

2. 공인 CA 사용 (Let’s Encrypt 등)

도메인 검증 필요

기본 신뢰

결론

자가서명인데 경고 없이 사용하는 것은

서버 설정 문제가 아니라

클라이언트가 신뢰하도록 설정하는 문제다.

최종 핵심 요약

Docker는 커널을 공유하지만 namespace로 격리한다.

PID 독립은 PID namespace 때문.

포트 독립은 Network namespace 때문.

PID 1은 컨테이너의 init 역할을 한다.

컨테이너 종료는 PID 1 종료와 동일하다.

자가서명 인증서 경고 제거는 클라이언트 신뢰 설정 문제다.


