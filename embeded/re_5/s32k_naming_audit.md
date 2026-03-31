# S32K 코드베이스 네이밍 감사 결과

대상:
- `S32K_Can_slave`
- `S32K_Lin_slave`
- `S32K_LinCan_master`

기준:
- 이름이 **실제 작동방식**과 맞는가
- 이름만 보고 **오해 가능성**이 없는가
- 함수/변수/상태/파일 이름이 **레이어 책임**과 맞는가
- 프로젝트 전체에서 **접두사, 동사, 시간 단위, 상태 표현**이 통일되어 있는가

---

## 1. 전체 평가

전체 평가는 **좋은 편**이다.

강점:
1. **접두사 규칙이 매우 잘 잡혀 있음**
   - `AppCore_`, `AppMaster_`, `AppSlave1_`, `AppSlave2_`
   - `CanModule_`, `CanService_`, `CanProto_`
   - `LinModule_`, `LinHw_`, `UartService_`
   - `BoardHw_`, `IsoSdk_`, `RuntimeIo_`
   - 이 규칙 덕분에 함수가 어느 레이어/모듈 소속인지 빠르게 파악된다.

2. **동사 선택도 전반적으로 안정적임**
   - `Init`, `Task`, `Set`, `Get`, `Handle`, `Queue`, `Request`, `Consume`, `Attach`, `Configure`
   - 대체로 라이프사이클과 의미가 잘 분리되어 있다.

3. **파일명과 공개 타입명도 대체로 일관적임**
   - `can_module`, `can_service`, `can_proto`, `lin_module`, `uart_service`, `board_hw` 등은 읽는 사람이 구조를 예측하기 쉽다.

하지만 아래 항목들은 반드시 손보는 편이 좋다.

---

## 2. 반드시 수정 권장

### 2.1 `AppMaster_blockOkRelayForEmergency`
- 위치: `S32K_LinCan_master/app/app_master.c:228`
- 문제:
  - 전체 코드가 `AppMaster_XxxYyy` 형태인데 여기만 `block`가 소문자로 시작한다.
  - 검색/정렬/자동완성 관점에서도 튄다.
- 현재 이름:
  - `AppMaster_blockOkRelayForEmergency`
- 권장 이름:
  - `AppMaster_BlockOkRelayForEmergency`
  - 또는 더 직접적으로 `AppMaster_CancelOkRelayForEmergency`
- 판단:
  - **가장 명백한 통일성 위반**이다.

---

### 2.2 `CanModuleRequest.code0 / code1 / code2`
- 위치: `S32K_LinCan_master/services/can_module.h:21-30`
- 문제:
  - 이 구조체는 public 성격이 강한 중간 표현인데, 필드명이 의미를 전혀 설명하지 못한다.
  - 실제로는 command일 때는 `command_code/arg0/arg1`, response일 때는 `request_id/result_code/detail_code`, event일 때는 `event_code/arg0/arg1`로 해석된다.
  - 즉, **하나의 필드명이 여러 의미를 동시에 가진다.**
- 현재 이름:
  - `code0`, `code1`, `code2`
- 권장 방향:
  1. 가장 좋음: `union`으로 분리
     - `command.command_code`, `command.arg0`, `command.arg1`
     - `response.request_id`, `response.result_code`, `response.detail_code`
     - `event.event_code`, `event.arg0`, `event.arg1`
  2. 최소 수정안:
     - `field0`, `field1`, `field2` 로 바꾸고 주석 강화
     - 다만 이건 임시방편이다.
- 판단:
  - **이 코드베이스에서 가장 의미가 불투명한 이름**이다.

---

### 2.3 `AppMaster_RequestOk`
- 위치: `S32K_LinCan_master/app/app_master.c:327`
- 실제 동작:
  - slave1에서 들어온 OK 요청을 받는다.
  - 최신 LIN 상태를 검증한다.
  - emergency/fault/stale 여부를 판단한다.
  - 조건이 맞으면 `LinModule_RequestOk()`를 호출한다.
  - 동시에 OK relay 상태를 시작한다.
