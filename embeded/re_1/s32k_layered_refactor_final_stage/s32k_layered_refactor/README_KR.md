# S32K 레이어드 리팩터링 초안

이 패키지는 네가 올린 `main.c`의 LIN 슬레이브 동작을 기준으로, S32K SDK 의존성을 하위에 가두고 LIN/CAN/UART/LED를 레이어별로 나눈 시작점이다.

## 중요한 점

- `main.c`에서 실제로 확인된 SDK 심볼은 LIN/ADC/GPIO/LPTMR 쪽뿐이다.
- 그래서 **LIN 센서 슬레이브 경로는 꽤 구체적으로 옮겨 놓았고**,
- **CAN/UART 경로는 S32K 일반 패턴 기준 템플릿**으로 넣었다.
- CAN/UART 쪽은 네 실제 프로젝트의 generated 이름으로 `platform/s32k_bindings.h`만 맞춰 주면 된다.

## 디렉터리

- `core/` : 공통 결과값, tick 타입
- `platform/` : S32K 보드/생성 심볼 바인딩, timebase
- `hal/` : SDK 함수명 캡슐화
- `drivers/` : 장치 단위 제어
- `services/` : zone 분류, LIN payload 조립
- `app/` : 노드별 앱 조립
- `main/` : 노드별 엔트리
- `docs/` : 이전 코드 -> 새 구조 매핑, 적용 순서

## 현재 포함된 것

### 1. LIN 센서 슬레이브
- ADC 주기 샘플링
- SAFE/WARNING/DANGER/EMERGENCY zone 분류
- emergency latch + blink
- LIN PID 0x24 상태 응답
- LIN PID 0x25 OK 명령 수신

### 2. 마스터 노드 스켈레톤
- CAN/UART/LIN 드라이버/서비스/app 분리 위치
- gateway service 위치
- periodic task 조립 틀

### 3. CAN 버튼 슬레이브 스켈레톤
- 버튼 이벤트 -> CAN publish 구조 틀

## 바로 해야 할 작업

1. `platform/s32k_bindings.h`에서 CAN/UART generated 심볼 연결
2. 실제 master/button 프로젝트 헤더 include 정리
3. CAN ID / UART 명령 / LIN PID 계약 문서 확정
4. 이후 service와 app 로직 확장

## 원칙

- App는 SDK 함수 직접 호출 금지
- Service는 SDK 타입 노출 금지
- ISR은 짧게, Process에서 무거운 일 처리
- 정적 버퍼, 고정 용량 우선
