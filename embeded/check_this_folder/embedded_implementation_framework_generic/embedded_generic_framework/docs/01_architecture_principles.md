# 01. Architecture Principles

## 1) 레이어 구조

권장 기준:

- **BSP / Platform**
  - 클럭, 핀, 인터럽트 테이블, 메모리 맵, 보드 초기화
- **HAL Adapter**
  - 벤더 SDK / 레지스터 / RTOS primitive를 일반 계약으로 감싸는 층
- **Driver**
  - 특정 주변장치의 동작을 일반화
  - GPIO/UART/SPI/I2C/CAN/LIN/ADC/PWM/DMA/WDT/FLASH/TIMER
- **Service**
  - 센서 체인, 프로토콜, 작업 스케줄, 진단, 메시지 흐름
- **App**
  - 시스템 모드, 시퀀스, 상태 전이, 정책

## 2) 레이어 경계 규칙

- App은 Driver 내부 버퍼 구조를 알면 안 된다.
- Service는 SDK 타입을 알면 안 된다.
- Driver는 vendor enum을 public header에 노출하면 안 된다.
- HAL Adapter만 SDK 의존성을 안다.
- ISR은 Driver 내부 또는 Adapter 내부에서 끝나야 하며, App까지 직접 올라가면 안 된다.

## 3) 모듈 공통 계약

모든 모듈은 아래 네 축을 가져라.

- `Init` : 정적 객체 초기화
- `Start/Open` : 실제 동작 시작
- `Stop/Close` : 동작 중단
- `Process/Task` : deferred work, timeout, retry, state handling

권장 반환값:

- `EMB_OK`
- `EMB_EINVAL`
- `EMB_ESTATE`
- `EMB_EBUSY`
- `EMB_ENOSPACE`
- `EMB_ETIMEOUT`
- `EMB_EIO`
- `EMB_EUNSUPPORTED`

## 4) 수명주기 기본 순서

1. Board/clock/pinmux init
2. timebase init
3. fault/logging init
4. low-level drivers init
5. peripheral start
6. services init
7. app init
8. main loop / scheduler start

## 5) ISR 규칙

ISR은 다음만 한다.

- 상태 플래그 set
- 짧은 카운터 증가
- 고정 버퍼에 데이터 저장
- 다음 처리 이벤트 등록
- 하드웨어 acknowledge

ISR이 하면 안 되는 것:

- printf
- 큰 memcpy
- 파싱
- 상태 전이 전체 수행
- 동적 할당
- block/wait
- 복잡한 retry 루프

## 6) 자료구조 선택의 대원칙

- worst-case 시간/공간 상한이 명확해야 한다.
- overflow/drop 정책이 명시돼야 한다.
- ISR-safe 여부가 분명해야 한다.
- 단일 생산자/단일 소비자 가정이 있으면 문서화한다.
- intrusive 구조는 비용이 낮지만 소유권 규칙을 명확히 적어야 한다.

## 7) 에러 정책

에러는 최소한 아래 세 계층으로 나눠라.

- **Local recoverable**
  - retry 가능, 한 모듈 내부에서 복구
- **Propagated operational**
  - 상위 서비스에 보고, degraded mode 전환 가능
- **Fatal / safe-state**
  - 강제 중지, watchdog reset, fail-safe 진입

## 8) 설계 리뷰 체크포인트

- 런타임 동적 할당이 있는가?
- Driver public header에 SDK 타입이 새는가?
- ISR이 너무 많은 일을 하는가?
- 큐 overflow 정책이 정의되었는가?
- timeout/retry/cancel 경로가 있는가?
- fault counter와 last error code가 있는가?
- reset reason, recovery action이 추적 가능한가?
