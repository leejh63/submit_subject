# S32K LIN Slave 분석

ADC 샘플링과 LIN 상태 게시를 담당하는 센서 노드다. ADC 값을 semantic zone으로 분류하고 emergency latch를 유지하며, master가 보낸 LIN OK token으로만 latch 해제를 허용한다.

## 전체 흐름

1. `main()`이 `Runtime_Init()`을 호출해 보드/LIN transceiver/tick/AppCore를 준비한다.
2. `RuntimeIo_GetSlave2AdcConfig()`가 ADC threshold와 sample callback을 묶고, `RuntimeIo_GetSlaveLinConfig()`가 LIN slave binding과 PID/token 상수를 조립한다.
3. `AppCore_Init()`은 AppCore를 초기화하고 `AppSlave2_Init()`으로 LED, ADC, LIN 모듈을 함께 시작한다.
4. `RuntimeTick_RegisterHook(AppCore_OnTickIsr, &g_runtime.app)`로 LIN timeout/service tick이 base interrupt에 묶인다.
5. `Runtime_BuildTaskTable()`은 `lin_fast`, `adc`, `led`, `heartbeat` task를 등록한다.
6. `AppCore_TaskAdc()`는 `AppSlave2_TaskAdc()`를 호출해 ADC를 샘플링하고 snapshot을 만들며, 이 snapshot을 LIN status cache와 로컬 LED pattern으로 동시에 반영한다.
7. `AppCore_TaskLinFast()`는 `AppSlave2_HandleLinOkToken()`을 호출해 master의 승인 token을 소비하고 emergency latch 해제 가능 여부를 검사한다.
8. `AppCore_TaskLed()`는 현재 zone/latch 상태에 맞는 LED 패턴을 유지한다.
9. LIN master가 status PID를 poll하면 `LinModule`이 최신 `slave_status_cache`를 전송하고, ok PID를 poll하면 ok token을 수신해 `ok_token_pending`으로 app 계층에 넘긴다.

## 주요 설정 상수

- `APP_NODE_ID_SLAVE2`: `3U`
- `APP_TASK_LIN_FAST_MS`: `1U`
- `APP_TASK_ADC_MS`: `20U`
- `APP_TASK_LIN_POLL_MS`: `20U`
- `APP_TASK_LED_MS`: `100U`
- `APP_TASK_HEARTBEAT_MS`: `1000U`

## 프로젝트 핵심 자료구조

### `app/app_core.h`

LIN sensor slave 전용 애플리케이션 context 헤더다. ADC, LED, LIN 상태 게시에 필요한 필드만 남겨, console과 CAN 없이 센서 노드 흐름만 바로 읽을 수 있게 한다.

#### `AppCore` (struct)

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint8_t lin_enabled`
- `uint8_t adc_enabled`
- `uint8_t led2_enabled`
- `uint32_t heartbeat_count`
- `LinModule lin_module`
- `LedModule slave2_led`
- `AdcModule adc_module`
- `char adc_text[48]`
- `char lin_input_text[48]`
- `char lin_link_text[32]`

## 프로젝트 전용 함수 상세

### `app/app_config.h`

LIN sensor slave 슬림 프로젝트 전용 설정 헤더다. 센서 노드가 실제로 쓰는 로컬 node id와 task 주기, ADC/LIN 바인딩에 필요한 최소 상수만 남긴다.

### `app/app_core.c`

LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다.

#### `static void AppCore_SetText(char *buffer, size_t size, const char *text)`

- 위치: `app/app_core.c:16`
- 역할: `AppCore_SetText`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `char *buffer`, `size_t size`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `snprintf`

#### `void AppCore_SetAdcText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:26`
- 역할: `AppCore_SetAdcText`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->adc_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetLinInputText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:34`
- 역할: `AppCore_SetLinInputText`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->lin_input_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `void AppCore_SetLinLinkText(AppCore *app, const char *text)`