- 문제:
  - 이름만 보면 “그냥 OK를 요청한다” 정도로 보이는데 실제로는 **검증 + 릴레이 시작 + LIN ok token 큐잉**까지 한다.
  - 즉, 함수명이 실제 책임보다 너무 작다.
- 권장 이름:
  - `AppMaster_ProcessSlave1OkRequest`
  - `AppMaster_BeginOkRelay`
  - `AppMaster_StartOkApprovalFlow`
- 판단:
  - 이 함수는 동작이 크기 때문에 지금 이름은 **너무 가볍다**.

---

### 2.4 `LinModule_RequestOk`
- 위치: `S32K_LinCan_master/services/lin_module.h:88`, `lin_module.c:531`
- 실제 동작:
  - 즉시 송신하지 않는다.
  - `ok_tx_pending = 1`로 두고, 다음 poll 슬롯에서 `pid_ok` header + token 송신 흐름으로 들어가게 만든다.
- 문제:
  - `RequestOk`는 “상위에서 승인 요청을 한다”처럼 들린다.
  - 실제론 더 좁은 의미인 **next poll에서 ok token TX 예약**이다.
- 권장 이름:
  - `LinModule_QueueOkTokenTx`
  - `LinModule_RequestOkTokenTx`
  - `LinModule_ScheduleOkTokenTx`
- 판단:
  - 이름이 너무 추상적이라, `AppMaster_RequestOk`와 함께 보면 둘의 역할 차이가 희미해진다.

---

### 2.5 `AppMaster_AfterLinPoll`
- 위치: `S32K_LinCan_master/app/app_master.c:546`
- 실제 동작:
  - relay 진행 중인지 확인
  - latest LIN status 평가
  - timeout / stale / retry limit 판정
  - 필요 시 `LinModule_RequestOk()` 재시도
- 문제:
  - `AfterLinPoll`은 “poll 뒤에 부르는 후처리”라는 호출 시점만 보여 준다.
  - 그런데 이 함수의 핵심 책임은 **OK relay retry/timeout 관리**다.
- 권장 이름:
  - `AppMaster_TaskOkRelayRetry`
  - `AppMaster_CheckOkRelayAfterPoll`
  - `AppMaster_ProcessOkRelayAfterLinPoll`
- 판단:
  - 지금 이름은 **언제 불리는가**만 말하고, **왜 존재하는가**를 말하지 못한다.

---

### 2.6 `LinModule_TaskFast`
- 위치: `S32K_LinCan_master/services/lin_module.h:86`, `lin_module.c:458`
- 실제 동작:
  - ISR/callback에서 적재된 pending event를 꺼내 처리
  - PID/RX/TX/error event를 deferred work로 풀어냄
- 문제:
  - `Fast`는 상대적 표현이라 기준이 없다.
  - 실제 의미는 “빠른 task”가 아니라 **pending event deferred processing**이다.
- 권장 이름:
  - `LinModule_ProcessPendingEvents`
  - `LinModule_TaskDeferredEvents`
- 판단:
  - 현재 동작을 정확히 아는 사람에게는 통하지만, 처음 보는 사람에게는 설명력이 약하다.

---

### 2.7 `LinStatusFrame.fresh`
- 위치: `S32K_LinCan_master/services/lin_module.h:60-68`, `lin_module.c:595-620`
- 문제:
  - 이 필드는 의미가 두 겹이다.
  - 한쪽에서는 wire/status freshness 비트다.
  - 다른 한쪽에서는 `ConsumeFreshStatus()`에서 소모되는 **local unread/new-data flag**처럼 쓰인다.
- 왜 위험한가:
  - `fresh`를 보고 “센서 데이터가 최신이다”라고 생각할 수 있는데,
  - 실제로는 “아직 소비 안 한 최신 수신본”이라는 의미도 섞여 있다.
