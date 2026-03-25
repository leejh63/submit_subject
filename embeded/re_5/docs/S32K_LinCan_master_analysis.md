# S32K LIN/CAN Master 분석

UART 콘솔, CAN, LIN을 모두 사용하는 coordinator 노드다. slave2의 LIN 센서 상태를 읽고 policy를 판단해 slave1으로 CAN command를 보내며, operator 명령을 UART 콘솔에서 받아 CAN/LIN 동작으로 변환한다.

## 전체 흐름

1. `main()`이 `Runtime_Init()`을 호출해 보드 초기화, tick 초기화, AppCore 초기화, tick hook 등록, task table 구성을 순서대로 끝낸다.
2. `RuntimeIo_BoardInit()`은 보드 초기화 뒤 LIN transceiver를 켜고, `RuntimeIo_GetMasterLinConfig()`는 LIN master binding과 PID/token 상수를 조립한다.
3. `AppCore_Init()`은 AppCore를 초기화하고 `AppMaster_Init()`을 호출해 UART 콘솔, CAN 모듈, LIN 모듈을 올린다.
4. `RuntimeTick_RegisterHook(AppCore_OnTickIsr, &g_runtime.app)`가 설정되어 base tick 인터럽트마다 `LinModule_OnBaseTick()`이 호출된다.
5. `Runtime_BuildTaskTable()`은 `uart`, `lin_fast`, `can`, `lin_poll`, `render`, `heartbeat` task를 등록한다.
6. `AppCore_TaskUart()`는 `AppConsole_Task()`를 통해 명령 입력, 에러 처리, recover, CAN 명령 큐 적재를 담당한다.
7. `AppCore_TaskCan()`은 콘솔에서 꺼낸 명령을 CAN request로 바꾸고, CAN 결과/수신 메시지를 읽어 `AppMaster_HandleCanCommand()` 또는 UI 문자열 갱신으로 연결한다.
8. `AppCore_TaskLinFast()`는 `LinModule_TaskFast()`와 `LinModule_ConsumeFreshStatus()`를 통해 새 센서 상태를 잡고, `AppMaster_HandleFreshLinStatus()`에서 emergency 정책과 slave1 제어를 결정한다.
9. `AppCore_TaskLinPoll()`은 local ok 요청과 pending ok 요청을 LIN token 전송으로 연결하고, `AppMaster_AfterLinPoll()`에서 재시도 policy를 수행한다.
10. `AppCore_TaskRender()`는 task/source/value 화면 문자열을 합성해서 UART 콘솔 화면을 갱신한다.

## 주요 설정 상수

- `APP_NODE_ID_MASTER`: `1U`
- `APP_NODE_ID_SLAVE1`: `2U`
- `APP_TASK_UART_MS`: `1U`
- `APP_TASK_LIN_FAST_MS`: `1U`
- `APP_TASK_CAN_MS`: `10U`
- `APP_TASK_LIN_POLL_MS`: `20U`
- `APP_TASK_RENDER_MS`: `100U`
- `APP_TASK_HEARTBEAT_MS`: `1000U`
- `APP_CONSOLE_LINE_BUFFER_SIZE`: `64U`
- `APP_CONSOLE_INPUT_VIEW_SIZE`: `64U`
- `APP_CONSOLE_TASK_VIEW_SIZE`: `160U`
- `APP_CONSOLE_SOURCE_VIEW_SIZE`: `128U`
- `APP_CONSOLE_RESULT_VIEW_SIZE`: `96U`
- `APP_CONSOLE_VALUE_VIEW_SIZE`: `128U`
- `APP_CONSOLE_CAN_CMD_QUEUE_SIZE`: `4U`

## 프로젝트 핵심 자료구조

### `app/app_core.h`

master 노드 전용 애플리케이션 context 헤더다. coordinator가 실제로 사용하는 console, CAN, LIN 상태만 남겨, 다른 노드용 필드 없이 핵심 흐름을 바로 읽을 수 있게 한다.

