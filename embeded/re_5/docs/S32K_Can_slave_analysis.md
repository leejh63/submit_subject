# S32K CAN Slave 분석

버튼 입력과 LED 피드백을 가진 CAN 현장 반응 노드다. master의 CAN 명령을 받아 emergency/ack 상태를 표시하고, 로컬 버튼 입력을 debounce해서 다시 CAN OK 요청으로 올린다.

## 전체 흐름

1. `main()`이 `Runtime_Init()`을 호출해 보드, tick, AppCore를 준비한다.
2. `Runtime_Init()`은 `RuntimeIo_BoardInit()`으로 보드를 올리고 `RuntimeTick_Init()`으로 기준 시간을 만든 뒤 `AppCore_Init()`으로 역할별 상태를 초기화한다.
3. `AppCore_Init()`은 `RuntimeIo_GetLocalNodeId()`로 slave1 node id를 채우고 기본 문자열을 세팅한 다음 `AppSlave1_Init()`으로 CAN/LED 모듈을 시작한다.
4. `Runtime_BuildTaskTable()`은 `button`, `can`, `led`, `heartbeat` task를 period 기반으로 등록한다.
5. `Runtime_Run()` super-loop는 `RuntimeTask_RunDue()`를 반복 호출해 due task만 cooperative하게 실행한다.
6. `AppCore_TaskCan()`은 `CanModule_Task()`를 통해 CAN 송수신을 진행하고, 들어온 command는 `AppCore_HandleCanIncoming()` -> `AppSlave1_HandleCanCommand()`로 전달된다.
7. `AppSlave1_TaskButton()`은 raw button 입력을 debounce하고 emergency 상태에서 안정된 press가 검출되면 master로 `CAN_CMD_OK`를 보낸다.
8. `AppSlave1_TaskLed()`는 finite blink를 끝까지 진행하고, acknowledgement 시퀀스가 끝나면 slave1 상태를 normal로 되돌린다.

## 주요 설정 상수

- `APP_NODE_ID_MASTER`: `1U`
- `APP_NODE_ID_SLAVE1`: `2U`
- `APP_TASK_BUTTON_MS`: `10U`
- `APP_TASK_CAN_MS`: `10U`
- `APP_TASK_LED_MS`: `100U`
- `APP_TASK_HEARTBEAT_MS`: `1000U`
- `APP_SLAVE1_ACK_TOGGLES`: `6U`

## 프로젝트 핵심 자료구조

### `app/app_core.h`

CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 context 헤더다. slave1이 실제로 쓰는 CAN 상태, 버튼 debounce 상태, 로컬 LED 제어기만 남겨 UART/콘솔 없이도 흐름이 보이게 한다.

#### `AppCore` (struct)

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint8_t can_enabled`
- `uint8_t led1_enabled`
- `uint8_t slave1_mode`
- `uint8_t slave1_last_sample_pressed`
- `uint8_t slave1_stable_pressed`
- `uint8_t slave1_same_sample_count`
- `uint32_t heartbeat_count`
- `CanModule can_module`
- `LedModule slave1_led`
- `char mode_text[32]`
- `char button_text[32]`
- `char adc_text[48]`
- `char can_input_text[48]`

## 프로젝트 전용 함수 상세

### `app/app_config.h`

CAN 현장 반응 slave 최소 운영 버전용 설정 헤더다. slave1이 실제 동작에 쓰는 peer node id, 버튼/LED/CAN 주기, 그리고 승인 blink 횟수만 남겨 설정도 역할 중심으로 줄인다.

### `app/app_core.c`

CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다.

#### `static void AppCore_SetText(char *buffer, size_t size, const char *text)`

- 위치: `app/app_core.c:16`
- 역할: `AppCore_SetText`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `char *buffer`, `size_t size`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`

#### `void AppCore_SetModeText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:26`
- 역할: `AppCore_SetModeText`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->mode_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetButtonText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:34`
- 역할: `AppCore_SetButtonText`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->button_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetAdcText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:42`
- 역할: `AppCore_SetAdcText`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->adc_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetCanInputText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:50`
- 역할: `AppCore_SetCanInputText`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->can_input_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetResultText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:58`
- 역할: `AppCore_SetResultText`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppCore_InitDefaultTexts(AppCore *app)`

- 위치: `app/app_core.c:64`
- 역할: `AppCore_InitDefaultTexts`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetModeText`, `AppCore_SetButtonText`, `AppCore_SetAdcText`, `AppCore_SetCanInputText`

