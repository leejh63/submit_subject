# S32K_Can_slave / S32K_Lin_slave / S32K_LinCan_master 구조 분석 및 정리

## 1. 문서 목적

이 문서는 다음 3개 폴더를 기준으로 현재 구조를 분석하고, 정리/주의사항/보완점을 한 번에 볼 수 있도록 만든 리뷰 문서다.

- `S32K_Can_slave`
- `S32K_Lin_slave`
- `S32K_LinCan_master`

분석 기준은 `check_this_folder/embedded_implementation_framework_generic/embedded_generic_framework` 안의 문서를 적용했다.

- `README.md`
- `docs/01_architecture_principles.md`
- `docs/02_module_contracts.md`
- `docs/03_data_structure_selection.md`
- `docs/04_peripheral_patterns.md`
- `docs/05_special_cases_reference.md`

핵심 기준은 아래 5가지다.

1. 레이어 경계가 명확한가
2. SDK/레지스터 의존성이 바깥으로 새지 않는가
3. ISR은 짧고 deferred work 중심인가
4. 정적 메모리, 고정 크기 queue, 예측 가능한 자료구조를 쓰는가
5. 현재 규모에 비해 과한 분리 또는 부족한 분리가 없는가

---

## 2. 한 줄 결론

현재 구조는 "작은 역할별 펌웨어를 꽤 정석적으로 분리한 상태"에 가깝다.

좋은 점은 이미 `main -> runtime -> app -> module` 흐름이 잡혀 있고, CAN/LIN/ADC/UART를 정적 메모리와 고정 크기 queue 위에서 다루며, ISR도 비교적 짧게 유지하려는 방향이 보인다는 점이다.

다만 아래 4가지는 분명히 손볼 가치가 있다.

1. 공통 코드가 3개 프로젝트 폴더에 복제되어 있다.
2. `runtime_io.c`, `app_console.c`, `can_service.c`처럼 한 파일이 너무 많은 책임을 가진 구간이 있다.
3. `infra` 또는 public header 일부에 SDK 의존성이 섞여 있어, check 기준의 "HAL adapter 안으로 의존성 격리"가 완전히 지켜지지는 않는다.
4. 화면용 문자열, 상태 표시, 장치 제어가 일부 app 함수에서 함께 섞여 있어 이후 확장 시 유지보수 부담이 커질 수 있다.

---

## 3. 현재 구조를 체크 기준에 대입한 결과

| 기준 | 현재 상태 | 평가 |
|---|---|---|
| `main`은 얇고 최상위 진입점만 맡는가 | 세 프로젝트 모두 `main.c`가 매우 얇고 `Runtime_Init()` / `Runtime_Run()`만 호출 | 좋음 |
| App / Runtime / Module 구분이 있는가 | 있음. 역할별 펌웨어마다 `runtime`, `app`, `infra`, 통신/주변장치 폴더 분리 | 좋음 |
| ISR은 짧고 deferred work 중심인가 | `runtime_tick` hook 등록 후 `LinModule_OnBaseTick()` 정도만 ISR 쪽에 연결하고 실제 처리 대부분은 task에서 수행 | 좋음 |
| 동적 할당 회피 / 고정 버퍼 사용 | queue, 상태 객체, task table 모두 정적/고정 크기 중심 | 좋음 |
| 자료구조 선택이 단순하고 예측 가능한가 | ring/queue/fixed buffer 중심 | 좋음 |
| HAL adapter 안에 SDK 의존성이 잘 갇혔는가 | 일부는 잘 됨. 그러나 `runtime_tick.c`, `led_module.c`, `uart_hw.h` 등은 기준 대비 아쉬움 | 보완 필요 |
| 모듈 계약이 일관적인가 | `Init`, `Task`, `Get`, `Request` 형태는 비교적 일관적 | 대체로 좋음 |
| `GetState` / `GetStats` / 에러 정책이 충분한가 | 일부 모듈만 충분하고 전체 일관성은 부족 | 보완 필요 |
| 프로젝트 규모에 맞는 분리인가 | 대체로 적당하지만 `app_console.c`, `runtime_io.c`는 덩치가 큼. 반대로 공통 코드는 복제됨 | 부분 개선 필요 |

---

## 4. 프로젝트별 분석

### 4.1 `S32K_Can_slave`

#### 역할

- 현장 반응 노드
- 버튼 입력
- 로컬 LED 표시
- CAN 명령 수신 및 응답

#### 잘 된 점