#### `AppCore` (struct)

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint8_t console_enabled`
- `uint8_t can_enabled`
- `uint8_t lin_enabled`
- `uint8_t master_emergency_active`
- `uint8_t master_slave1_ok_pending`
- `uint8_t can_last_activity`
- `uint8_t lin_last_reported_zone`
- `uint8_t lin_last_reported_lock`
- `uint32_t heartbeat_count`
- `uint32_t uart_task_count`
- `uint32_t can_task_count`
- `AppConsole console`
- `CanModule can_module`
- `LinModule lin_module`
- `char mode_text[32]`
- `char button_text[32]`
- `char adc_text[48]`
- `char can_input_text[48]`
- `char lin_input_text[48]`
- `char lin_link_text[32]`

### `app/app_console.h`

사람이 읽을 수 있는 제어와 상태 표시용 UART 콘솔 인터페이스다. 콘솔은 화면 렌더 상태를 유지하고 명령을 파싱하며, queue에 쌓인 CAN 요청을 app 계층에 노출한다.

#### `AppConsoleCanCommand` (struct)

파싱된 CAN 지향 콘솔 명령 구조체다. 콘솔은 operator 의도를 이 중립적인 형태로 저장해 두고, AppCore가 나중에 CAN 모듈 요청으로 변환한다.

- `uint8_t type`
- `uint8_t target_node_id`
- `uint8_t target_is_broadcast`
- `char text[CAN_TEXT_MAX_LEN + 1U]`
- `uint8_t event_code`
- `uint8_t arg0`
- `uint8_t arg1`

#### `AppConsoleView` (struct)

콘솔 view 문자열과 dirty flag를 캐시하는 구조체다. 렌더링은 이 구조를 사용해, 마지막 UART 전송 이후 바뀐 부분만 다시 그린다.

- `char input_text[APP_CONSOLE_INPUT_VIEW_SIZE]`
- `char task_text[APP_CONSOLE_TASK_VIEW_SIZE]`
- `char source_text[APP_CONSOLE_SOURCE_VIEW_SIZE]`
- `char result_text[APP_CONSOLE_RESULT_VIEW_SIZE]`
- `char value_text[APP_CONSOLE_VALUE_VIEW_SIZE]`
- `uint8_t full_refresh_required`
- `uint8_t input_dirty`
- `uint8_t task_dirty`
- `uint8_t source_dirty`
- `uint8_t result_dirty`
- `uint8_t value_dirty`
- `uint8_t layout_drawn`

#### `AppConsole` (struct)

하나의 펌웨어 이미지가 소유하는 전체 UART 콘솔 상태다. UART transport와 operator 요청 queue, 그리고 현재 화면에 표시된 task/source/value view를 모두 가진다.

- `uint8_t initialized`
- `uint8_t node_id`
- `AppConsoleState state`
- `uint8_t local_ok_pending`
- `UartService uart`
- `InfraQueue can_cmd_queue`
- `AppConsoleCanCommand can_cmd_storage[APP_CONSOLE_CAN_CMD_QUEUE_SIZE]`
- `AppConsoleView view`

## 프로젝트 전용 함수 상세

### `app/app_config.h`

master 슬림 프로젝트 전용 설정 헤더다. coordinator가 실제로 쓰는 노드 식별값, task 주기, 콘솔 버퍼 크기만 남겨 설정 파일도 역할 중심으로 단순화한다.

### `app/app_console.c`

UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다.

#### `static void AppConsole_RequestFullRefresh(AppConsole *console)`

- 위치: `app/app_console.c:110`
- 역할: `AppConsole_RequestFullRefresh`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `view.full_refresh_required`, `view.input_dirty`, `view.task_dirty`, `view.source_dirty`, `view.result_dirty`, `view.value_dirty`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_RequestInputRefresh(AppConsole *console)`

- 위치: `app/app_console.c:125`
- 역할: `AppConsole_RequestInputRefresh`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `view.input_dirty`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_RequestTaskRefresh(AppConsole *console)`

- 위치: `app/app_console.c:133`
- 역할: `AppConsole_RequestTaskRefresh`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `view.task_dirty`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_RequestSourceRefresh(AppConsole *console)`

- 위치: `app/app_console.c:141`
- 역할: `AppConsole_RequestSourceRefresh`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `view.source_dirty`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_RequestResultRefresh(AppConsole *console)`

- 위치: `app/app_console.c:149`
- 역할: `AppConsole_RequestResultRefresh`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `view.result_dirty`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_RequestValueRefresh(AppConsole *console)`

- 위치: `app/app_console.c:157`
- 역할: `AppConsole_RequestValueRefresh`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `view.value_dirty`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_SetViewText(char *dst, uint16_t dst_size, const char *src, uint8_t *out_changed)`

- 위치: `app/app_console.c:165`
- 역할: `AppConsole_SetViewText`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `char *dst`, `uint16_t dst_size`, `const char *src`, `uint8_t *out_changed`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`, `strncmp`

#### `void AppConsole_SetResultText(AppConsole *console, const char *text)`

