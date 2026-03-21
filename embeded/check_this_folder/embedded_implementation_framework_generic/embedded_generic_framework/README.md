# Embedded Implementation Framework Skeleton

이 저장소는 특정 보드, 특정 SDK, 특정 프로젝트에 종속되지 않는 **범용 임베디드 구현 골격**이다.

목표는 세 가지다.

1. 임베디드 초급자가 **정석적인 레이어 구조**를 익히게 한다.
2. 특정 통신(UART/CAN/LIN)이나 특정 MCU가 아니라 **전반적인 구현 습관**을 잡게 한다.
3. 예외적/특수한 분야(모터제어, OTA, USB, Ethernet, BLE, 안전인증, 파일시스템 등)는 템플릿이 아니라 **참고 문서**로 분리한다.

## 핵심 원칙

- 보드/SDK 의존성은 HAL adapter 계층 안으로 가둔다.
- Driver는 레지스터나 SDK 심볼을 외부로 누출하지 않는다.
- Service는 비즈니스 규칙과 데이터 흐름을 담당한다.
- App은 상태/모드/시퀀스만 조합한다.
- ISR은 짧게 끝내고, 무거운 일은 deferred work나 event queue로 넘긴다.
- 런타임 동적 할당보다 정적 메모리, 고정 크기 버퍼, 명시적 용량 제한을 우선한다.
- 자료구조는 “멋있어 보여서”가 아니라 **상한 분석 가능성**과 **복구 전략**을 기준으로 선택한다.

## 디렉터리

- `docs/` : 설계 철학, 자료구조 선택 기준, 특수 케이스 참고
- `include/core/` : 공통 타입, 상태값, 시간, fault, 모듈 수명주기
- `include/platform/` : IRQ, atomic, logging hook 등 플랫폼 추상
- `include/infra/` : 자료구조, 이벤트, 타이머, 상태머신
- `include/hal/` : 하드웨어 인터페이스 계약
- `include/drivers/` : 각 주변장치 드라이버 프레임
- `include/services/` : 센서/통신/작업 조정 서비스 프레임
- `include/app/` : 앱 수명주기와 최상위 상태 제어
- `src/` : 각 헤더의 기본 골격 구현
- `examples/` : 부팅/주기 루프/ISR-deferred 패턴 예시

## 적용 순서

1. `docs/01_architecture_principles.md`
2. `docs/02_module_contracts.md`
3. `docs/03_data_structure_selection.md`
4. `examples/example_baremetal_main.c`
5. 필요한 HAL adapter 작성
6. 필요한 Driver만 선택 도입
7. Service 조합
8. App 상태 흐름 설계

## 이 저장소가 일부러 하지 않는 것

- 특정 MCU 레지스터 맵 직접 제공
- 특정 벤더 SDK 호출 직접 박아 넣기
- 모터제어/USB/Ethernet/OTA/BLE 같은 특수 영역의 완성 드라이버 제공
- `malloc/free` 기반 프레임워크 제공

## 추천 사용법

이 골격을 “완성품”으로 보지 말고, **팀 규칙의 기준선**으로 써라.

- 네이밍 규칙
- 에러 처리 규칙
- 모듈 생명주기
- ISR 분리 규칙
- 자료구조 선택 기준
- HAL/Driver/Service/App 경계

이 다섯 축이 흔들리지 않으면, UART/CAN/LIN/ADC/PWM/DMA/SPI/I2C/GPIO/Flash 모두 같은 철학 위에서 붙일 수 있다.