- `main.c`가 아주 얇다.
- `runtime/runtime.c`가 task table만 만들고 실제 정책은 app 계층으로 넘긴다.
- `app/app_slave1.c`가 "현장 반응 노드 policy"를 담당해 역할이 분명하다.
- `can/can_hw.c -> can_proto.c -> can_service.c -> can_module.c` 계층이 비교적 선명하다.
- `runtime/runtime_io.c`가 이 노드에 필요한 보드 바인딩만 남아 있어 master/LIN slave보다 훨씬 읽기 쉽다.

#### 주의할 점

- `app/app_core.h`에 `mode_text`, `button_text`, `adc_text`, `can_input_text` 같은 화면/디버그성 문자열이 남아 있는데, 실제 이 프로젝트는 콘솔이 없다.
- `AppCore_SetResultText()`는 no-op이다.
- 즉, "실제 동작 상태"와 "관찰용 문자열 상태"가 분리되지 않고 일부 잔존해 있다.

#### 판단

- 구조 자체는 적절하다.
- 다만 현재 이미지가 정말 콘솔 없는 최소 운영 버전이라면, 문자열 상태 필드는 줄여도 된다.
- 이 프로젝트는 과분리보다 "디버그용 상태가 조금 남아 있는 정도"에 가깝다.

---

### 4.2 `S32K_Lin_slave`

#### 역할

- ADC 샘플링
- ADC zone 해석
- emergency latch 유지/해제
- LIN slave status 응답
- 로컬 LED 피드백

#### 잘 된 점

- 역할이 선명하다.
- `AdcModule`이 raw 값을 semantic zone으로 바꾸고 latch까지 유지해 주기 때문에 app이 threshold 비교를 직접 하지 않는다.
- `LinModule`이 portable 상태기계로 설계돼 있어, runtime IO가 하드웨어 callback만 바인딩하는 구조가 좋다.
- `AppCore_OnTickIsr()`는 `LinModule_OnBaseTick()`만 호출해 ISR을 짧게 유지한다.

#### 주의할 점

- `runtime/runtime_io.c`가 board init + LIN SDK glue + ADC SDK glue + threshold 정의까지 모두 가진다.
- `app/app_slave2.c`의 `AppSlave2_TaskAdc()`는
  - ADC task 호출
  - snapshot 읽기
  - 상태 문자열 생성
  - LIN status publish
  - LED 패턴 변경
  를 한 함수에서 모두 수행한다.

#### 판단

- 현재 규모에서는 이해 가능한 수준이다.
- 하지만 slave2 기능이 조금만 늘어나면 `runtime_io.c`와 `AppSlave2_TaskAdc()`가 가장 먼저 비대해질 가능성이 높다.

---

### 4.3 `S32K_LinCan_master`

#### 역할

- LIN status polling
- CAN field node 제어
- UART 콘솔 인터페이스
- emergency 승인 흐름 조정

#### 잘 된 점

- 역할상 가장 복잡한 노드인데도 `runtime -> app_core -> app_master` 흐름이 유지된다.
- `AppMaster`가 emergency 정책을 담당하고, `AppCore`가 task orchestration을 담당하는 방향은 맞다.
- 콘솔 입력을 바로 CAN으로 내리지 않고 `AppConsoleCanCommand` queue를 거쳐 app 계층이 소비하는 구조는 좋다.
- UART도 `uart_hw` / `uart_service`로 분리해 둔 점은 좋다.

#### 주의할 점

- `app/app_console.c`가 너무 크다. 대략 아래 기능이 한 파일 안에 모여 있다.
  - view dirty 관리
  - terminal layout 렌더링
  - input 파싱
  - command tokenize
  - local 명령 처리
  - CAN command queue 적재
  - UART recover 흐름
- `app/app_core.c`도 CAN 결과 포맷, 콘솔 명령 소비, LIN fresh status 처리 연결, render용 문자열 구성까지 꽤 많은 역할을 갖는다.
- `runtime/runtime_io.c`는 master LIN binding만 처리해서 slave보다 낫지만, 여전히 board init과 LIN glue가 한 파일에 있다.

#### 판단

- 세 프로젝트 중 구조 리스크가 가장 큰 곳은 master다.
- 기능 복잡도 자체는 정당하지만, 콘솔 관련 책임이 한 파일에 과도하게 뭉쳐 있다.

---

## 5. 공통 구조 관점에서 본 평가

### 5.1 잘 분리된 부분

다음은 현재 구조에서 유지해야 할 장점이다.

#### 1) 매우 얇은 `main.c`

