# USER_DOC — Inception (jaeholee)

## 1. 제공 서비스

본 프로젝트는 Docker 기반의 웹 인프라를 구성한다.

다음 세 가지 서비스로 구성되어 있다:

- NGINX 
  HTTPS(포트 443)만 외부에 노출하는 Reverse Proxy 역할을 수행한다.

- WordPress 
  php-fpm 기반으로 동작하는 웹 애플리케이션 서버이다.

- MariaDB 
  WordPress에서 사용하는 데이터베이스 서버이다.

각 서비스는 독립된 Docker 컨테이너에서 실행된다.


---

## 2. 프로젝트 실행 및 종료 방법

프로젝트 루트 디렉토리에서 실행한다.

프로젝트 시작:

make

이 명령은 다음을 수행한다:
- Docker 이미지 빌드
- 네트워크 및 볼륨 생성
- 모든 컨테이너를 백그라운드로 실행

프로젝트 중지:

make down

완전 초기화 후 재실행:

make re


---

## 3. 웹사이트 및 관리자 페이지 접근

메인 웹사이트:

https://jaeholee.42.fr

WordPress 관리자 페이지:

https://jaeholee.42.fr/wp-admin


---

## 4. 인증 정보 관리

민감한 정보는 Docker secrets를 통해 관리된다.

./secrets/db_root_password.txt 
./secrets/db_password.txt 
./secrets/wp_admin_password.txt 
./secrets/wp_user_password.txt 

위 파일들은 Git에 포함되지 않는다.

환경 설정 변수는 다음 파일에 저장된다:

srcs/.env

.env 파일에는 설정 값만 존재하며, 비밀번호는 포함되지 않는다.


---

## 5. 데이터 저장 및 지속성

데이터는 호스트 머신에 영구 저장된다.

데이터베이스 파일 위치:
  /home/jaeholee/data/mariadb

WordPress 파일 위치:
  /home/jaeholee/data/wordpress

컨테이너를 삭제하거나 재시작해도 해당 디렉토리가 존재하는 한 데이터는 유지된다.


---

## 6. 서비스 정상 동작 확인 방법

컨테이너 상태 확인:

docker compose -f srcs/docker-compose.yml ps

서비스 로그 확인:

docker compose -f srcs/docker-compose.yml logs --tail=200

HTTPS 응답 테스트:

curl -kI https://jaeholee.42.fr



