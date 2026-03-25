# S32K_Can_slave 프로젝트 분석

## 한 줄 요약
- 이 프로젝트는 현장 반응 노드입니다.
- master가 CAN으로 보내는 명령을 받아 LED를 바꾸고, 사용자의 버튼 입력을 다시 master에게 전달합니다.

## 이 프로젝트를 처음 볼 때 핵심만 먼저
- 역할: slave1
- node id: `2`
- 직접 쓰는 핵심 모듈:
  - `runtime`
  - `infra`
  - `can`
  - `led`
  - `app_slave1`
  - `runtime_io`의 button/LED binding

## 시작 흐름
1. `main.c`에서 `Runtime_Init()`
2. `AppCore_Init()`가 현재 역할이 slave1임을 확인
3. `AppSlave1_Init()` 호출
4. console/CAN 초기화
5. slave1 LED 초기화
6. 이후 periodic task로 CAN, button, LED를 돌림

## 이 프로젝트에서 중요한 파일
- `src/app/app_slave1.c`
  - 실제 정책 핵심
- `src/led/led_module.c`
  - LED 의미 단위 제어
- `src/can/can_module.c`
  - CAN app boundary
- `src/runtime/runtime_io.c`
  - button pin, LED pin binding
- `src/app/app_core.c`
  - 공통 orchestration

## 실제 동작 시나리오

### 평상시
- 특별한 CAN 명령이 없으면 LED는 꺼져 있거나 normal 상태입니다.
- button은 눌러도 의미가 없습니다.

### emergency 명령 수신
- master가 `CAN_CMD_EMERGENCY`를 보내면 slave1은 emergency mode로 들어갑니다.
- LED는 빨강 고정으로 바뀝니다.
- 화면/상태 문자열은 "press ok"처럼 바뀝니다.

### 사용자가 버튼 누름
- button task가 raw GPIO를 바로 믿지 않고 debounce합니다.
- 충분히 안정된 눌림으로 판단되면, 그리고 현재 emergency mode이면 master에게 `CAN_CMD_OK`를 보냅니다.
- 이 메시지는 "사용자가 확인을 눌렀다"는 의미입니다.

### master 승인 후
- master가 slave2 latch 해제를 마친 뒤 slave1에 `CAN_CMD_OK`를 보냅니다.
- slave1은 초록 점멸 ack blink를 시작합니다.
- 점멸이 끝나면 다시 normal 상태로 돌아갑니다.

## task별로 보면

### `CAN task`
- master가 보낸 명령을 받습니다.
- `AppSlave1_HandleCanCommand()`가 command code별 행동을 수행합니다.

### `button task`
- `RuntimeIo_ReadSlave1ButtonPressed()`로 raw 입력을 읽습니다.
- 연속 샘플이 같은지 확인해서 debounce합니다.
- emergency mode에서만 OK 요청을 올립니다.

### `LED task`
- blink 패턴을 진행합니다.
- finite blink가 끝나면 normal 상태로 복귀시킵니다.

## 이 프로젝트에서 중요한 데이터

### `AppCore.slave1_mode`
- slave1의 현재 mode입니다.
- normal, emergency, ack blink 같은 의미를 가집니다.

### `AppCore.slave1_last_sample_pressed`
- 직전 raw button 샘플입니다.

### `AppCore.slave1_same_sample_count`
- 같은 샘플이 몇 번 연속 나왔는지 나타냅니다.
- debounce에 쓰입니다.

### `AppCore.slave1_stable_pressed`
- 현재 안정 상태로 확정된 버튼 상태입니다.

### `LedPattern`
- slave1은 LED를 핀으로 제어하지 않고 pattern으로 제어합니다.
- emergency면 `RED_SOLID`
- 승인 표시면 `GREEN_BLINK`

## 왜 이 구조가 좋은가
- button debounce 로직이 app에 있고, pin read는 `runtime_io.c`에 있습니다.
- 즉 "하드웨어 읽기"와 "버튼 의미 판단"이 분리되어 있습니다.
- LED도 마찬가지로 app은 semantic pattern만 사용하고, 실제 pin polarity는 숨겨져 있습니다.

## 모듈 경계 관점에서 보면
- `app_slave1.c`
  - emergency/승인 흐름 정책
- `led_module.c`
  - LED 표현
- `runtime_io.c`
  - button/LED 하드웨어 연결
- `can_module.c`
  - master와의 통신 경계

## 이 프로젝트를 디버깅할 때 보는 순서
1. `app_slave1.c`에서 현재 mode 전환 확인
2. `runtime_io.c`에서 button pin polarity 확인
3. `led_module.c`에서 LED active level 확인
4. `can_service.c`에서 master 명령 수신/OK 송신 확인

## 평가

### 장점
- 역할이 단순하고 읽기 쉽습니다.
- 버튼 입력, LED 출력, CAN 통신이 서로 섞이지 않고 app에서 연결됩니다.
- 현장 장치로서 필요한 동작이 명확합니다.

### 참고할 점
- 프로젝트 폴더 안에는 LIN/ADC/UART 관련 파일도 공통 베이스로 존재합니다.
- 하지만 실제 동작 관점에서는 `app_slave1.c`와 해당 CAN/LED path만 집중해서 보면 됩니다.