- 위치: `app/app_console.c:185`
- 역할: `AppConsole_SetResultText`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`, `const char *text`
- 로컬 변수: `uint8_t changed`
- 접근 상태/필드: `console->view`, `view.result_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_SetViewText`, `AppConsole_RequestResultRefresh`

#### `void AppConsole_SetTaskText(AppConsole *console, const char *text)`

- 위치: `app/app_console.c:204`
- 역할: `AppConsole_SetTaskText`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`, `const char *text`
- 로컬 변수: `uint8_t changed`
- 접근 상태/필드: `console->view`, `view.task_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_SetViewText`, `AppConsole_RequestTaskRefresh`

#### `void AppConsole_SetSourceText(AppConsole *console, const char *text)`

- 위치: `app/app_console.c:223`
- 역할: `AppConsole_SetSourceText`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`, `const char *text`
- 로컬 변수: `uint8_t changed`
- 접근 상태/필드: `console->view`, `view.source_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_SetViewText`, `AppConsole_RequestSourceRefresh`

#### `void AppConsole_SetValueText(AppConsole *console, const char *text)`

- 위치: `app/app_console.c:242`
- 역할: `AppConsole_SetValueText`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`, `const char *text`
- 로컬 변수: `uint8_t changed`
- 접근 상태/필드: `console->view`, `view.value_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_SetViewText`, `AppConsole_RequestValueRefresh`

#### `static void AppConsole_UpdateInputView(AppConsole *console)`

- 위치: `app/app_console.c:261`
- 역할: `AppConsole_UpdateInputView`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: `uint8_t changed`
- 접근 상태/필드: `console->uart`, `console->view`, `view.input_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_GetCurrentInputText`, `AppConsole_SetViewText`, `AppConsole_RequestInputRefresh`

#### `static void AppConsole_RenderLayout(AppConsole *console)`

- 위치: `app/app_console.c:288`
- 역할: `AppConsole_RenderLayout`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: `const char *title_text`
- 접근 상태/필드: `console->node_id`, `console->uart`, `console->view`, `view.layout_drawn`
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`, `UartService_RequestTx`

#### `static void AppConsole_ExtractTextLine(const char *src, uint8_t line_index, char *out_buffer, uint16_t out_buffer_size)`

- 위치: `app/app_console.c:332`
- 역할: `AppConsole_ExtractTextLine`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const char *src`, `uint8_t line_index`, `char *out_buffer`, `uint16_t out_buffer_size`
- 로컬 변수: `uint8_t current_line`, `uint16_t out_index`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void AppConsole_RenderDirtyLines(AppConsole *console)`

- 위치: `app/app_console.c:381`
- 역할: `AppConsole_RenderDirtyLines`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->view`, `console->node_id`, `console->uart`, `view.input_dirty`, `view.input_text`, `view.task_dirty`, `view.task_text`, `view.source_dirty`, `view.source_text`, `view.result_dirty`, `view.result_text`, `view.value_dirty`, `view.value_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`, `AppConsole_ExtractTextLine`, `UartService_RequestTx`

#### `static uint8_t AppConsole_IsSpaceChar(char ch)`

- 위치: `app/app_console.c:496`
- 역할: `AppConsole_IsSpaceChar`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `char ch`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t AppConsole_ParseU8(const char *text, uint8_t *out_value)`

- 위치: `app/app_console.c:501`
- 역할: `AppConsole_ParseU8`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const char *text`, `uint8_t *out_value`
- 로컬 변수: `uint16_t value`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t AppConsole_ParseTarget(const char *text, uint8_t *out_node_id, uint8_t *out_is_broadcast)`

- 위치: `app/app_console.c:531`
- 역할: `AppConsole_ParseTarget`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const char *text`, `uint8_t *out_node_id`, `uint8_t *out_is_broadcast`
- 로컬 변수: `uint8_t node_id`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `strcmp`, `AppConsole_ParseU8`

#### `static uint8_t AppConsole_Tokenize(char *buffer, char *argv[], uint8_t argv_capacity, uint8_t *out_argc)`