#### `uint8_t AppCore_QueueCanCommandCode(AppCore *app, uint8_t target_node_id, uint8_t command_code, uint8_t need_response)`

- 위치: `app/app_core.c:77`
- 역할: `AppCore_QueueCanCommandCode`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint8_t target_node_id`, `uint8_t command_code`, `uint8_t need_response`
- 로컬 변수: 없음
- 접근 상태/필드: `app->can_enabled`, `app->can_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanModule_QueueCommand`

#### `InfraStatus AppCore_InitConsoleCan(AppCore *app)`

- 위치: `app/app_core.c:95`
- 역할: `AppCore_InitConsoleCan`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: `CanModuleConfig can_config`
- 접근 상태/필드: `app->local_node_id`, `app->can_module`, `app->can_enabled`, `can_config.local_node_id`, `can_config.default_timeout_ms`, `can_config.max_submit_per_tick`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanModule_Init`

#### `static void AppCore_HandleCanIncoming(AppCore *app, const CanMessage *message)`

- 위치: `app/app_core.c:117`
- 역할: `AppCore_HandleCanIncoming`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const CanMessage *message`
- 로컬 변수: `uint8_t response_code`
- 접근 상태/필드: `message->message_type`, `message->flags`, `app->can_module`, `message->source_node_id`, `message->request_id`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppSlave1_HandleCanCommand`, `CanModule_QueueResponse`

#### `InfraStatus AppCore_Init(AppCore *app)`

- 위치: `app/app_core.c:144`
- 역할: `AppCore_Init`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: `app->local_node_id`, `app->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `RuntimeIo_GetLocalNodeId`, `AppCore_InitDefaultTexts`, `AppSlave1_Init`

#### `void AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:167`
- 역할: `AppCore_TaskHeartbeat`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->heartbeat_count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void AppCore_TaskCan(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:177`
- 역할: `AppCore_TaskCan`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: `CanServiceResult result`, `CanMessage message`
- 접근 상태/필드: `app->can_enabled`, `app->can_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanModule_Task`, `CanModule_TryPopResult`, `CanModule_TryPopIncoming`, `AppCore_HandleCanIncoming`

