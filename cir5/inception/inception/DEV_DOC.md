# DEV_DOC — Inception (jaeholee)

## 0. 초기 설정 (최초 1회 실행)

프로젝트 실행 전에 secrets 파일과 환경 변수 파일을 먼저 생성해야 한다.

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

chmod 600 secrets/*.txt

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

이미지, 데이터 폴더 까지 모두 삭제:

make fclean

완전 재빌드:

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