- 권장 방향:
  - 분리하는 것이 가장 좋다.
    - `is_fresh_sample` 또는 `status_fresh`
    - `unconsumed_update` 또는 `has_unread_status`
- 판단:
  - **의미 중첩**이 있어서 장기적으로 가장 헷갈리기 쉬운 필드다.

---

### 2.8 `RUNTIME_IO_MASTER_LIN_*` 매크로를 slave 프로젝트에서도 사용
- 위치: `S32K_Lin_slave/runtime/runtime_io.c:12-15`, `128-151`
- 문제:
  - slave 프로젝트 안인데 상수명이 `MASTER`로 시작한다.
  - 값은 master schedule과 맞춰 쓰는 상수라는 건 이해되지만, 이름만 보면 slave 파일 안에서 왜 master가 나오나 혼란스럽다.
- 현재 이름:
  - `RUNTIME_IO_MASTER_LIN_ADC_PID`
  - `RUNTIME_IO_MASTER_LIN_OK_PID`
  - `RUNTIME_IO_MASTER_LIN_OK_TOKEN`
  - `RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS`
- 권장 이름:
  - `RUNTIME_IO_LIN_STATUS_PID`
  - `RUNTIME_IO_LIN_OK_PID`
  - `RUNTIME_IO_LIN_OK_TOKEN`
  - `RUNTIME_IO_LIN_TIMEOUT_TICKS`
- 판단:
  - **동작은 맞아도 읽는 사람을 헷갈리게 하는 이름**이다.

---

## 3. 강하게 수정 권장은 아니지만, 오해 소지가 있는 이름

### 3.1 `ok_tx_pending`
- 위치: `lin_module_internal.h`
- 문제:
  - 이름만 보면 “TX가 진행 중”처럼 들릴 수 있다.
  - 실제로는 “다음 poll 슬롯에서 ok token TX를 해야 함”이라는 예약 의미가 더 강하다.
- 권장 이름:
  - `ok_token_tx_pending`
  - `ok_token_tx_requested`

### 3.2 `ok_token_pending`
- 위치: `lin_module_internal.h`
- 문제:
  - slave 기준으로는 “ok token이 수신되었고 아직 app이 소비하지 않음”이다.
  - 단순 pending보다 `rx`가 들어가면 의미가 더 정확하다.
- 권장 이름:
  - `ok_token_rx_pending`
  - `pending_ok_token_rx`

### 3.3 `local_ok_pending`
- 위치: `app_console_internal.h:36`
- 문제:
  - 현재 의미는 “사용자가 콘솔에서 ok를 눌렀고 아직 app이 소비 안 함”이다.
  - `pending`도 맞지만, 의도는 사실상 request다.
- 권장 이름:
  - `local_ok_requested`
  - `local_ok_request_pending`

### 3.4 `last_activity`, `can_last_activity`
- 여러 파일에서 사용
- 문제:
  - `activity`라는 단어만 보면 count인지 timestamp인지 bool인지 바로 알기 어렵다.
  - 실제론 대부분 “이번 tick에 활동이 있었는가”에 가깝다.
- 권장 이름:
  - `activity_flag`
  - `did_activity_this_tick`
  - `had_can_activity`

### 3.5 `start_tick_ms`
- 위치: `can_service.h:20`
- 문제:
  - 프로젝트 전반은 `now_ms`, `started_ms`, `last_retry_ms`처럼 `ms` 중심 표현을 쓴다.
  - 여기만 `tick_ms`가 끼어 있다.
- 권장 이름:
  - `start_ms`
  - 또는 `started_ms`
- 판단:
  - 시간 단위 규칙 통일 차원에서 바꾸는 게 좋다.