#### `void AppCore_TaskButton(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:200`
- 역할: `AppCore_TaskButton`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppSlave1_TaskButton`

#### `void AppCore_TaskLed(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:210`
- 역할: `AppCore_TaskLed`는 CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 구현부다. slave1에 필요한 CAN 수신/응답, 버튼 debounce, LED 상태기계만 남겨, UART 콘솔 없이도 현장 반응 로직이 독립적으로 동작하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppSlave1_TaskLed`

### `app/app_core.h`

CAN 현장 반응 slave 최소 운영 버전용 애플리케이션 context 헤더다. slave1이 실제로 쓰는 CAN 상태, 버튼 debounce 상태, 로컬 LED 제어기만 남겨 UART/콘솔 없이도 흐름이 보이게 한다.

노출 타입

- `AppCore` (struct)

### `app/app_core_internal.h`

CAN 현장 반응 slave 내부 helper 선언이다. slave1 정책이 모드 문자열과 CAN helper를 사용하더라도, 외부에는 실제 task 진입점만 보이도록 보조 함수를 분리한다.

### `app/app_slave1.c`

slave1 policy 구현부다. CAN 명령을 LED 동작으로 바꾸고, 안정된 버튼 입력을 master로 보내는 승인 요청으로 변환한다.

#### `InfraStatus AppSlave1_Init(AppCore *app)`

- 위치: `app/app_slave1.c:20`
- 역할: 반응 노드 역할을 초기화한다. slave1은 공통 console과 CAN 경로, 그리고 emergency/approval 표시를 위한 로컬 LED 제어기가 필요하다.
- 파라미터: `AppCore *app`
- 로컬 변수: `LedConfig led_config`
- 접근 상태/필드: `app->slave1_led`, `app->led1_enabled`, `app->slave1_mode`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_InitConsoleCan`, `RuntimeIo_GetSlave1LedConfig`, `LedModule_Init`

#### `void AppSlave1_HandleCanCommand(AppCore *app, const CanMessage *message, uint8_t *out_response_code)`

- 위치: `app/app_slave1.c:49`
- 역할: CAN 제어 명령을 로컬 slave1 동작으로 바꾼다. master는 이 경로를 통해, slave1을 emergency 상태로 두거나 승인 완료 표시, 출력 해제를 지시한다.
- 파라미터: `AppCore *app`, `const CanMessage *message`, `uint8_t *out_response_code`
- 로컬 변수: 없음
- 접근 상태/필드: `app->led1_enabled`, `message->payload`, `app->slave1_mode`, `app->slave1_led`, `message->source_node_id`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_SetPattern`, `AppCore_SetModeText`, `AppCore_SetButtonText`, `AppCore_SetAdcText`, `snprintf`, `in`, `AppCore_SetCanInputText`, `LedModule_StartGreenAckBlink`

#### `void AppSlave1_TaskButton(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_slave1.c:116`
- 역할: 로컬 승인 버튼을 debounce하고 안정된 입력만 보고한다. slave1은 master가 만든 emergency 상태에서 버튼이 눌릴 때만, OK 요청을 올린다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: `uint8_t raw_pressed`
- 접근 상태/필드: `app->can_enabled`, `app->slave1_last_sample_pressed`, `app->slave1_same_sample_count`, `app->slave1_stable_pressed`, `app->slave1_mode`
- 사용 전역/static: 없음
- 직접 호출 함수: `RuntimeIo_ReadSlave1ButtonPressed`, `AppCore_SetButtonText`, `AppCore_QueueCanCommandCode`, `AppCore_SetResultText`

#### `void AppSlave1_TaskLed(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_slave1.c:170`
- 역할: 로컬 LED pattern 상태기계를 진행시킨다. 유한한 acknowledgement blink 시퀀스를 여기서 끝까지 수행하여, slave1이 이후 정상 standby 상태로 돌아가게 한다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->led1_enabled`, `app->slave1_led`, `app->slave1_mode`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_Task`, `LedModule_GetPattern`, `AppCore_SetModeText`, `AppCore_SetButtonText`

### `app/app_slave1.h`

CAN 기반 현장 반응 노드인 slave1의 policy 인터페이스다. slave1은 LED 출력과 버튼 승인 입력, master에서 오는 CAN 명령 처리에 집중한다.

### `main.c`

역할별 펌웨어 이미지의 프로그램 시작점이다. 실행 파일은 최대한 얇게 두고, 보드 초기화와 task 등록, 역할별 동작은 runtime 계층에서 맡는다.

#### `int main(void)`

- 위치: `main.c:8`
- 역할: `main`는 역할별 펌웨어 이미지의 프로그램 시작점이다. 실행 파일은 최대한 얇게 두고, 보드 초기화와 task 등록, 역할별 동작은 runtime 계층에서 맡는다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `Runtime_Init`, `Runtime_Run`

### `runtime/runtime.c`

CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다.

파일 전역 상태

- `g_runtime`

#### `static void Runtime_TaskHeartbeat(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:23`
- 역할: `Runtime_TaskHeartbeat`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskHeartbeat`

#### `static void Runtime_TaskCan(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:28`
- 역할: `Runtime_TaskCan`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskCan`

#### `static void Runtime_TaskButton(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:33`
- 역할: `Runtime_TaskButton`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskButton`

#### `static void Runtime_TaskLed(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:38`
- 역할: `Runtime_TaskLed`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskLed`

#### `static void Runtime_BuildTaskTable(RuntimeContext *runtime)`

- 위치: `runtime/runtime.c:43`
- 역할: `Runtime_BuildTaskTable`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `RuntimeContext *runtime`
- 로컬 변수: 없음
- 접근 상태/필드: `runtime->tasks`, `runtime->app`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void Runtime_FaultLoop(void)`

- 위치: `runtime/runtime.c:70`
- 역할: `Runtime_FaultLoop`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus Runtime_Init(void)`

- 위치: `runtime/runtime.c:77`
- 역할: `Runtime_Init`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `InfraStatus status`, `uint32_t start_ms`
- 접근 상태/필드: `g_runtime.initialized`, `g_runtime.init_status`, `g_runtime.app`, `g_runtime.tasks`
- 사용 전역/static: `g_runtime`
- 직접 호출 함수: `RuntimeIo_BoardInit`, `RuntimeTick_Init`, `AppCore_Init`, `Runtime_BuildTaskTable`, `RuntimeTick_GetMs`, `RuntimeTask_ResetTable`, `INFRA_ARRAY_COUNT`

#### `void Runtime_Run(void)`

- 위치: `runtime/runtime.c:117`
- 역할: `Runtime_Run`는 CAN 현장 반응 slave 최소 운영 버전용 runtime 구현부다. slave1이 실제로 쓰는 button, can, led task만 등록하여, UART/render 같은 관찰용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: `g_runtime.initialized`, `g_runtime.init_status`, `g_runtime.tasks`
- 사용 전역/static: `g_runtime`
- 직접 호출 함수: `Runtime_FaultLoop`, `RuntimeTask_RunDue`, `INFRA_ARRAY_COUNT`, `RuntimeTick_GetMs`

### `runtime/runtime.h`

펌웨어 이미지의 최상위 runtime 인터페이스다. 애플리케이션은 이 함수들만 호출하면 시스템을 초기화하고, 무한 cooperative scheduler로 진입할 수 있다.

### `runtime/runtime_io.c`

CAN 현장 반응 slave용 실제 보드 바인딩 구현부다. slave1에 필요한 버튼 입력과 LED 배선만 남기고, 플랫폼 세부사항은 driver 계층 아래로 숨긴다.

#### `InfraStatus RuntimeIo_BoardInit(void)`

- 위치: `runtime/runtime_io.c:11`
- 역할: `RuntimeIo_BoardInit`는 CAN 현장 반응 slave용 실제 보드 바인딩 구현부다. slave1에 필요한 버튼 입력과 LED 배선만 남기고, 플랫폼 세부사항은 driver 계층 아래로 숨긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `BoardHw_Init`

#### `uint8_t RuntimeIo_GetLocalNodeId(void)`

- 위치: `runtime/runtime_io.c:16`
- 역할: `RuntimeIo_GetLocalNodeId`는 CAN 현장 반응 slave용 실제 보드 바인딩 구현부다. slave1에 필요한 버튼 입력과 LED 배선만 남기고, 플랫폼 세부사항은 driver 계층 아래로 숨긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus RuntimeIo_GetSlave1LedConfig(LedConfig *out_config)`

- 위치: `runtime/runtime_io.c:21`
- 역할: `RuntimeIo_GetSlave1LedConfig`는 CAN 현장 반응 slave용 실제 보드 바인딩 구현부다. slave1에 필요한 버튼 입력과 LED 배선만 남기고, 플랫폼 세부사항은 driver 계층 아래로 숨긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LedConfig *out_config`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `BoardHw_GetRgbLedConfig`

#### `uint8_t RuntimeIo_ReadSlave1ButtonPressed(void)`

- 위치: `runtime/runtime_io.c:26`
- 역할: `RuntimeIo_ReadSlave1ButtonPressed`는 CAN 현장 반응 slave용 실제 보드 바인딩 구현부다. slave1에 필요한 버튼 입력과 LED 배선만 남기고, 플랫폼 세부사항은 driver 계층 아래로 숨긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `BoardHw_ReadSlave1ButtonPressed`

### `runtime/runtime_io.h`

CAN 현장 반응 slave가 실제로 사용하는 보드 바인딩 인터페이스다. slave1은 버튼과 로컬 LED 배선만 필요하므로, LIN과 ADC 관련 API를 제거해 보드 계층을 단순화한다.

## 공통 모듈 참고

이 프로젝트가 실제로 사용하는 공통 `core/drivers/services/platform` 함수 상세는 [S32K 공통 모듈 참고서](./S32K_common_modules_reference.md)에 정리했다.