- 위치: `app/app_core.c:42`
- 역할: `AppCore_SetLinLinkText`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `const char *text`
- 로컬 변수: 없음
- 접근 상태/필드: `app->lin_link_text`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetText`

#### `static void AppCore_InitDefaultTexts(AppCore *app)`

- 위치: `app/app_core.c:50`
- 역할: `AppCore_InitDefaultTexts`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_SetAdcText`, `AppCore_SetLinInputText`, `AppCore_SetLinLinkText`

#### `InfraStatus AppCore_Init(AppCore *app)`

- 위치: `app/app_core.c:62`
- 역할: `AppCore_Init`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: `app->local_node_id`, `app->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `RuntimeIo_GetLocalNodeId`, `AppCore_InitDefaultTexts`, `AppSlave2_Init`

#### `void AppCore_OnTickIsr(void *context)`

- 위치: `app/app_core.c:85`
- 역할: `AppCore_OnTickIsr`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`
- 로컬 변수: `AppCore *app`
- 접근 상태/필드: `app->lin_enabled`, `app->lin_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_OnBaseTick`

#### `void AppCore_TaskHeartbeat(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:98`
- 역할: `AppCore_TaskHeartbeat`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->heartbeat_count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:108`
- 역할: `AppCore_TaskLinFast`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->lin_enabled`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppSlave2_HandleLinOkToken`

#### `void AppCore_TaskLed(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:120`
- 역할: `AppCore_TaskLed`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->led2_enabled`, `app->slave2_led`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_Task`

#### `void AppCore_TaskAdc(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_core.c:130`
- 역할: `AppCore_TaskAdc`는 LIN sensor slave 전용 애플리케이션 오케스트레이션 구현부다. slave2에 필요한 ADC, LIN, LED task만 남겨, 센서 상태 생성과 latch 해제 흐름을 간단하게 추적할 수 있게 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `app->adc_enabled`
- 사용 전역/static: 없음
- 직접 호출 함수: `AppSlave2_TaskAdc`

### `app/app_core.h`

LIN sensor slave 전용 애플리케이션 context 헤더다. ADC, LED, LIN 상태 게시에 필요한 필드만 남겨, console과 CAN 없이 센서 노드 흐름만 바로 읽을 수 있게 한다.

노출 타입

- `AppCore` (struct)

### `app/app_core_internal.h`

LIN sensor slave AppCore 내부 helper 선언이다. 센서 노드는 console 없이도 상태 문자열을 내부에 유지하므로, 필요한 텍스트 갱신 함수만 좁게 노출한다.

### `app/app_slave2.c`

slave2 policy 구현부다. ADC 값을 샘플링하고 최신 LIN 상태 프레임을 게시하며, 해석된 센서 상태에 따라 로컬 LED 피드백도 갱신한다.

#### `InfraStatus AppSlave2_Init(AppCore *app)`

- 위치: `app/app_slave2.c:19`
- 역할: 센서 노드 역할을 초기화한다. slave2는 로컬 LED 피드백과 ADC 샘플링, LIN slave 상태 제공을 모두 맡기 때문에 세 모듈을 여기서 함께 시작한다.
- 파라미터: `AppCore *app`
- 로컬 변수: `LedConfig led_config`, `AdcConfig adc_config`, `LinConfig lin_config`
- 접근 상태/필드: `app->slave2_led`, `app->led2_enabled`, `app->adc_module`, `app->adc_enabled`, `app->lin_module`, `app->lin_enabled`
- 사용 전역/static: 없음
- 직접 호출 함수: `RuntimeIo_GetSlave2LedConfig`, `LedModule_Init`, `RuntimeIo_GetSlave2AdcConfig`, `AdcModule_Init`, `AppCore_SetAdcText`, `RuntimeIo_GetSlaveLinConfig`, `LinModule_Init`, `RuntimeIo_AttachLinModule`, `AppCore_SetLinLinkText`

#### `void AppSlave2_HandleLinOkToken(AppCore *app)`

- 위치: `app/app_slave2.c:65`
- 역할: LIN으로 전달된 승인 token을 소비한다. 이 token은 latch를 강제로 지우지 않고, zone이 더 이상 emergency가 아닐 때만 ADC 모듈에 clear를 요청한다.
- 파라미터: `AppCore *app`
- 로컬 변수: 없음
- 접근 상태/필드: `app->adc_enabled`, `app->lin_module`, `app->adc_module`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_ConsumeSlaveOkToken`, `AdcModule_ClearEmergencyLatch`, `AppCore_SetLinInputText`

#### `void AppSlave2_TaskAdc(AppCore *app, uint32_t now_ms)`

- 위치: `app/app_slave2.c:84`
- 역할: ADC 값을 샘플링하고 최신 해석 상태를 게시한다. 이 task는 센서 획득과 LIN 상태 cache 갱신, 콘솔 문자열과 로컬 LED 피드백을 함께 묶는 지점이다.
- 파라미터: `AppCore *app`, `uint32_t now_ms`
- 로컬 변수: `AdcSnapshot snapshot`, `LinStatusFrame status`
- 접근 상태/필드: `app->adc_enabled`, `app->adc_module`, `app->adc_text`, `app->lin_enabled`, `app->lin_module`, `app->led2_enabled`, `app->slave2_led`, `snapshot.raw_value`, `snapshot.zone`, `snapshot.emergency_latched`, `status.adc_value`, `status.zone`, `status.emergency_latched`, `status.valid`, `status.fresh`
- 사용 전역/static: 없음
- 직접 호출 함수: `AdcModule_Task`, `AdcModule_GetSnapshot`, `snprintf`, `u`, `AdcModule_ZoneText`, `LinModule_SetSlaveStatus`, `LedModule_SetPattern`

### `app/app_slave2.h`

LIN 센서 노드인 slave2의 policy 인터페이스다. slave2는 ADC 샘플링과 LIN 상태 제공, 그리고 승인 절차로만 해제되는 emergency latch를 담당한다.

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

LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다.

파일 전역 상태

- `g_runtime`

#### `static void Runtime_TaskHeartbeat(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:23`
- 역할: `Runtime_TaskHeartbeat`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskHeartbeat`

#### `static void Runtime_TaskLinFast(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:28`
- 역할: `Runtime_TaskLinFast`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskLinFast`

#### `static void Runtime_TaskLed(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:33`
- 역할: `Runtime_TaskLed`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskLed`

#### `static void Runtime_TaskAdc(void *context, uint32_t now_ms)`

- 위치: `runtime/runtime.c:38`
- 역할: `Runtime_TaskAdc`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `AppCore_TaskAdc`

#### `static void Runtime_BuildTaskTable(RuntimeContext *runtime)`

- 위치: `runtime/runtime.c:43`
- 역할: `Runtime_BuildTaskTable`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `RuntimeContext *runtime`
- 로컬 변수: 없음
- 접근 상태/필드: `runtime->tasks`, `runtime->app`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void Runtime_FaultLoop(void)`

- 위치: `runtime/runtime.c:70`
- 역할: `Runtime_FaultLoop`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus Runtime_Init(void)`

- 위치: `runtime/runtime.c:77`
- 역할: `Runtime_Init`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `InfraStatus status`, `uint32_t start_ms`
- 접근 상태/필드: `g_runtime.initialized`, `g_runtime.init_status`, `g_runtime.app`, `g_runtime.tasks`
- 사용 전역/static: `g_runtime`
- 직접 호출 함수: `RuntimeIo_BoardInit`, `RuntimeTick_Init`, `AppCore_Init`, `RuntimeTick_ClearHooks`, `RuntimeTick_RegisterHook`, `Runtime_BuildTaskTable`, `RuntimeTick_GetMs`, `RuntimeTask_ResetTable`, `INFRA_ARRAY_COUNT`

#### `void Runtime_Run(void)`

- 위치: `runtime/runtime.c:125`
- 역할: `Runtime_Run`는 LIN sensor slave 전용 runtime 구현부다. 센서 노드가 실제로 쓰는 task만 등록하여, CAN, UART, render 같은 다른 노드용 스케줄 항목을 제거한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: `g_runtime.initialized`, `g_runtime.init_status`, `g_runtime.tasks`
- 사용 전역/static: `g_runtime`
- 직접 호출 함수: `Runtime_FaultLoop`, `RuntimeTask_RunDue`, `INFRA_ARRAY_COUNT`, `RuntimeTick_GetMs`

### `runtime/runtime.h`

펌웨어 이미지의 최상위 runtime 인터페이스다. 애플리케이션은 이 함수들만 호출하면 시스템을 초기화하고, 무한 cooperative scheduler로 진입할 수 있다.

### `runtime/runtime_io.c`

LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다.

#### `InfraStatus RuntimeIo_BoardInit(void)`

- 위치: `runtime/runtime_io.c:28`
- 역할: `RuntimeIo_BoardInit`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `BoardHw_Init`, `BoardHw_EnableLinTransceiver`

#### `uint8_t RuntimeIo_GetLocalNodeId(void)`

- 위치: `runtime/runtime_io.c:42`
- 역할: `RuntimeIo_GetLocalNodeId`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus RuntimeIo_GetSlave2LedConfig(LedConfig *out_config)`

- 위치: `runtime/runtime_io.c:47`
- 역할: `RuntimeIo_GetSlave2LedConfig`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LedConfig *out_config`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `BoardHw_GetRgbLedConfig`

#### `InfraStatus RuntimeIo_GetSlave2AdcConfig(AdcConfig *out_config)`

- 위치: `runtime/runtime_io.c:52`
- 역할: `RuntimeIo_GetSlave2AdcConfig`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `AdcConfig *out_config`
- 로컬 변수: 없음
- 접근 상태/필드: `out_config->init_fn`, `out_config->sample_fn`, `out_config->hw_context`, `out_config->sample_period_ms`, `out_config->range_max`, `out_config->safe_max`, `out_config->warning_max`, `out_config->emergency_min`, `out_config->blocking_mode`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `AdcHw_IsSupported`

#### `void RuntimeIo_AttachLinModule(LinModule *module)`

- 위치: `runtime/runtime_io.c:72`
- 역할: `RuntimeIo_AttachLinModule`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LinHw_AttachModule`

#### `void RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid)`

- 위치: `runtime/runtime_io.c:78`
- 역할: `RuntimeIo_LinNotifyEvent`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinEventId event_id`, `uint8_t current_pid`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_OnEvent`

#### `InfraStatus RuntimeIo_GetSlaveLinConfig(LinConfig *out_config)`

- 위치: `runtime/runtime_io.c:88`
- 역할: `RuntimeIo_GetSlaveLinConfig`는 LIN sensor slave용 프로젝트 바인딩 구현부다. 역할별 상수와 주기 설정을 조립하고, 실제 하드웨어 접근은 shared driver 계층에 위임한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinConfig *out_config`
- 로컬 변수: 없음
- 접근 상태/필드: `out_config->role`, `out_config->pid_status`, `out_config->pid_ok`, `out_config->ok_token`, `out_config->status_frame_size`, `out_config->ok_frame_size`, `out_config->timeout_ticks`, `out_config->poll_period_ms`, `out_config->binding`, `binding.init_fn`, `binding.master_send_header_fn`, `binding.start_receive_fn`, `binding.start_send_fn`, `binding.goto_idle_fn`, `binding.set_timeout_fn`, `binding.service_tick_fn`, `binding.context`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `LinHw_Configure`, `LinHw_IsSupported`

### `runtime/runtime_io.h`

LIN sensor slave가 실제로 사용하는 프로젝트 바인딩 인터페이스다. slave2는 ADC, LED, LIN slave 설정만 필요하므로, console, CAN, 버튼 관련 API는 제거해 읽기 쉽게 만든다.

## 공통 모듈 참고

이 프로젝트가 실제로 사용하는 공통 `core/drivers/services/platform` 함수 상세는 [S32K 공통 모듈 참고서](./S32K_common_modules_reference.md)에 정리했다.