세 프로젝트 모두 `main.c`가 동일하며 진입점 역할만 한다.

이건 유지하는 편이 좋다.

#### 2) `runtime`의 책임이 비교적 명확함

`runtime/runtime.c`는

- board/tick/app 초기화
- task table 구성
- super-loop 실행

정도만 맡는다.

이건 check 기준의 "App은 상태/정책, Runtime은 실행 orchestration"에 가깝다.

#### 3) CAN 계층 구조는 현재 규모에서 적절함

`can_hw -> can_proto -> can_service -> can_module`

이 구조는 처음 보면 살짝 세분화돼 보이지만, 실제로는 다음 이유로 유지 가치가 있다.

- master와 CAN slave가 공통 재사용한다.
- protocol/transport/request-tracking 책임이 다르다.
- 이후 diagnostic/event/text traffic이 조금 늘어도 견딜 수 있다.

즉, 여기서는 "과분리"보다는 "재사용 가능한 적정 분리"에 가깝다.

#### 4) LIN은 portable 상태기계 + runtime_io binding 구조가 좋음

`lin_module.c`가 protocol/state machine을 가지고, `runtime_io.c`가 SDK callback을 연결하는 구조는 check 기준에 비교적 잘 맞는다.

#### 5) 자료구조 선택이 무난함

- 고정 크기 queue
- ring buffer
- 정적 상태 객체
- 단순 task table

위주라서 worst-case 분석과 유지보수 관점에서 좋다.

---

### 5.2 과한 부분 또는 경미한 과분리

현재 규모 기준으로 "조금 과하다"라고 볼 수 있는 지점은 아래 정도다.

#### 1) 프로젝트별 복제 구조

세 프로젝트 전체 C/H 파일은 총 89개인데, 그중 51개가 중복 그룹에 들어간다.

대표 중복은 아래와 같다.

- `infra/*` 전부 3중 복제
- `main.c` 3중 복제
- `runtime/runtime.h` 3중 복제
- `can/*` 대부분 2중 복제
- `led/*` 2중 복제
- `lin/lin_module.h` 2중 복제

이건 "분리"라기보다 "복제 기반 구조"라서, 지금은 괜찮아도 수정 누락 위험이 커진다.

#### 2) `app_core_internal.h`

이 파일 자체가 잘못은 아니다.
다만 규모가 작은 역할별 이미지에서는 internal helper가 많아질수록 실제 흐름을 찾기 위해 헤더를 한 번 더 따라가야 한다.

결론적으로:

- 지금 당장 없애야 할 정도는 아님
- 다만 helper가 더 늘어나면 `static` 함수로 C 파일 안에 접는 편이 더 읽기 쉬울 수 있음

---

### 5.3 분리가 부족한 부분

여기가 실제 개선 포인트다.

#### 1) `runtime_io.c`가 너무 많은 책임을 가진다

특히 `S32K_Lin_slave/runtime/runtime_io.c`는 다음이 한 파일에 모여 있다.

- board init
- LIN transceiver enable
- LIN SDK callback bridge
- LIN config 구성
- ADC init/sample binding
- ADC threshold 정의
- LED 배선 정보

check 기준으로 보면 이는 `HAL adapter` 안에서도 다시 나눌 수 있는 수준이다.

권장 분리 예:

- `platform/board_init.c`
- `platform/tick_lptmr.c`
- `platform/lin_adapter.c`
- `platform/adc_adapter.c`
- `platform/led_binding.c`

#### 2) `app_console.c`는 기능 응집도가 너무 높다

현재 한 파일 안에 아래 성격이 모두 섞여 있다.

- UART 기반 view rendering
- dirty line redraw
- command parsing
- local command execution
- CAN command request 생성
- error 화면 복구

이건 가장 명확한 "과도한 집합"이다.

권장 분리 예:

- `app_console_view.c`
- `app_console_parser.c`
- `app_console_controller.c`

혹은 더 단순하게

- `app_console_render.c`
- `app_console_parse.c`

정도로만 나눠도 충분하다.

#### 3) `can_service.c`는 논리적으로는 맞지만 파일 덩치가 큼

현재 `can_service.c`는 다음을 모두 처리한다.

- transport queue
- hw 상태 추적
- request ID/pending table
- timeout 처리
- decode 후 dispatch
- result queue

기능상 맞는 레이어이긴 하지만, 파일 크기와 내부 책임 수를 보면 향후 분리 후보로 보는 게 좋다.