- 위치: `app/app_console.c:562`
- 역할: `AppConsole_Tokenize`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `char *buffer`, `char *argv[]`, `uint8_t argv_capacity`, `uint8_t *out_argc`
- 로컬 변수: `uint8_t argc`, `char *cursor`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_IsSpaceChar`

#### `static void AppConsole_QueueCanCommand(AppConsole *console, const AppConsoleCanCommand *command)`

- 위치: `app/app_console.c:618`
- 역할: 파싱이 끝난 CAN 명령 하나를 콘솔 요청 큐에 넣는다. AppCore는 CAN task에서 이 큐를 비우므로, 사용자 입력 처리와 실제 통신 작업이 느슨하게 분리된다.
- 파라미터: `AppConsole *console`, `const AppConsoleCanCommand *command`
- 로컬 변수: 없음
- 접근 상태/필드: `console->can_cmd_queue`, `command->type`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_Push`, `AppConsole_SetResultText`

#### `static void AppConsole_HandleLine(AppConsole *console, const char *line)`

- 위치: `app/app_console.c:657`
- 역할: `AppConsole_HandleLine`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`, `const char *line`
- 로컬 변수: `uint8_t argc`, `AppConsoleCanCommand command`, `uint8_t index`, `uint8_t target_is_broadcast`, `size_t remaining`, `char *write_ptr`, `int written`
- 접근 상태/필드: `console->state`, `console->uart`, `console->can_cmd_queue`, `console->local_ok_pending`, `command.target_node_id`, `command.target_is_broadcast`, `command.type`, `command.text`, `command.event_code`, `command.arg0`, `command.arg1`
- 사용 전역/static: 없음
- 직접 호출 함수: `strcmp`, `AppConsole_SetResultText`, `snprintf`, `UartService_HasError`, `UartService_GetCurrentInputLength`, `UartService_IsTxBusy`, `InfraQueue_GetCount`, `InfraQueue_GetCapacity`, `AppConsole_Tokenize`, `memset`, `AppConsole_ParseTarget`, `AppConsole_QueueCanCommand`, `AppConsole_ParseU8`

#### `InfraStatus AppConsole_Init(AppConsole *console, uint8_t node_id)`

- 위치: `app/app_console.c:837`
- 역할: 콘솔 상태와 UART transport를 초기화한다. live 데이터가 오기 전에도 operator가 구조화된 화면을 보도록, view에 기본 placeholder 문자열을 미리 채워둔다.
- 파라미터: `AppConsole *console`, `uint8_t node_id`
- 로컬 변수: 없음
- 접근 상태/필드: `console->node_id`, `console->state`, `console->can_cmd_queue`, `console->can_cmd_storage`, `console->uart`, `console->view`, `console->initialized`, `view.task_text`, `view.source_text`, `view.result_text`, `view.value_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `InfraQueue_Init`, `UartService_Init`, `snprintf`, `AppConsole_RequestFullRefresh`

#### `void AppConsole_Task(AppConsole *console, uint32_t now_ms)`

- 위치: `app/app_console.c:892`
- 역할: UART RX/TX 작업을 진행시키고 완성된 입력 줄을 처리한다. 오류 처리와 `recover` 명령도 여기 들어 있어, 펌웨어 재시작 없이 콘솔을 복구할 수 있게 한다.
- 파라미터: `AppConsole *console`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `console->initialized`, `console->uart`, `console->state`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_ProcessRx`, `UartService_HasError`, `AppConsole_SetResultText`, `AppConsole_GetUartErrorText`, `UartService_GetErrorCode`, `UartService_HasLine`, `UartService_ReadLine`, `strcmp`, `UartService_Recover`, `AppConsole_RequestFullRefresh`, `stopped`, `AppConsole_HandleLine`, `UartService_ProcessTx`

#### `void AppConsole_Render(AppConsole *console)`

- 위치: `app/app_console.c:949`
- 역할: 현재 dirty flag에 따라 콘솔 UI를 갱신한다. layout과 바뀐 줄을 분리해서 보내므로, 작은 텍스트 필드 하나만 변해도 효율적으로 렌더링할 수 있다.
- 파라미터: `AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->initialized`, `console->state`, `console->view`, `view.layout_drawn`, `view.full_refresh_required`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_UpdateInputView`, `AppConsole_RenderLayout`, `AppConsole_RenderDirtyLines`

#### `uint8_t AppConsole_TryPopCanCommand(AppConsole *console, AppConsoleCanCommand *out_cmd)`

- 위치: `app/app_console.c:975`
- 역할: `AppConsole_TryPopCanCommand`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`, `AppConsoleCanCommand *out_cmd`
- 로컬 변수: 없음
- 접근 상태/필드: `console->can_cmd_queue`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_Pop`

#### `uint8_t AppConsole_ConsumeLocalOk(AppConsole *console)`