### 3.6 `AppCore_SetCanInputText`, `AppCore_SetLinInputText`
- 문제:
  - `input`이라고 하면 사용자의 입력처럼 읽히기 쉽다.
  - 실제로는 “최근 수신/최근 상태 표시”에 가깝다.
- 권장 방향:
  - 그대로 써도 되지만 UI 명확성을 높이려면
  - `SetCanStatusText`, `SetLinStatusText`, `SetCanRxText`, `SetLinRxText` 중 하나로 정리 가능
- 판단:
  - 현재도 사용 가능하지만, 화면 의미가 커질수록 이름이 애매해진다.

---

## 4. 레이어 경계 관점에서 이름이 아쉬운 부분

### 4.1 `BoardHw_GetSlave1LedConfig`, `BoardHw_ReadSlave1ButtonPressed`
- 위치:
  - `S32K_Can_slave/drivers/board_hw.h:10-12`
  - `S32K_Can_slave/platform/s32k_sdk/isosdk_board.h:6-14`
- 문제:
  - `BoardHw`와 `IsoSdk_Board`는 하드웨어/보드 계층인데, 여기에 `Slave1`이라는 앱 역할 이름이 들어가 있다.
  - 즉, board 계층이 제품 역할을 알고 있다.
- 구조적으로 더 자연스러운 방식:
  - `BoardHw_GetStatusLedConfig`
  - `BoardHw_ReadUserButtonPressed`
  - 그리고 `AppPort_GetSlave1LedConfig` / `AppPort_ReadSlave1ButtonPressed`에서 slave1 의미를 부여
- 판단:
  - 현재 동작에는 문제 없지만, **레이어 책임을 이름 수준에서 약간 새게 만든다.**

### 4.2 `led_module`
- 문제:
  - 이름은 driver처럼 보이는데 실제 구현은 blink pattern, finite ack blink까지 포함한다.
  - 즉 raw GPIO driver보다 상위 의미를 가진다.
- 권장 대안:
  - `status_led`
  - `led_service`
  - `led_controller`
- 판단:
  - 지금도 틀렸다고 보긴 어렵지만, **이름보다 실제 책임이 약간 더 크다.**

---

## 5. 전체 통일성 평

### 5.1 잘 맞는 부분

#### 접두사 규칙
아주 좋다.
- App: `AppCore_`, `AppMaster_`, `AppConsole_`, `AppSlave1_`, `AppSlave2_`
- Service: `CanService_`, `CanModule_`, `LinModule_`, `UartService_`, `AdcModule_`
- Driver/HW: `CanHw_`, `LinHw_`, `BoardHw_`, `TickHw_`
- Adapter: `IsoSdk_`
- Runtime: `Runtime_`, `RuntimeIo_`, `RuntimeTick_`, `RuntimeTask_`

이건 유지해야 한다.

#### 동사 규칙
대체로 안정적이다.
- `Init`: 생성/초기화
- `Task`: 주기 실행 진입점
- `Handle`: 입력 이벤트/메시지 해석
- `Set`: 단순 값 반영
- `Get`: 소모 없는 조회
- `Consume` / `TryPop`: 소모성 조회
- `Queue` / `Request`: 나중 실행 예약

이 축은 좋다.

---

### 5.2 부분적으로 어긋나는 부분

#### `Get` vs `Consume` vs `TryPop`
전체적으로는 맞지만, CAN/LIN/UART 사이에 용어가 조금씩 다르다.

예:
- `CanModule_TryPopIncoming`
- `CanService_PopReceivedMessage`
- `LinModule_ConsumeFreshStatus`
- `LinModule_GetLatestStatusIfFresh`
- `UartService_ReadLine`

문제는 함수마다 틀렸다기보다, **비슷한 “소모성 조회” 동작을 각자 다른 단어로 표현**한다는 점이다.

권장 통일안:
- 소모 없는 조회: `Get*`
- 소모 있는 조회: `TryPop*` 또는 `Consume*` 중 하나로 통일
- 읽기 + 소모: `Read*`는 stream/UART에만 한정