지금 당장 나눌 필요는 없지만, 기능이 더 늘어나면 다음 단위 분리가 자연스럽다.

- `can_transport.c`
- `can_request_tracker.c`
- `can_dispatch.c`

#### 4) 일부 app 함수가 정책 + 표현 + 출력 제어를 동시에 수행

대표적으로:

- `AppSlave2_TaskAdc()`
- `AppMaster_HandleFreshLinStatus()`

이 함수들은 현재 규모에서는 충분히 이해 가능하다.
하지만 이미 아래 3종류가 섞여 있다.

- 도메인 판단
- 사용자/디버그 표시 문자열 갱신
- 장치 출력 반영

기능이 조금만 늘면 유지보수성이 빠르게 떨어질 수 있다.

---

## 6. check 기준에서 특히 주의해야 할 위반/아쉬움

### 6.1 `infra` 안에 플랫폼 종속 코드가 들어 있음

`infra/runtime_tick.c`는 이름상 infra 계층인데 실제로는 다음 SDK 의존을 가진다.

- `interrupt_manager.h`
- `sdk_project_config.h`
- `LPTMR_DRV_*`

즉, 이 파일은 순수 infra라기보다 "platform timebase adapter"에 더 가깝다.

### 권장

- `infra`에는 시간 비교 유틸과 scheduler만 남긴다.
- LPTMR/IRQ 초기화는 `platform` 또는 `hal` 성격 폴더로 이동한다.

---

### 6.2 `LED`는 portable logic과 GPIO access가 섞여 있음

`led/led_module.h`는 비교적 깨끗하지만, `led/led_module.c`는 직접 `pins_driver.h`를 포함하고 실제 pin write를 수행한다.

즉, 현재 LED 모듈은

- pattern state machine
- board GPIO write

를 동시에 가진다.

현재 규모에서는 큰 문제는 아니지만, check 기준에 더 맞추려면 다음처럼 분리하는 편이 좋다.

- portable `led_service` 또는 `led_pattern`
- platform binding `led_hw` 또는 callback

---

### 6.3 `uart_hw.h`의 SDK 타입 노출

`S32K_LinCan_master/uart/uart_hw.h`는 public header인데 아래를 직접 include한다.

- `status.h`
- `lpuart_driver.h`

즉, 상위 계층이 이 헤더를 포함하는 순간 SDK 타입이 노출된다.

check 기준의

- "Driver public header에 SDK 타입이 새면 안 된다"
- "HAL adapter만 SDK 의존성을 안다"

관점에서는 아쉬운 부분이다.

### 권장

- public header에는 프로젝트 공통 타입만 남긴다.
- SDK 타입은 `uart_hw.c` 안으로 최대한 숨긴다.
- 반환형도 가능하면 `InfraStatus` 중심으로 바꾼다.

참고로 `can_hw.h`는 이 점에서 상대적으로 더 잘 감춰져 있다.

---

## 7. 현재 규모에 맞는 현실적인 권장 구조

### 7.1 너무 거창하게 바꾸지 말고, 공통화와 플랫폼 분리만 먼저

지금 규모에서 바로 check 문서의 전체 디렉터리 체계를 완전히 복제할 필요는 없다.

대신 아래 정도면 충분하다.

```text
common/
  infra/
    infra_types.*
    infra_queue.*
    runtime_task.*
  can/
    can_types.*
    can_proto.*
    can_service.*
    can_module.*
    can_hw.*
  lin/
    lin_module.*
  adc/
    adc_module.*
  led/
    led_module.*
  uart/
    uart_service.*
    uart_types.*
    uart_hw.*

platform_s32k/
  board/
    board_init.*
  tick/
    runtime_tick_lptmr.*
  can/
    can_hw_s32k.*
  lin/
    lin_binding_s32k.*
  adc/
    adc_binding_s32k.*
  uart/
    uart_hw_s32k.*
  led/
    led_binding_s32k.*

projects/
  S32K_Can_slave/
    app/
    runtime/
    main.c
  S32K_Lin_slave/
    app/
    runtime/
    main.c
  S32K_LinCan_master/
    app/
    runtime/
    main.c
```

핵심은 "공통 portable 코드"와 "S32K binding 코드"를 빼고, 역할별 프로젝트에는 app/runtime만 남기는 것이다.

---

### 7.2 현재 폴더 구조를 최대한 유지하면서도 개선 가능한 최소안

지금 폴더명을 크게 안 바꾸고 싶다면 아래만 해도 효과가 크다.

