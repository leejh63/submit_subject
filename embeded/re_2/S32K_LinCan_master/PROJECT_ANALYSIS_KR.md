# S32K_LinCan_master 프로젝트 분석

## 한 줄 요약
- 이 프로젝트는 시스템의 판단 중심입니다.
- LIN으로 센서 상태를 읽고, CAN으로 다른 노드에 명령을 내리며, UART 콘솔로 전체 상태를 보여줍니다.

## 이 프로젝트를 처음 볼 때 핵심만 먼저
- 역할: master
- node id: `1`
- 직접 쓰는 핵심 모듈:
  - `runtime`
  - `infra`
  - `uart`
  - `app_console`
  - `can`
  - `lin`
  - `app_master`

## 시작 흐름
1. `main.c`에서 `Runtime_Init()`
2. `runtime.c`가 board init, tick init, app init 수행
3. `AppCore_Init()`가 현재 역할이 master임을 확인
4. `AppMaster_Init()` 호출
5. console + CAN + LIN을 초기화
6. 이후 super-loop에서 주기 task 수행

## 이 프로젝트에서 중요한 파일
- `src/main.c`
  - 시작점
- `src/runtime/runtime.c`
  - task table
- `src/app/app_core.c`
  - 공통 app orchestration
- `src/app/app_master.c`
  - master 정책 핵심
- `src/app/app_console.c`
  - UART 콘솔 명령/화면 처리
- `src/lin/lin_module.c`
  - slave2 상태 poll과 OK token 전송
- `src/can/can_module.c`
  - slave1으로 CAN 명령 전송
- `src/runtime/runtime_io.c`
  - UART/LIN/CAN SDK binding

## 실제 동작 시나리오

### 평상시
- master는 주기적으로 LIN slave2에 status PID를 보냅니다.
- slave2가 ADC 상태를 담은 status frame을 응답합니다.
- master는 그 값을 받아 현재 상태를 console에 표시합니다.

### emergency 발생 시
- slave2의 status에서 `zone == emergency` 또는 `emergency_latched == 1`이 들어옵니다.
- master는 이를 보고 시스템을 emergency 상태로 해석합니다.
- master는 CAN으로 slave1에 `CAN_CMD_EMERGENCY`를 보냅니다.
- slave1은 LED를 빨강으로 켜고, 사용자 버튼 입력을 기다립니다.

### 사용자 확인 이후
- slave1 사용자가 버튼을 누르면 slave1이 master에 `CAN_CMD_OK`를 보냅니다.
- master는 이것을 "사용자가 해제 요청을 했다"는 의미로 해석합니다.
- 단, ADC가 아직 emergency면 바로 승인하지 않습니다.
- ADC가 emergency에서 벗어났지만 latch가 남아 있으면 LIN으로 OK token을 전송합니다.
- slave2가 그 token을 받아 latch를 해제하면, master는 slave1에 `CAN_CMD_OK`를 보내 LED 승인 점멸을 시킵니다.

## task별로 보면

### `UART task`
- console 입력을 읽습니다.
- 사용자가 친 명령을 파싱해서 CAN 요청 queue에 넣습니다.

### `CAN task`
- console에서 올라온 명령을 실제 CAN 전송 요청으로 바꿉니다.
- CAN service를 돌립니다.
- 결과/result queue와 incoming message를 꺼내 화면 상태를 갱신합니다.

### `LIN fast task`
- LIN master 상태기계를 진행합니다.
- 새 LIN status가 오면 `AppMaster_HandleFreshLinStatus()`가 정책을 수행합니다.

### `LIN poll task`
- poll 주기가 되면 status poll 또는 OK PID를 보냅니다.
- console에서 local ok 요청이 있으면 `AppMaster_RequestOk()`를 호출합니다.

### `render task`
- 현재 상태를 UART 화면에 다시 그립니다.

## 이 프로젝트에서 중요한 데이터

### `AppCore.master_emergency_active`
- 현재 master가 시스템을 emergency로 보고 있는지 나타냅니다.

### `AppCore.master_slave1_ok_pending`
- slave1이 OK 요청을 했고, 아직 slave2 latch 해제가 끝나지 않았다는 의미입니다.

### `AppCore.lin_last_reported_zone`
- 직전 표시된 LIN zone입니다.
- 같은 상태 반복 출력 방지에 도움됩니다.

### `LinStatusFrame`
- slave2에서 온 ADC 상태 요약입니다.
- `adc_value`, `zone`, `emergency_latched`가 가장 중요합니다.

### `CanServiceResult`
- CAN 요청이 성공했는지, timeout 났는지 알려줍니다.

## 모듈 경계 관점에서 보면
- `app_master.c`
  - 정책을 담당합니다.
  - 어떤 조건에서 어떤 명령을 보낼지 결정합니다.
- `lin_module.c`
  - LIN 버스 상태기계와 frame 흐름을 담당합니다.
  - "언제 poll하고 언제 RX/TX를 시작할지"는 여기서 처리합니다.
- `can_module.c`
  - app에서 다루기 쉬운 request queue 경계입니다.
- `can_service.c`
  - request/response/timeouts를 실제로 관리합니다.
- `app_console.c`
  - UART를 사람 친화적인 명령/화면 인터페이스로 바꿉니다.

## 이 프로젝트를 디버깅할 때 보는 순서
1. `runtime.c`의 task 순서 확인
2. `app_master.c`의 상태 전이 확인
3. `lin_module.c`에서 status가 fresh로 들어오는지 확인
4. `can_module.c`와 `can_service.c`에서 slave1 명령이 큐에 쌓이고 나가는지 확인
5. `runtime_io.c`에서 LIN/CAN/UART binding 확인

## 평가

### 장점
- 판단 흐름이 `app_master.c`에 모여 있어서 읽기 쉽습니다.
- UART/LIN/CAN이 직접 얽히지 않고 app에서 조정됩니다.
- emergency 승인 절차가 단계적으로 드러나서 이해하기 좋습니다.

### 참고할 점
- master 프로젝트에도 실제로는 ADC/LED/slave용 파일이 같이 보입니다.
- 하지만 그것은 공통 베이스 복사 구조 때문이고, 실제 역할은 `APP_ACTIVE_ROLE` 기준으로 master만 사용합니다.