---

## 6. 프로젝트별 평가

### 6.1 `S32K_LinCan_master`
가장 네이밍 품질이 높다.
다만 가장 중요한 제어 흐름이 여기에 있어서, 애매한 이름 하나가 전체 이해를 크게 방해한다.

핵심 수정 포인트:
- `AppMaster_RequestOk`
- `AppMaster_AfterLinPoll`
- `LinModule_RequestOk`
- `LinModule_TaskFast`
- `LinStatusFrame.fresh`
- `AppMaster_blockOkRelayForEmergency`

### 6.2 `S32K_Can_slave`
전반적으로 깔끔하다.
가장 큰 문제는 역할 이름이 아래 계층까지 내려간 부분이다.

핵심 수정 포인트:
- `BoardHw_GetSlave1LedConfig`
- `BoardHw_ReadSlave1ButtonPressed`
- `IsoSdk_BoardReadSlave1ButtonPressed`
- `led_module` 책임 대비 이름

### 6.3 `S32K_Lin_slave`
전반적으로 안정적이다.
다만 runtime_io의 macro 이름이 가장 먼저 눈에 띈다.

핵심 수정 포인트:
- `RUNTIME_IO_MASTER_LIN_*` -> 중립 이름
- `LinStatusFrame.fresh`
- `LinModule_RequestOk` 계열 의미 정리

---

## 7. 실제 리네이밍 우선순위

### 우선순위 1: 바로 바꿔도 되는 것
1. `AppMaster_blockOkRelayForEmergency` -> `AppMaster_BlockOkRelayForEmergency`
2. `RUNTIME_IO_MASTER_LIN_*` -> `RUNTIME_IO_LIN_*`
3. `start_tick_ms` -> `start_ms`
4. `ok_token_pending` -> `ok_token_rx_pending`
5. `ok_tx_pending` -> `ok_token_tx_pending`

### 우선순위 2: 의미 개선 효과가 큰 것
1. `AppMaster_RequestOk` -> `AppMaster_ProcessSlave1OkRequest` 또는 `AppMaster_BeginOkRelay`
2. `LinModule_RequestOk` -> `LinModule_QueueOkTokenTx`
3. `AppMaster_AfterLinPoll` -> `AppMaster_ProcessOkRelayAfterLinPoll`
4. `LinModule_TaskFast` -> `LinModule_ProcessPendingEvents`

### 우선순위 3: 구조까지 같이 손볼 때
1. `CanModuleRequest.code0/1/2` -> union 구조로 분리
2. `LinStatusFrame.fresh` 의미 분리
3. `BoardHw_*Slave1*` 계열을 board 의미명으로 중립화
4. `led_module` -> `status_led` 또는 `led_controller`

---

## 8. 최종 판단

이 코드베이스는 **네이밍 감각이 없는 코드가 아니다.**
오히려 전체적인 접두사, 레이어, 라이프사이클 동사 규칙은 꽤 잘 잡혀 있다.

문제는 아래 두 종류다.

1. **아주 일부 이름이 실제 책임보다 너무 작거나 추상적임**
   - `AppMaster_RequestOk`
   - `LinModule_RequestOk`
   - `AppMaster_AfterLinPoll`
   - `LinModule_TaskFast`

2. **중간 표현 구조체나 하위 계층 이름에 의미 누수/의미 부족이 있음**
   - `CanModuleRequest.code0/1/2`
   - `LinStatusFrame.fresh`
   - `BoardHw_*Slave1*`
   - `RUNTIME_IO_MASTER_LIN_*` in slave project

즉, 지금 상태는
- **대부분은 좋다**
- **몇몇 핵심 이름이 전체 이해를 방해한다**
- 그래서 “전면 재작명”보다는 **핵심 흐름 이름만 정밀하게 손보는 방식**이 맞다.