```text
shared/
  infra/
  can/
  lin/
  adc/
  led/
  uart/

s32k_platform/
  board/
  tick/
  bindings/

S32K_Can_slave/
  app/
  runtime/
  main.c

S32K_Lin_slave/
  app/
  runtime/
  main.c

S32K_LinCan_master/
  app/
  runtime/
  main.c
```

이 방식이 현재 코드베이스에는 더 현실적이다.

---

## 8. 우선순위별 수정 권장사항

### P1. 가장 먼저 할 것

#### 1) 공통 코드 복제 제거

우선 아래는 shared/common으로 빼는 것이 좋다.

- `infra/*`
- `can/*` 공통부
- `lin/lin_module.*`
- `led/*`
- 공통 `runtime/runtime.h`
- 공통 `main.c` 템플릿 성격 부분

#### 2) `app_console.c` 분리

master에서 가장 빨리 복잡도가 폭증할 파일이다.

최소 분리안:

- parse
- render

2개만 쪼개도 효과가 크다.

#### 3) `runtime_tick.c`를 `infra` 밖으로 이동

이건 구조 정합성 측면에서 가장 깔끔한 개선이다.

---

### P2. 다음 단계에서 할 것

#### 1) `runtime_io.c` 분리

특히 LIN slave 쪽부터 분리 효과가 크다.

#### 2) LED hardware access 분리

pattern logic과 GPIO write를 분리하면 재사용성과 테스트성이 좋아진다.

#### 3) `uart_hw.h` SDK 의존 축소

public header에서 SDK 흔적을 줄이면 레이어 경계가 좋아진다.

---

### P3. 문서화/보완

#### 1) CAN ID scheme / command table 문서화

`CAN_CMD_*`, response code, event code, broadcast 사용 규칙을 짧은 md로 따로 두는 것이 좋다.

#### 2) LIN schedule / PID / token 문서화

현재는 코드에 값이 박혀 있다.

- status PID
- ok PID
- ok token
- timeout ticks

이 값은 문서가 먼저 있어야 한다.

#### 3) ADC threshold 정책 문서화

- `safe_max`
- `warning_max`
- `emergency_min`
- latch clear 조건

이건 유지보수자 입장에서 매우 중요하다.

#### 4) queue overflow 정책 명시

고정 queue를 쓰는 건 매우 좋지만,
"가득 찼을 때 drop인지 reject인지"를 문서로 남겨야 한다.

---

## 9. 이 규모에서 굳이 안 해도 되는 것

아래는 지금 단계에서 과하다.

### 1) RTOS 도입

현재 구조는 cooperative super-loop로 충분하다.
task 수와 역할이 아직 명확해서 RTOS로 얻는 이득이 크지 않다.

### 2) 동적 메모리 기반 프레임워크

현재처럼 정적 객체와 고정 queue가 더 적절하다.

### 3) 모든 기능을 지나치게 세분화

예를 들어 작은 helper까지 전부 파일을 쪼개면 오히려 읽기성이 나빠질 수 있다.

특히 아래는 지금 상태를 유지해도 된다.

- 얇은 `main.c`
- 역할별 `app_master`, `app_slave1`, `app_slave2`
- CAN의 다단 분리 구조

---

## 10. 최종 정리

### 유지할 것

- 얇은 `main`
- `runtime` 중심 super-loop
- 역할별 app policy 분리
- 정적 메모리 / fixed queue
- CAN/LIN portable 모듈화 방향

### 줄일 것

- 프로젝트별 공통 코드 복제
- console 없는 이미지의 잔존 UI 문자열 상태

### 나눌 것

- `app_console.c`
- `runtime_io.c`
- `runtime_tick.c`의 플랫폼 종속 부분

### 문서화할 것

- CAN command / response / event 규칙
- LIN PID / token / schedule
- ADC threshold / latch clear 정책
- queue overflow / timeout 정책

---

## 11. 최종 판단

현재 구조는 "망가진 구조"가 아니라 "좋은 방향으로 이미 많이 가 있는 구조"다.

특히 역할별 이미지 분리, 정적 메모리 사용, ISR 최소화, app/runtime 분리는 충분히 칭찬할 만하다.

다만 다음 단계로 넘어가려면 반드시 정리해야 하는 축은 명확하다.

1. 공통 코드 shared/common화
2. 플랫폼 의존성의 위치 정리
3. master 콘솔과 runtime_io의 책임 분리

이 3가지만 먼저 정리해도, 이 코드베이스는 지금보다 훨씬 오래 버티는 구조가 된다.