- 위치: `app/app_console.c:985`
- 역할: `AppConsole_ConsumeLocalOk`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppConsole *console`
- 로컬 변수: `uint8_t pending`
- 접근 상태/필드: `console->local_ok_pending`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t AppConsole_IsError(const AppConsole *console)`

- 위치: `app/app_console.c:999`
- 역할: `AppConsole_IsError`는 UART 콘솔 구현부다. 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며, AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const AppConsole *console`
- 로컬 변수: 없음
- 접근 상태/필드: `console->state`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `app/app_console.h`

사람이 읽을 수 있는 제어와 상태 표시용 UART 콘솔 인터페이스다. 콘솔은 화면 렌더 상태를 유지하고 명령을 파싱하며, queue에 쌓인 CAN 요청을 app 계층에 노출한다.

노출 타입

- `AppConsoleCanCommandType` (enum)
- `AppConsoleState` (enum)
- `AppConsoleCanCommand` (struct)
- `AppConsoleView` (struct)
- `AppConsole` (struct)

### `app/app_core.c`

master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다.

#### `static void AppCore_SetText(char *buffer, size_t size, const char *text)`

- 위치: `app/app_core.c:17`
- 역할: `AppCore_SetText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `char *buffer`, `size_t size`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`

#### `void AppCore_SetModeText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:27`
- 역할: `AppCore_SetModeText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->mode_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetButtonText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:35`
- 역할: `AppCore_SetButtonText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->button_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetAdcText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:43`
- 역할: `AppCore_SetAdcText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->adc_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetCanInputText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:51`
- 역할: `AppCore_SetCanInputText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->can_input_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetLinInputText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:59`
- 역할: `AppCore_SetLinInputText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->lin_input_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetLinLinkText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:67`
- 역할: `AppCore_SetLinLinkText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->lin_link_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetResultText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:75`
- 역할: `AppCore_SetResultText`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->console_enabled`, `app->console`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_SetResultText`

#### `static void AppCore_InitDefaultTexts(AppCore *app)`

- 위치: `app/app_core.c:104`
- 역할: `AppCore_InitDefaultTexts`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetModeText`, `AppCore_SetButtonText`, `AppCore_SetAdcText`, `AppCore_SetCanInputText`, `AppCore_SetLinInputText`, `AppCore_SetLinLinkText`

#### `uint8_t AppCore_QueueCanCommandCode(AppCore *app, uint8_t target_node_id, uint8_t command_code, uint8_t need_response)`

- 위치: `app/app_core.c:119`
- 역할: `AppCore_QueueCanCommandCode`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint8_t target_node_id`, `uint8_t command_code`, `uint8_t need_response`
- 로컬 변수: 없음
- 접근 상태/필드: `app->can_enabled`, `app->can_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanModule_QueueCommand`

#### `InfraStatus AppCore_InitConsoleCan(AppCore *app)`

- 위치: `app/app_core.c:137`
- 역할: `AppCore_InitConsoleCan`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: `CanModuleConfig can_config`
- 접근 상태/필드: `app->console`, `app->local_node_id`, `app->console_enabled`, `app->can_module`, `app->can_enabled`, `can_config.local_node_id`, `can_config.default_timeout_ms`, `can_config.max_submit_per_tick`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_Init`, `memset`, `CanModule_Init`

#### `static void AppCore_FormatCanResult(const CanServiceResult *result, char *buffer, size_t size)`

- 위치: `app/app_core.c:164`
- 역할: `AppCore_FormatCanResult`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanServiceResult *result`, `char *buffer`, `size_t size`
- 로컬 변수: `const char *name`
- 접근 상태/필드: `result->command_code`, `result->kind`, `result->source_node_id`, `result->result_code`
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`

#### `static void AppCore_HandleCanIncoming(AppCore *app, const CanMessage *message)`

- 위치: `app/app_core.c:242`
- 역할: `AppCore_HandleCanIncoming`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const CanMessage *message`
- 로컬 변수: `uint8_t response_code`
- 접근 상태/필드: `message->message_type`, `message->payload`, `message->source_node_id`, `message->text`, `message->flags`, `app->can_module`, `message->request_id`
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`, `AppCore_SetResultText`, `AppMaster_HandleCanCommand`, `CanModule_QueueResponse`

#### `InfraStatus AppCore_Init(AppCore *app)`

- 위치: `app/app_core.c:301`
- 역할: `AppCore_Init`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: `app->local_node_id`, `app->lin_last_reported_zone`, `app->lin_last_reported_lock`, `app->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `RuntimeIo_GetLocalNodeId`, `AppCore_InitDefaultTexts`, `AppMaster_Init`

#### `void AppCore_OnTickIsr(void *context)`

- 위치: `app/app_core.c:326`
- 역할: `AppCore_OnTickIsr`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`
- 로컬 변수: `AppCore *app`
- 접근 상태/필드: `app->lin_enabled`, `app->lin_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_OnBaseTick`

#### `void AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:339`
- 역할: `AppCore_TaskHeartbeat`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->heartbeat_count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void AppCore_TaskUart(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:349`
- 역할: `AppCore_TaskUart`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->console_enabled`, `app->uart_task_count`, `app->console`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_Task`

#### `void AppCore_TaskCan(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:360`
- 역할: `AppCore_TaskCan`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: `AppConsoleCanCommand command`, `CanServiceResult result`, `CanMessage message`, `uint8_t activity`
- 접근 상태/필드: `app->can_enabled`, `app->console_enabled`, `app->console`, `app->can_module`, `app->can_last_activity`, `app->can_task_count`, `command.type`, `command.target_node_id`, `command.text`, `command.event_code`, `command.arg0`, `command.arg1`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_TryPopCanCommand`, `CanModule_QueueCommand`, `CanModule_QueueText`, `CanModule_QueueEvent`, `CanModule_Task`, `CanModule_TryPopResult`, `AppCore_FormatCanResult`, `AppCore_SetResultText`, `CanModule_TryPopIncoming`, `AppCore_HandleCanIncoming`

#### `void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:460`
- 역할: `AppCore_TaskLinFast`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: `LinStatusFrame status`
- 접근 상태/필드: `app->lin_enabled`, `app->lin_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_TaskFast`, `LinModule_ConsumeFreshStatus`, `AppMaster_HandleFreshLinStatus`

#### `void AppCore_TaskLinPoll(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:476`
- 역할: `AppCore_TaskLinPoll`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->console_enabled`, `app->console`, `app->lin_enabled`, `app->lin_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppConsole_ConsumeLocalOk`, `AppMaster_RequestOk`, `LinModule_TaskPoll`, `AppMaster_AfterLinPoll`

#### `void AppCore_TaskRender(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:499`
- 역할: `AppCore_TaskRender`는 master 전용 애플리케이션 오케스트레이션 구현부다. coordinator에 필요한 console, CAN, LIN 경로만 연결하여, 다른 노드 역할 분기 없이 master 흐름만 남긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: `const char *can_text`, `const char *uart_text`
- 접근 상태/필드: `app->console_enabled`, `app->can_last_activity`, `app->console`, `app->heartbeat_count`, `app->can_task_count`, `app->lin_link_text`, `app->uart_task_count`, `app->can_input_text`, `app->lin_input_text`, `app->mode_text`, `app->button_text`, `app->adc_text`, `console.uart`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_HasError`, `snprintf`, `AppConsole_SetTaskText`, `AppConsole_SetSourceText`, `AppConsole_SetValueText`, `AppConsole_Render`

### `app/app_core.h`

master 노드 전용 애플리케이션 context 헤더다. coordinator가 실제로 사용하는 console, CAN, LIN 상태만 남겨, 다른 노드용 필드 없이 핵심 흐름을 바로 읽을 수 있게 한다.

노출 타입

- `AppCore` (struct)

### `app/app_core_internal.h`

master AppCore 구현 내부에서만 쓰는 helper 선언이다. coordinator 정책이 UI 문자열과 공통 CAN helper를 사용하더라도, 외부에는 필요한 인터페이스만 노출되도록 범위를 줄인다.

### `app/app_master.c`

master 노드 policy 구현부다. 시스템이 언제 emergency인지 판단하고, slave1 통지 시점과 slave2 latch 해제 가능 시점을 결정한다.

#### `InfraStatus AppMaster_Init(AppCore *app)`

- 위치: `app/app_master.c:20`
- 역할: master 역할의 통신 모듈을 초기화한다. coordinator는 항상 console, CAN, LIN이 필요하므로, startup은 이 공통 기능을 켜는 데 집중한다.
- 파라미터: `AppCore *app`
- 로컬 변수: `LinConfig lin_config`
- 접근 상태/필드: `app->lin_module`, `app->lin_enabled`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_InitConsoleCan`, `RuntimeIo_GetMasterLinConfig`, `LinModule_Init`, `RuntimeIo_AttachLinModule`, `AppCore_SetLinLinkText`

#### `void AppMaster_RequestOk(AppCore *app)`

- 위치: `app/app_master.c:53`
- 역할: slave1에서 온 로컬/원격 승인 요청을 처리한다. master는 센서 상태가 active emergency 구간을 벗어난 경우에만, 요청을 slave2로 전달한다.
- 파라미터: `AppCore *app`
- 로컬 변수: `LinStatusFrame status`
- 접근 상태/필드: `app->lin_enabled`, `app->lin_module`, `app->master_slave1_ok_pending`, `status.zone`, `status.emergency_latched`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetResultText`, `LinModule_GetLatestStatus`, `AppCore_SetButtonText`, `LinModule_RequestOk`, `AppCore_SetLinLinkText`

#### `void AppMaster_HandleFreshLinStatus(AppCore *app, const LinStatusFrame *status)`

- 위치: `app/app_master.c:96`
- 역할: 새로 수신한 slave2 상태 프레임을 해석한다. UI 상태를 갱신하고 emergency 전이를 추적하며, slave1으로 보낼 CAN 동작을 결정하는 master policy의 핵심이다.
- 파라미터: `AppCore *app`, `const LinStatusFrame *status`
- 로컬 변수: `uint8_t emergency_active`
- 접근 상태/필드: `status->adc_value`, `status->zone`, `status->emergency_latched`, `app->master_emergency_active`, `app->master_slave1_ok_pending`, `app->lin_last_reported_zone`, `app->lin_last_reported_lock`
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`, `u`, `AppCore_GetLinZoneText`, `AppCore_SetAdcText`, `AppCore_SetLinInputText`, `AppCore_SetLinLinkText`, `AppCore_SetModeText`, `AppCore_SetResultText`, `AppCore_SetButtonText`, `AppCore_QueueCanCommandCode`

#### `void AppMaster_HandleCanCommand(AppCore *app, const CanMessage *message, uint8_t *out_response_code)`

- 위치: `app/app_master.c:185`
- 역할: master가 수신한 역할 관련 CAN 명령을 처리한다. 현재 중요한 입력은 slave1의 OK 요청이며, 이것이 승인 판단 흐름을 시작시키는 계기가 된다.
- 파라미터: `AppCore *app`, `const CanMessage *message`, `uint8_t *out_response_code`
- 로컬 변수: 없음
- 접근 상태/필드: `message->source_node_id`, `message->payload`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetCanInputText`, `AppCore_SetButtonText`, `AppMaster_RequestOk`

#### `void AppMaster_AfterLinPoll(AppCore *app)`

- 위치: `app/app_master.c:209`
- 역할: 승인 대기 중일 때 LIN OK-token 요청을 재시도한다. master는 이 helper를 통해, fresh status가 latch 해제를 확인할 때까지 slave2를 계속 자극한다.
- 파라미터: `AppCore *app`
- 로컬 변수: `LinStatusFrame status`
- 접근 상태/필드: `app->master_slave1_ok_pending`, `app->lin_module`, `status.zone`, `status.emergency_latched`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_GetLatestStatus`, `LinModule_RequestOk`, `AppCore_SetLinLinkText`

### `app/app_master.h`

master 노드 역할의 policy 인터페이스다. master는 LIN 센서 상태와 CAN 현장 동작, 그리고 emergency 해제 승인 절차를 조정한다.

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

master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다.

파일 전역 상태

- `g_runtime`

#### `static void Runtime_TaskHeartbeat(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:23`
- 역할: `Runtime_TaskHeartbeat`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskHeartbeat`

#### `static void Runtime_TaskUart(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:28`
- 역할: `Runtime_TaskUart`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskUart`

#### `static void Runtime_TaskCan(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:33`
- 역할: `Runtime_TaskCan`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskCan`

#### `static void Runtime_TaskLinFast(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:38`
- 역할: `Runtime_TaskLinFast`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskLinFast`

#### `static void Runtime_TaskLinPoll(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:43`
- 역할: `Runtime_TaskLinPoll`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskLinPoll`

#### `static void Runtime_TaskRender(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:48`
- 역할: `Runtime_TaskRender`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskRender`

#### `static void Runtime_BuildTaskTable(RuntimeContext *runtime)`

- 위치: `runtime/runtime.c:53`
- 역할: `Runtime_BuildTaskTable`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `RuntimeContext *runtime`
- 로컬 변수: 없음
- 접근 상태/필드: `runtime->tasks`, `runtime->app`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void Runtime_FaultLoop(void)`

- 위치: `runtime/runtime.c:92`
- 역할: `Runtime_FaultLoop`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus Runtime_Init(void)`

- 위치: `runtime/runtime.c:99`
- 역할: `Runtime_Init`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `InfraStatus status`, `uint32_t start_ms`
- 접근 상태/필드: `g_runtime.initialized`, `g_runtime.init_status`, `g_runtime.app`, `g_runtime.tasks`
- 사용 전역/static: `g_runtime`
- 직접 호출 함수: `RuntimeIo_BoardInit`, `RuntimeTick_Init`, `AppCore_Init`, `RuntimeTick_ClearHooks`, `RuntimeTick_RegisterHook`, `Runtime_BuildTaskTable`, `RuntimeTick_GetMs`, `RuntimeTask_ResetTable`, `INFRA_ARRAY_COUNT`

#### `void Runtime_Run(void)`

- 위치: `runtime/runtime.c:147`
- 역할: `Runtime_Run`는 master 노드 전용 runtime 구현부다. coordinator가 실제로 쓰는 task만 등록하여, 다른 노드용 no-op task 없이 super-loop를 구성한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: `g_runtime.initialized`, `g_runtime.init_status`, `g_runtime.tasks`
- 사용 전역/static: `g_runtime`
- 직접 호출 함수: `Runtime_FaultLoop`, `RuntimeTask_RunDue`, `INFRA_ARRAY_COUNT`, `RuntimeTick_GetMs`

### `runtime/runtime.h`

펌웨어 이미지의 최상위 runtime 인터페이스다. 애플리케이션은 이 함수들만 호출하면 시스템을 초기화하고, 무한 cooperative scheduler로 진입할 수 있다.

### `runtime/runtime_io.c`

LIN/CAN master용 프로젝트 바인딩 구현부다. 역할별 LIN master 상수만 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다.

#### `InfraStatus RuntimeIo_BoardInit(void)`

- 위치: `runtime/runtime_io.c:22`
- 역할: `RuntimeIo_BoardInit`는 LIN/CAN master용 프로젝트 바인딩 구현부다. 역할별 LIN master 상수만 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `BoardHw_Init`, `BoardHw_EnableLinTransceiver`

#### `uint8_t RuntimeIo_GetLocalNodeId(void)`

- 위치: `runtime/runtime_io.c:36`
- 역할: `RuntimeIo_GetLocalNodeId`는 LIN/CAN master용 프로젝트 바인딩 구현부다. 역할별 LIN master 상수만 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void RuntimeIo_AttachLinModule(LinModule *module)`

- 위치: `runtime/runtime_io.c:41`
- 역할: `RuntimeIo_AttachLinModule`는 LIN/CAN master용 프로젝트 바인딩 구현부다. 역할별 LIN master 상수만 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LinHw_AttachModule`

#### `void RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid)`

- 위치: `runtime/runtime_io.c:47`
- 역할: `RuntimeIo_LinNotifyEvent`는 LIN/CAN master용 프로젝트 바인딩 구현부다. 역할별 LIN master 상수만 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinEventId event_id`, `uint8_t current_pid`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_OnEvent`

#### `InfraStatus RuntimeIo_GetMasterLinConfig(LinConfig *out_config)`

- 위치: `runtime/runtime_io.c:57`
- 역할: `RuntimeIo_GetMasterLinConfig`는 LIN/CAN master용 프로젝트 바인딩 구현부다. 역할별 LIN master 상수만 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinConfig *out_config`
- 로컬 변수: 없음
- 접근 상태/필드: `out_config->role`, `out_config->pid_status`, `out_config->pid_ok`, `out_config->ok_token`, `out_config->status_frame_size`, `out_config->ok_frame_size`, `out_config->timeout_ticks`, `out_config->poll_period_ms`, `out_config->binding`, `binding.init_fn`, `binding.master_send_header_fn`, `binding.start_receive_fn`, `binding.start_send_fn`, `binding.goto_idle_fn`, `binding.set_timeout_fn`, `binding.service_tick_fn`, `binding.context`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `LinHw_Configure`, `LinHw_IsSupported`

### `runtime/runtime_io.h`

master 노드가 실제로 사용하는 프로젝트 바인딩 인터페이스다. coordinator는 LIN master 설정과 event 연결만 필요하므로, 하드웨어 세부사항은 shared driver 계층 아래로 숨긴다.

## 공통 모듈 참고

이 프로젝트가 실제로 사용하는 공통 `core/drivers/services/platform` 함수 상세는 [S32K 공통 모듈 참고서](./S32K_common_modules_reference.md)에 정리했다.

