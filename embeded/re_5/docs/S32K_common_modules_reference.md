# S32K 공통 모듈 참고서

세 프로젝트에서 재사용되는 `core`, `drivers`, `services`, `platform/s32k_sdk` 계층을 한곳에 모아 정리한 문서다. 동일 구현은 한 번만 설명하고, 어떤 프로젝트가 그 파일을 공유하는지도 같이 적었다.

## Core

### `core/infra_types.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 공통 인프라 타입과 시간 계산 도우미를 모아둔 헤더다. 대부분의 모듈이 같은 상태 코드 체계와 wrap-safe 시간 비교 함수를 재사용하기 위해 포함한다.

자료구조/열거형

#### `InfraStatus` (enum)

모든 모듈이 공유하는 공통 상태 코드다. 하나의 enum 체계를 쓰면 초기화, queue, 통신 오류를 계층 전체에서 같은 방식으로 비교할 수 있다.

- `INFRA_STATUS_OK = 0`
- `INFRA_STATUS_BUSY`
- `INFRA_STATUS_EMPTY`
- `INFRA_STATUS_FULL`
- `INFRA_STATUS_TIMEOUT`
- `INFRA_STATUS_INVALID_ARG`
- `INFRA_STATUS_NOT_READY`
- `INFRA_STATUS_IO_ERROR`
- `INFRA_STATUS_UNSUPPORTED`

### `core/infra_queue.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 고정 크기 ring queue 추상화를 제공하는 헤더다. 여러 모듈이 이 저장 구조를 재사용하여 동적 메모리 없이 메시지를 버퍼링할 수 있게 한다.

자료구조/열거형

#### `InfraQueue` (struct)

범용 ring queue 메타데이터 구조체다. 실제 저장 공간은 구조체 밖에 두어서, 모듈이 queue 소유권을 명확히 유지한 채 공통 구현을 재사용할 수 있다.

- `uint8_t *buffer`
- `uint16_t item_size`
- `uint16_t capacity`
- `uint16_t head`
- `uint16_t tail`
- `uint16_t count`

### `core/infra_queue.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다.

#### `static uint16_t InfraQueue_NextIndex(const InfraQueue *queue, uint16_t index)`

- 위치: `core/infra_queue.c:16`
- 역할: wrap-around를 고려해 ring index를 한 칸 전진시킨다. queue 포인터 계산을 한곳에 모아두면, push, pop, reset 로직을 작고 일관되게 유지할 수 있다.
- 파라미터: `const InfraQueue *queue`, `uint16_t index`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->capacity`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus InfraQueue_Init(InfraQueue *queue, void *storage, uint16_t item_size, uint16_t capacity)`

- 위치: `core/infra_queue.c:32`
- 역할: queue 메타데이터를 caller가 제공한 저장 공간에 연결한다. 버퍼를 먼저 비워서, 모듈이 오래된 데이터 없이 예측 가능한 빈 queue 상태에서 시작하게 한다.
- 파라미터: `InfraQueue *queue`, `void *storage`, `uint16_t item_size`, `uint16_t capacity`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->buffer`, `queue->item_size`, `queue->capacity`, `queue->head`, `queue->tail`, `queue->count`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `void InfraQueue_Reset(InfraQueue *queue)`

- 위치: `core/infra_queue.c:54`
- 역할: `InfraQueue_Reset`는 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `InfraQueue *queue`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->head`, `queue->tail`, `queue->count`, `queue->buffer`, `queue->item_size`, `queue->capacity`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `InfraStatus InfraQueue_Push(InfraQueue *queue, const void *item)`

- 위치: `core/infra_queue.c:78`
- 역할: ring queue의 tail 슬롯에 item 하나를 복사한다. queue가 가득 찬 경우를 명시적으로 알려서, producer가 재시도, drop, backpressure 전달 여부를 결정할 수 있게 한다.
- 파라미터: `InfraQueue *queue`, `const void *item`
- 로컬 변수: `uint8_t *slot`
- 접근 상태/필드: `queue->buffer`, `queue->count`, `queue->capacity`, `queue->tail`, `queue->item_size`
- 사용 전역/static: 없음
- 직접 호출 함수: `memcpy`, `InfraQueue_NextIndex`

#### `InfraStatus InfraQueue_Pop(InfraQueue *queue, void *out_item)`

- 위치: `core/infra_queue.c:106`
- 역할: ring queue의 head에서 item 하나를 꺼낸다. 꺼낸 슬롯을 0으로 지워서, 디버깅을 쉽게 하고 오래된 payload가 남지 않게 한다.
- 파라미터: `InfraQueue *queue`, `void *out_item`
- 로컬 변수: `uint8_t *slot`
- 접근 상태/필드: `queue->buffer`, `queue->count`, `queue->head`, `queue->item_size`
- 사용 전역/static: 없음
- 직접 호출 함수: `memcpy`, `memset`, `InfraQueue_NextIndex`

#### `InfraStatus InfraQueue_Peek(const InfraQueue *queue, void *out_item)`

- 위치: `core/infra_queue.c:130`
- 역할: `InfraQueue_Peek`는 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const InfraQueue *queue`, `void *out_item`
- 로컬 변수: `const uint8_t *slot`
- 접근 상태/필드: `queue->buffer`, `queue->count`, `queue->head`, `queue->item_size`
- 사용 전역/static: 없음
- 직접 호출 함수: `memcpy`

#### `uint16_t InfraQueue_GetCount(const InfraQueue *queue)`

- 위치: `core/infra_queue.c:150`
- 역할: `InfraQueue_GetCount`는 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const InfraQueue *queue`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint16_t InfraQueue_GetCapacity(const InfraQueue *queue)`

- 위치: `core/infra_queue.c:160`
- 역할: `InfraQueue_GetCapacity`는 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const InfraQueue *queue`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->capacity`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t InfraQueue_IsEmpty(const InfraQueue *queue)`

- 위치: `core/infra_queue.c:170`
- 역할: `InfraQueue_IsEmpty`는 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const InfraQueue *queue`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t InfraQueue_IsFull(const InfraQueue *queue)`

- 위치: `core/infra_queue.c:180`
- 역할: `InfraQueue_IsFull`는 고정 크기 ring queue 구현부다. queue는 caller가 제공한 저장 공간에 item 전체를 복사하며, 예측 가능한 단일 스레드 모듈 버퍼링을 목표로 한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const InfraQueue *queue`
- 로컬 변수: 없음
- 접근 상태/필드: `queue->count`, `queue->capacity`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `core/runtime_task.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 가벼운 주기 task 스케줄러 정의를 담는 헤더다. runtime 계층은 이 테이블 기반 인터페이스를 사용해 cooperative super-loop에서 AppCore task를 호출한다.

자료구조/열거형

#### `RuntimeTaskEntry` (struct)

cooperative task 테이블의 한 항목이다. runtime은 주기, 마지막 실행 시각, callback, context를 함께 저장해 스케줄링을 데이터 중심으로 유지한다.

- `const char *name`
- `uint32_t period_ms`
- `uint32_t last_run_ms`
- `RuntimeTaskFn task_fn`
- `void *context`

### `core/runtime_task.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: runtime super-loop를 위한 cooperative 스케줄러 구현부다. 이 함수들은 task 실행 시점만 판단하며, 애플리케이션 정책이나 하드웨어 상태는 직접 다루지 않는다.

#### `void RuntimeTask_ResetTable(RuntimeTaskEntry *table, uint32_t task_count, uint32_t start_ms)`

- 위치: `core/runtime_task.c:15`
- 역할: 모든 task 항목의 마지막 실행 시각을 같은 값으로 초기화한다. 스케줄러는 정의되지 않은 개별 값 대신, 하나의 공통 startup timestamp부터 주기를 계산하게 된다.
- 파라미터: `RuntimeTaskEntry *table`, `uint32_t task_count`, `uint32_t start_ms`
- 로컬 변수: `uint32_t index`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void RuntimeTask_RunDue(RuntimeTaskEntry *table, uint32_t task_count, uint32_t now_ms)`

- 위치: `core/runtime_task.c:37`
- 역할: task 테이블을 순회하며 주기가 지난 항목을 실행한다. runtime 계층 super-loop가 사용하는 cooperative scheduler의 핵심 함수다.
- 파라미터: `RuntimeTaskEntry *table`, `uint32_t task_count`, `uint32_t now_ms`
- 로컬 변수: `uint32_t index`, `RuntimeTaskEntry *task = &table[index]`
- 접근 상태/필드: `task->task_fn`, `task->period_ms`, `task->last_run_ms`, `task->context`
- 사용 전역/static: 없음
- 직접 호출 함수: `Infra_TimeIsDue`, `task_fn`

### `core/runtime_tick.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 시스템 tick 소스의 공개 인터페이스다. runtime과 통신 모듈은 이 계층을 통해 millisecond 시간과 간단한 ISR hook 등록 기능을 사용한다.

상수/매크로

- `RUNTIME_TICK_ISR_PERIOD_US` = `500U`

### `core/runtime_tick.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다.

파일 전역 상태

- `g_runtime_tick_ms`
- `g_runtime_tick_base_count`
- `g_runtime_tick_us_accumulator`
- `g_runtime_tick_hooks`

#### `InfraStatus RuntimeTick_Init(void)`

- 위치: `core/runtime_tick.c:27`
- 역할: `RuntimeTick_Init`는 LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `uint32_t index`
- 접근 상태/필드: 없음
- 사용 전역/static: `g_runtime_tick_ms`, `g_runtime_tick_base_count`, `g_runtime_tick_us_accumulator`, `g_runtime_tick_hooks`
- 직접 호출 함수: `TickHw_Init`

#### `uint32_t RuntimeTick_GetMs(void)`

- 위치: `core/runtime_tick.c:44`
- 역할: `RuntimeTick_GetMs`는 LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `g_runtime_tick_ms`
- 직접 호출 함수: 없음

#### `uint32_t RuntimeTick_GetBaseCount(void)`

- 위치: `core/runtime_tick.c:49`
- 역할: `RuntimeTick_GetBaseCount`는 LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `g_runtime_tick_base_count`
- 직접 호출 함수: 없음

#### `void RuntimeTick_ClearHooks(void)`

- 위치: `core/runtime_tick.c:54`
- 역할: `RuntimeTick_ClearHooks`는 LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `uint32_t index`
- 접근 상태/필드: 없음
- 사용 전역/static: `g_runtime_tick_hooks`
- 직접 호출 함수: 없음

#### `InfraStatus RuntimeTick_RegisterHook(RuntimeTickHook hook, void *context)`

- 위치: `core/runtime_tick.c:65`
- 역할: `RuntimeTick_RegisterHook`는 LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `RuntimeTickHook hook`, `void *context`
- 로컬 변수: `uint32_t index`
- 접근 상태/필드: 없음
- 사용 전역/static: `g_runtime_tick_hooks`
- 직접 호출 함수: 없음

#### `static void RuntimeTick_IrqHandler(void)`

- 위치: `core/runtime_tick.c:87`
- 역할: `RuntimeTick_IrqHandler`는 LPTMR 기반 시스템 tick 구현부다. 고정 주기의 base interrupt를 millisecond 시간으로 바꾸고, 등록된 소수의 hook에 같은 interrupt를 전달한다. 맥락에서 동작하는 helper/API다.
- 파라미터: 없음
- 로컬 변수: `uint32_t index`
- 접근 상태/필드: 없음
- 사용 전역/static: `g_runtime_tick_ms`, `g_runtime_tick_base_count`, `g_runtime_tick_us_accumulator`, `g_runtime_tick_hooks`
- 직접 호출 함수: `hook`, `TickHw_ClearCompareFlag`

## Drivers

### `drivers/board_hw.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

### `drivers/board_hw.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

#### `InfraStatus BoardHw_Init(void)`

- 위치: `drivers/board_hw.c:8`
- 역할: `BoardHw_Init`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_BoardInit`

#### `void BoardHw_EnableLinTransceiver(void)`

- 위치: `drivers/board_hw.c:13`
- 역할: `BoardHw_EnableLinTransceiver`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_BoardEnableLinTransceiver`

#### `InfraStatus BoardHw_GetRgbLedConfig(LedConfig *out_config)`

- 위치: `drivers/board_hw.c:18`
- 역할: `BoardHw_GetRgbLedConfig`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `LedConfig *out_config`
- 로컬 변수: 없음
- 접근 상태/필드: `out_config->gpio_port`, `out_config->red_pin`, `out_config->green_pin`, `out_config->active_on_level`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `IsoSdk_BoardGetRgbLedPort`, `IsoSdk_BoardGetRgbLedRedPin`, `IsoSdk_BoardGetRgbLedGreenPin`, `IsoSdk_BoardGetRgbLedActiveOnLevel`

#### `uint8_t BoardHw_ReadSlave1ButtonPressed(void)`

- 위치: `drivers/board_hw.c:33`
- 역할: `BoardHw_ReadSlave1ButtonPressed`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_BoardReadSlave1ButtonPressed`

### `drivers/tick_hw.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

### `drivers/tick_hw.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

#### `InfraStatus TickHw_Init(TickHwHandler handler)`

- 위치: `drivers/tick_hw.c:7`
- 역할: `TickHw_Init`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `TickHwHandler handler`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_TickInit`

#### `void TickHw_ClearCompareFlag(void)`

- 위치: `drivers/tick_hw.c:17`
- 역할: `TickHw_ClearCompareFlag`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_TickClearCompareFlag`

### `drivers/led_module.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 의미 기반 LED 제어 인터페이스다. 호출자는 raw GPIO polarity나 pin 순서를 몰라도, 빨강 고정이나 초록 점멸 같은 패턴만 요청하면 된다.

자료구조/열거형

#### `LedPattern` (enum)

- `LED_PATTERN_OFF = 0`
- `LED_PATTERN_GREEN_SOLID`
- `LED_PATTERN_RED_SOLID`
- `LED_PATTERN_YELLOW_SOLID`
- `LED_PATTERN_RED_BLINK`
- `LED_PATTERN_GREEN_BLINK`

#### `LedConfig` (struct)

LED 모듈에 필요한 보드 배선 정보 구조체다. Runtime IO가 이 값을 채워 넣어, LED 로직이 concrete GPIO 정의와 분리된 의미 기반 코드로 남게 한다.

- `void *gpio_port`
- `uint32_t red_pin`
- `uint32_t green_pin`
- `uint8_t active_on_level`

#### `LedModule` (struct)

하나의 로컬 출력 장치에 대한 전체 LED 모듈 상태다. 현재 pattern과 blink phase, finite acknowledgement 시퀀스 진행 여부를 함께 기억한다.

- `uint8_t initialized`
- `LedConfig config`
- `LedPattern pattern`
- `uint8_t output_phase_on`
- `uint8_t finite_blink_enabled`
- `uint8_t blink_toggles_remaining`

### `drivers/led_module.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다.

#### `static void LedModule_WritePin(const LedModule *module, uint32_t pin, uint8_t on)`

- 위치: `drivers/led_module.c:13`
- 역할: `LedModule_WritePin`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const LedModule *module`, `uint32_t pin`, `uint8_t on`
- 로컬 변수: `uint8_t level`
- 접근 상태/필드: `module->config`, `config.gpio_port`, `config.active_on_level`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_GpioWritePin`

#### `static void LedModule_ApplyOutputs(const LedModule *module, uint8_t red_on, uint8_t green_on)`

- 위치: `drivers/led_module.c:28`
- 역할: `LedModule_ApplyOutputs`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const LedModule *module`, `uint8_t red_on`, `uint8_t green_on`
- 로컬 변수: 없음
- 접근 상태/필드: `module->config`, `config.red_pin`, `config.green_pin`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_WritePin`

#### `static void LedModule_ApplyPattern(const LedModule *module, LedPattern pattern, uint8_t phase_on)`

- 위치: `drivers/led_module.c:39`
- 역할: `LedModule_ApplyPattern`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const LedModule *module`, `LedPattern pattern`, `uint8_t phase_on`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_ApplyOutputs`

#### `InfraStatus LedModule_Init(LedModule *module, const LedConfig *config)`

- 위치: `drivers/led_module.c:70`
- 역할: `LedModule_Init`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LedModule *module`, `const LedConfig *config`
- 로컬 변수: 없음
- 접근 상태/필드: `config->gpio_port`, `module->config`, `module->pattern`, `module->output_phase_on`, `module->initialized`, `config.gpio_port`, `config.red_pin`, `config.green_pin`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `IsoSdk_GpioSetPinsDirectionMask`, `LedModule_ApplyPattern`

#### `void LedModule_SetPattern(LedModule *module, LedPattern pattern)`

- 위치: `drivers/led_module.c:92`
- 역할: `LedModule_SetPattern`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LedModule *module`, `LedPattern pattern`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->pattern`, `module->finite_blink_enabled`, `module->blink_toggles_remaining`, `module->output_phase_on`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_ApplyPattern`

#### `void LedModule_StartGreenAckBlink(LedModule *module, uint8_t toggle_count)`

- 위치: `drivers/led_module.c:108`
- 역할: `LedModule_StartGreenAckBlink`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LedModule *module`, `uint8_t toggle_count`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->pattern`, `module->output_phase_on`, `module->finite_blink_enabled`, `module->blink_toggles_remaining`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_ApplyPattern`

#### `void LedModule_Task(LedModule *module, uint32_t now_ms)`

- 위치: `drivers/led_module.c:123`
- 역할: `LedModule_Task`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LedModule *module`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->pattern`, `module->output_phase_on`, `module->finite_blink_enabled`, `module->blink_toggles_remaining`
- 사용 전역/static: 없음
- 직접 호출 함수: `LedModule_ApplyPattern`

#### `LedPattern LedModule_GetPattern(const LedModule *module)`

- 위치: `drivers/led_module.c:160`
- 역할: `LedModule_GetPattern`는 로컬 보드 배선에 맞춘 LED 패턴 실행 구현부다. 의미 기반 패턴을 GPIO write로 바꾸고, 주기 LED task 동안 blink 시퀀스를 진행시킨다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const LedModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `module->pattern`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `drivers/can_hw.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: 저수준 FlexCAN transport 인터페이스다. 상위 CAN 계층은 mailbox 설정과 generated driver 호출 대신, 작은 queue 중심 하드웨어 API만 보게 된다.

자료구조/열거형

#### `CanHw` (struct)

FlexCAN mailbox 기반 raw CAN 하드웨어 context다. transport 계층은 이 상태를 사용해 수신 frame을 버퍼링하고, generated driver의 TX 완료 여부를 관찰한다.

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint8_t instance`
- `uint8_t tx_mb_index`
- `uint8_t rx_mb_index`
- `volatile uint8_t tx_busy`
- `volatile uint8_t last_error`
- `CanFrame rx_queue[CAN_HW_RX_QUEUE_SIZE]`
- `volatile uint8_t rx_head`
- `volatile uint8_t rx_tail`
- `volatile uint8_t rx_count`
- `volatile uint32_t tx_ok_count`
- `volatile uint32_t tx_error_count`
- `volatile uint32_t rx_ok_count`
- `volatile uint32_t rx_error_count`
- `volatile uint32_t rx_drop_count`

### `drivers/can_hw.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다.

#### `static void CanHw_ClearFrame(CanFrame *frame)`

- 위치: `drivers/can_hw.c:17`
- 역할: `CanHw_ClearFrame`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanFrame *frame`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static uint8_t CanHw_NextIndex(uint8_t index, uint8_t capacity)`

- 위치: `drivers/can_hw.c:25`
- 역할: `CanHw_NextIndex`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint8_t index`, `uint8_t capacity`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanHw_RxQueuePush(CanHw *hw, const CanFrame *frame)`

- 위치: `drivers/can_hw.c:36`
- 역할: `CanHw_RxQueuePush`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`, `const CanFrame *frame`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->rx_count`, `hw->rx_queue`, `hw->rx_tail`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_NextIndex`

#### `static uint8_t CanHw_CopyIsoSdkRxToFrame(uint32_t now_ms, CanFrame *out_frame)`

- 위치: `drivers/can_hw.c:54`
- 역할: `CanHw_CopyIsoSdkRxToFrame`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint32_t now_ms`, `CanFrame *out_frame`
- 로컬 변수: `uint8_t dlc`
- 접근 상태/필드: `out_frame->id`, `out_frame->is_extended_id`, `out_frame->is_remote_frame`, `out_frame->data`, `out_frame->dlc`, `out_frame->timestamp_ms`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_ClearFrame`, `IsoSdk_CanReadRxFrame`

#### `static uint8_t CanHw_StartReceive(CanHw *hw)`

- 위치: `drivers/can_hw.c:81`
- 역할: `CanHw_StartReceive`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->instance`, `hw->rx_mb_index`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanStartReceive`

#### `static void CanHw_OnRxComplete(CanHw *hw)`

- 위치: `drivers/can_hw.c:91`
- 역할: `CanHw_OnRxComplete`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`
- 로컬 변수: `CanFrame frame`
- 접근 상태/필드: `hw->initialized`, `hw->rx_ok_count`, `hw->rx_drop_count`, `hw->last_error`, `hw->rx_error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_CopyIsoSdkRxToFrame`, `CanHw_RxQueuePush`, `CanHw_StartReceive`

#### `static void CanHw_OnIsoSdkEvent(void *context, uint8_t event_id, uint8_t mb_index)`

- 위치: `drivers/can_hw.c:125`
- 역할: `CanHw_OnIsoSdkEvent`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint8_t event_id`, `uint8_t mb_index`
- 로컬 변수: `CanHw *hw`
- 접근 상태/필드: `hw->initialized`, `hw->rx_mb_index`, `hw->last_error`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_OnRxComplete`

#### `uint8_t CanHw_InitDefault(CanHw *hw, uint8_t local_node_id)`

- 위치: `drivers/can_hw.c:147`
- 역할: `CanHw_InitDefault`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`, `uint8_t local_node_id`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->local_node_id`, `hw->tx_mb_index`, `hw->rx_mb_index`, `hw->last_error`, `hw->instance`, `hw->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `IsoSdk_CanIsSupported`, `IsoSdk_CanGetDefaultInstance`, `IsoSdk_CanInitController`, `IsoSdk_CanInstallEventCallback`, `IsoSdk_CanInitTxMailbox`, `IsoSdk_CanConfigRxAcceptAll`, `IsoSdk_CanInitRxMailbox`, `CanHw_StartReceive`

#### `void CanHw_Task(CanHw *hw, uint32_t now_ms)`

- 위치: `drivers/can_hw.c:205`
- 역할: `CanHw_Task`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`, `uint32_t now_ms`
- 로컬 변수: `IsoSdkCanTransferState transfer_state`
- 접근 상태/필드: `hw->initialized`, `hw->tx_busy`, `hw->instance`, `hw->tx_mb_index`, `hw->tx_ok_count`, `hw->tx_error_count`, `hw->last_error`, `hw->rx_mb_index`, `hw->rx_error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanGetTransferState`, `CanHw_StartReceive`

#### `uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame)`

- 위치: `drivers/can_hw.c:244`
- 역할: `CanHw_StartTx`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`, `const CanFrame *frame`
- 로컬 변수: `uint8_t dlc`
- 접근 상태/필드: `hw->initialized`, `hw->last_error`, `hw->tx_busy`, `frame->dlc`, `hw->instance`, `hw->tx_mb_index`, `frame->id`, `frame->data`, `frame->is_extended_id`, `frame->is_remote_frame`, `hw->tx_error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanSend`

#### `uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *out_frame)`

- 위치: `drivers/can_hw.c:286`
- 역할: `CanHw_TryPopRx`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanHw *hw`, `CanFrame *out_frame`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->initialized`, `hw->rx_count`, `hw->rx_queue`, `hw->rx_head`
- 사용 전역/static: 없음
- 직접 호출 함수: `INT_SYS_DisableIRQGlobal`, `INT_SYS_EnableIRQGlobal`, `CanHw_ClearFrame`, `CanHw_NextIndex`

#### `uint8_t CanHw_IsReady(const CanHw *hw)`

- 위치: `drivers/can_hw.c:308`
- 역할: `CanHw_IsReady`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanHw *hw`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t CanHw_IsTxBusy(const CanHw *hw)`

- 위치: `drivers/can_hw.c:313`
- 역할: `CanHw_IsTxBusy`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanHw *hw`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->tx_busy`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t CanHw_GetLastError(const CanHw *hw)`

- 위치: `drivers/can_hw.c:318`
- 역할: `CanHw_GetLastError`는 FlexCAN mailbox 처리와 raw frame 버퍼링 구현부다. IsoSdk 계층이 실제 generated driver 호출을 맡고, 위쪽에는 더 단순한 push/pop transport 표면만 노출한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanHw *hw`
- 로컬 변수: 없음
- 접근 상태/필드: `hw->last_error`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `drivers/lin_hw.h`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`, `S32K_Lin_slave`

### `drivers/lin_hw.c`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`, `S32K_Lin_slave`

파일 전역 상태

- `s_lin_hw`

#### `static LinEventId LinHw_EventFromIsoSdk(uint8_t event_id)`

- 위치: `drivers/lin_hw.c:18`
- 역할: `LinHw_EventFromIsoSdk`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint8_t event_id`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void LinHw_OnIsoSdkEvent(void *context, uint8_t event_id, uint8_t current_pid)`

- 위치: `drivers/lin_hw.c:39`
- 역할: `LinHw_OnIsoSdkEvent`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`, `uint8_t event_id`, `uint8_t current_pid`
- 로컬 변수: `LinEventId lin_event_id`
- 접근 상태/필드: `s_lin_hw.module`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `LinHw_EventFromIsoSdk`, `LinModule_OnEvent`

#### `uint8_t LinHw_IsSupported(void)`

- 위치: `drivers/lin_hw.c:51`
- 역할: `LinHw_IsSupported`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_LinIsSupported`

#### `void LinHw_Configure(uint8_t role, uint16_t timeout_ticks)`

- 위치: `drivers/lin_hw.c:56`
- 역할: `LinHw_Configure`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint8_t role`, `uint16_t timeout_ticks`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.sdk_context`, `s_lin_hw.role`, `s_lin_hw.timeout_ticks`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `memset`

#### `void LinHw_AttachModule(LinModule *module)`

- 위치: `drivers/lin_hw.c:64`
- 역할: `LinHw_AttachModule`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.module`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: 없음

#### `InfraStatus LinHw_Init(void *context)`

- 위치: `drivers/lin_hw.c:69`
- 역할: `LinHw_Init`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`
- 로컬 변수: `uint8_t iso_role`
- 접근 상태/필드: `s_lin_hw.role`, `s_lin_hw.sdk_context`, `s_lin_hw.timeout_ticks`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinIsSupported`, `IsoSdk_LinInit`

#### `InfraStatus LinHw_MasterSendHeader(void *context, uint8_t pid)`

- 위치: `drivers/lin_hw.c:87`
- 역할: `LinHw_MasterSendHeader`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`, `uint8_t pid`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.sdk_context`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinMasterSendHeader`

#### `InfraStatus LinHw_StartReceive(void *context, uint8_t *buffer, uint8_t length)`

- 위치: `drivers/lin_hw.c:93`
- 역할: `LinHw_StartReceive`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`, `uint8_t *buffer`, `uint8_t length`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.sdk_context`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinStartReceive`

#### `InfraStatus LinHw_StartSend(void *context, const uint8_t *buffer, uint8_t length)`

- 위치: `drivers/lin_hw.c:99`
- 역할: `LinHw_StartSend`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`, `const uint8_t *buffer`, `uint8_t length`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.sdk_context`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinStartSend`

#### `void LinHw_GotoIdle(void *context)`

- 위치: `drivers/lin_hw.c:105`
- 역할: `LinHw_GotoIdle`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.sdk_context`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinGotoIdle`

#### `void LinHw_SetTimeout(void *context, uint16_t timeout_ticks)`

- 위치: `drivers/lin_hw.c:111`
- 역할: `LinHw_SetTimeout`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`, `uint16_t timeout_ticks`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.timeout_ticks`, `s_lin_hw.sdk_context`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinSetTimeout`

#### `void LinHw_ServiceTick(void *context)`

- 위치: `drivers/lin_hw.c:118`
- 역할: `LinHw_ServiceTick`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`
- 로컬 변수: 없음
- 접근 상태/필드: `s_lin_hw.sdk_context`
- 사용 전역/static: `s_lin_hw`
- 직접 호출 함수: `IsoSdk_LinServiceTick`

### `drivers/uart_hw.h`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`
- 모듈 역할: 저수준 UART 하드웨어 바인딩 인터페이스다. service 계층은 이 호출들을 사용해 전송을 시작하고, IsoSdk 계층 위에서 RX/TX 상태를 확인한다.

자료구조/열거형

#### `UartHwStatus` (enum)

- `UART_HW_STATUS_OK = 0`
- `UART_HW_STATUS_BUSY`
- `UART_HW_STATUS_ERROR`

### `drivers/uart_hw.c`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`
- 모듈 역할: IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다.

#### `static UartHwStatus UartHw_StatusFromBool(uint8_t ok)`

- 위치: `drivers/uart_hw.c:12`
- 역할: `UartHw_StatusFromBool`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint8_t ok`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint16_t UartHw_NextPendingIndex(uint16_t index)`

- 위치: `drivers/uart_hw.c:17`
- 역할: `UartHw_NextPendingIndex`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint16_t index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t UartHw_IsPendingFull(const UartRxPendingRing *ring)`

- 위치: `drivers/uart_hw.c:28`
- 역할: `UartHw_IsPendingFull`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartRxPendingRing *ring`
- 로컬 변수: `uint16_t next_tail`
- 접근 상태/필드: `ring->tail`, `ring->head`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartHw_NextPendingIndex`

#### `static void UartHw_PushPendingByte(UartService *service, uint8_t rx_byte)`

- 위치: `drivers/uart_hw.c:41`
- 역할: `UartHw_PushPendingByte`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`, `uint8_t rx_byte`
- 로컬 변수: `UartRxPendingRing *ring`
- 접근 상태/필드: `service->rx_pending`, `ring->overflow`, `ring->overflow_count`, `ring->buffer`, `ring->tail`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartHw_IsPendingFull`, `UartHw_NextPendingIndex`

#### `static UartHwStatus UartHw_StartReceiveByte(UartService *service)`

- 위치: `drivers/uart_hw.c:62`
- 역할: `UartHw_StartReceiveByte`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->instance`, `service->rx_byte`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartHw_StatusFromBool`, `IsoSdk_UartStartReceiveByte`

#### `static UartHwStatus UartHw_ContinueReceiveByte(UartService *service)`

- 위치: `drivers/uart_hw.c:73`
- 역할: `UartHw_ContinueReceiveByte`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->instance`, `service->rx_byte`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartHw_StatusFromBool`, `IsoSdk_UartContinueReceiveByte`

#### `static void UartHw_OnIsoSdkEvent(void *context, uint8_t event_id)`

- 위치: `drivers/uart_hw.c:84`
- 역할: `UartHw_OnIsoSdkEvent`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `void *context`, `uint8_t event_id`
- 로컬 변수: `UartService *service`, `UartHwStatus hw_status`
- 접근 상태/필드: `service->rx_byte`, `service->error_flag`, `service->error_code`, `service->error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartHw_PushPendingByte`, `UartHw_ContinueReceiveByte`

#### `UartHwStatus UartHw_InitDefault(UartService *service)`

- 위치: `drivers/uart_hw.c:122`
- 역할: `UartHw_InitDefault`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->instance`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_UartIsSupported`, `IsoSdk_UartGetDefaultInstance`, `IsoSdk_UartInit`, `UartHw_StartReceiveByte`

#### `UartHwStatus UartHw_StartTransmit(UartService *service, const uint8_t *data, uint16_t length)`

- 위치: `drivers/uart_hw.c:143`
- 역할: `UartHw_StartTransmit`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`, `const uint8_t *data`, `uint16_t length`
- 로컬 변수: 없음
- 접근 상태/필드: `service->instance`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartHw_StatusFromBool`, `IsoSdk_UartStartTransmit`

#### `UartHwStatus UartHw_GetTransmitStatus(UartService *service, uint32_t *bytes_remaining)`

- 위치: `drivers/uart_hw.c:153`
- 역할: `UartHw_GetTransmitStatus`는 IsoSdk UART 계층 위에 만든 하드웨어 바인딩 구현부다. 바이트 단위 RX callback 처리는 여기서 맡고, line 조립과 정책 판단은 UartService에 넘긴다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`, `uint32_t *bytes_remaining`
- 로컬 변수: `IsoSdkUartTxState tx_state`
- 접근 상태/필드: `service->instance`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_UartGetTransmitState`

### `drivers/adc_hw.h`

- 대표 프로젝트: `S32K_Lin_slave`
- 사용 프로젝트: `S32K_Lin_slave`

### `drivers/adc_hw.c`

- 대표 프로젝트: `S32K_Lin_slave`
- 사용 프로젝트: `S32K_Lin_slave`

파일 전역 상태

- `s_adc_hw_context`

#### `uint8_t AdcHw_IsSupported(void)`

- 위치: `drivers/adc_hw.c:10`
- 역할: `AdcHw_IsSupported`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_AdcIsSupported`

#### `InfraStatus AdcHw_Init(void *context)`

- 위치: `drivers/adc_hw.c:15`
- 역할: `AdcHw_Init`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `s_adc_hw_context`
- 직접 호출 함수: `memset`, `IsoSdk_AdcInit`

#### `InfraStatus AdcHw_Sample(void *context, uint16_t *out_raw_value)`

- 위치: `drivers/adc_hw.c:22`
- 역할: `AdcHw_Sample`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *context`, `uint16_t *out_raw_value`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `s_adc_hw_context`
- 직접 호출 함수: `IsoSdk_AdcSample`

## Services

### `services/can_types.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: CAN 프로토콜 공통 상수와 자료구조를 모아둔 헤더다. node ID와 payload 형식, result 타입, 그리고 모든 CAN 하위 모듈이 재사용하는 메시지 컨테이너를 정의한다.

상수/매크로

- `CAN_NODE_ID_INVALID` = `0U`
- `CAN_NODE_ID_MIN` = `1U`
- `CAN_NODE_ID_MAX` = `254U`
- `CAN_NODE_ID_BROADCAST` = `255U`
- `CAN_FRAME_DATA_SIZE` = `16U`
- `CAN_TEXT_MAX_LEN` = `11U`
- `CAN_TEXT_FRAME_HEADER_SIZE` = `5U`
- `CAN_HW_TX_MB_INDEX` = `1U`
- `CAN_HW_RX_MB_INDEX` = `0U`
- `CAN_HW_RX_QUEUE_SIZE` = `8U`
- `CAN_TRANSPORT_TX_QUEUE_SIZE` = `8U`
- `CAN_TRANSPORT_RX_QUEUE_SIZE` = `8U`
- `CAN_SERVICE_QUEUE_SIZE` = `8U`
- `CAN_SERVICE_PENDING_SIZE` = `4U`
- `CAN_MODULE_REQUEST_QUEUE_SIZE` = `8U`
- `CAN_PROTO_VERSION_V1` = `1U`
- `CAN_PROTO_STDID_COMMAND` = `0x120U`
- `CAN_PROTO_STDID_RESPONSE` = `0x121U`
- `CAN_PROTO_STDID_EVENT` = `0x122U`
- `CAN_PROTO_STDID_TEXT` = `0x123U`
- `CAN_MSG_FLAG_NEED_RESPONSE` = `(1U << 0)`
- `CAN_MSG_FLAG_ERROR` = `(1U << 1)`

자료구조/열거형

#### `CanMessageType` (enum)

- `CAN_MSG_COMMAND = 0`
- `CAN_MSG_RESPONSE`
- `CAN_MSG_EVENT`
- `CAN_MSG_TEXT`

#### `CanPayloadKind` (enum)

- `CAN_PAYLOAD_NONE = 0`
- `CAN_PAYLOAD_CTRL_CMD`
- `CAN_PAYLOAD_CTRL_RESULT`
- `CAN_PAYLOAD_EVENT_DATA`
- `CAN_PAYLOAD_TEXT_DATA`

#### `CanCommandCode` (enum)

- `CAN_CMD_NONE = 0`
- `CAN_CMD_OPEN`
- `CAN_CMD_CLOSE`
- `CAN_CMD_OFF`
- `CAN_CMD_TEST`
- `CAN_CMD_OK`
- `CAN_CMD_EMERGENCY`
- `CAN_CMD_STATUS_REQ`

#### `CanResultCode` (enum)

- `CAN_RES_NONE = 0`
- `CAN_RES_OK`
- `CAN_RES_FAIL`
- `CAN_RES_INVALID_TARGET`
- `CAN_RES_BUSY`
- `CAN_RES_TIMEOUT`
- `CAN_RES_NOT_SUPPORTED`

#### `CanEventCode` (enum)

- `CAN_EVENT_NONE = 0`
- `CAN_EVENT_BOOT`
- `CAN_EVENT_ONLINE`
- `CAN_EVENT_OFFLINE`
- `CAN_EVENT_HEARTBEAT`
- `CAN_EVENT_WARNING`
- `CAN_EVENT_ERROR`

#### `CanTextType` (enum)

- `CAN_TEXT_NONE = 0`
- `CAN_TEXT_USER`
- `CAN_TEXT_LOG`
- `CAN_TEXT_DEBUG`
- `CAN_TEXT_RESPONSE`
- `CAN_TEXT_EVENT`

#### `CanProtoDecodeStatus` (enum)

- `CAN_PROTO_DECODE_OK = 0`
- `CAN_PROTO_DECODE_IGNORED`
- `CAN_PROTO_DECODE_INVALID`
- `CAN_PROTO_DECODE_UNSUPPORTED`

#### `CanHwError` (enum)

- `CAN_HW_ERROR_NONE = 0`
- `CAN_HW_ERROR_NOT_READY`
- `CAN_HW_ERROR_INIT_FAIL`
- `CAN_HW_ERROR_RX_CONFIG_FAIL`
- `CAN_HW_ERROR_TX_CONFIG_FAIL`
- `CAN_HW_ERROR_TX_START_FAIL`
- `CAN_HW_ERROR_TX_STATUS_FAIL`
- `CAN_HW_ERROR_RX_STATUS_FAIL`
- `CAN_HW_ERROR_RX_QUEUE_FULL`
- `CAN_HW_ERROR_RX_RESTART_FAIL`

#### `CanTransportError` (enum)

- `CAN_TRANSPORT_ERROR_NONE = 0`
- `CAN_TRANSPORT_ERROR_NOT_READY`
- `CAN_TRANSPORT_ERROR_TX_QUEUE_FULL`
- `CAN_TRANSPORT_ERROR_RX_QUEUE_FULL`
- `CAN_TRANSPORT_ERROR_HW_TX_FAIL`
- `CAN_TRANSPORT_ERROR_HW_RX_FAIL`

#### `CanServiceError` (enum)

- `CAN_SERVICE_ERROR_NONE = 0`
- `CAN_SERVICE_ERROR_NOT_READY`
- `CAN_SERVICE_ERROR_INVALID_TARGET`
- `CAN_SERVICE_ERROR_PENDING_FULL`
- `CAN_SERVICE_ERROR_TX_QUEUE_FULL`
- `CAN_SERVICE_ERROR_RX_QUEUE_FULL`
- `CAN_SERVICE_ERROR_RESULT_QUEUE_FULL`
- `CAN_SERVICE_ERROR_PROTOCOL_ERROR`
- `CAN_SERVICE_ERROR_UNSUPPORTED`

#### `CanServiceResultKind` (enum)

- `CAN_SERVICE_RESULT_NONE = 0`
- `CAN_SERVICE_RESULT_RESPONSE`
- `CAN_SERVICE_RESULT_TIMEOUT`
- `CAN_SERVICE_RESULT_SEND_FAIL`

#### `CanFrame` (struct)

transport 계층이 사용하는 raw CAN frame 컨테이너다. 상위 계층은 보통 이것을 직접 다루지 않고, protocol 계층이 제공하는 논리적 CanMessage 객체를 사용한다.

- `uint32_t id`
- `uint8_t dlc`
- `uint8_t data[CAN_FRAME_DATA_SIZE]`
- `uint8_t is_extended_id`
- `uint8_t is_remote_frame`
- `uint32_t timestamp_ms`

#### `CanMessage` (struct)

app과 service 계층이 이해하는 논리적 CAN 메시지다. protocol encoder는 이 더 풍부한 구조를 frame으로 바꾸고, 수신 경로에서 다시 복원한다.

- `uint8_t version`
- `uint8_t message_type`
- `uint8_t source_node_id`
- `uint8_t target_node_id`
- `uint8_t request_id`
- `uint8_t flags`
- `uint8_t payload_kind`
- `uint8_t payload_length`
- `uint8_t payload[CAN_FRAME_DATA_SIZE]`
- `uint8_t text_type`
- `uint8_t text_length`
- `char text[CAN_TEXT_MAX_LEN + 1U]`

#### `CanServiceResult` (struct)

추적 중인 CAN 요청이 끝났을 때 만들어지는 결과 기록이다. AppCore는 이 결과를 읽어, response와 timeout, 원격 동작 실패에 대한 operator 피드백을 만든다.

- `uint8_t kind`
- `uint8_t request_id`
- `uint8_t source_node_id`
- `uint8_t target_node_id`
- `uint8_t command_code`
- `uint8_t result_code`
- `uint8_t detail_code`

### `services/can_proto.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: CAN 메시지 encode/decode 인터페이스다. protocol 계층은 논리 메시지를 raw frame으로 바꿔, 나머지 시스템이 typed payload와 node ID 기준으로 동작하게 한다.

자료구조/열거형

#### `CanProtoConfig` (struct)

CAN protocol codec의 정적 설정 구조체다. codec은 로컬 node identity를 유지하여, 이후 protocol 변화가 생겨도 app 세부사항을 노출하지 않게 한다.

- `uint8_t local_node_id`

#### `CanEncodedFrameList` (struct)

- `CanFrame frames[1]`
- `uint8_t count`

#### `CanProto` (struct)

CAN protocol encoder/decoder의 런타임 상태다. 설정 정보뿐 아니라 decode 통계도 함께 기록하여, 잘못된 frame이나 unsupported frame을 진단할 때 도움이 된다.

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint32_t decode_ok_count`
- `uint32_t decode_ignored_count`
- `uint32_t decode_invalid_count`
- `uint32_t decode_unsupported_count`

### `services/can_proto.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: CAN 프로토콜 encoder/decoder 구현부다. 내부 메시지와 실제 CAN frame 사이를 변환하기 전에, node ID와 payload layout을 먼저 검증한다.

#### `static void CanProto_ClearFrame(CanFrame *frame)`

- 위치: `services/can_proto.c:16`
- 역할: raw CAN frame 컨테이너 하나를 0으로 초기화한다. 이런 helper를 두면 encode/decode 경로에서, 필드를 채우기 전에 깨끗한 목적지 객체를 쉽게 준비할 수 있다.
- 파라미터: `CanFrame *frame`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static void CanProto_ClearMessage(CanMessage *message)`

- 위치: `services/can_proto.c:24`
- 역할: `CanProto_ClearMessage`는 CAN 프로토콜 encoder/decoder 구현부다. 내부 메시지와 실제 CAN frame 사이를 변환하기 전에, node ID와 payload layout을 먼저 검증한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanMessage *message`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static uint8_t CanProto_IsValidNodeId(uint8_t node_id)`

- 위치: `services/can_proto.c:32`
- 역할: `CanProto_IsValidNodeId`는 CAN 프로토콜 encoder/decoder 구현부다. 내부 메시지와 실제 CAN frame 사이를 변환하기 전에, node ID와 payload layout을 먼저 검증한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint8_t node_id`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanProto_IsPrintableAscii(const char *text, uint8_t length)`

- 위치: `services/can_proto.c:42`
- 역할: `CanProto_IsPrintableAscii`는 CAN 프로토콜 encoder/decoder 구현부다. 내부 메시지와 실제 CAN frame 사이를 변환하기 전에, node ID와 payload layout을 먼저 검증한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const char *text`, `uint8_t length`
- 로컬 변수: `uint8_t index`, `unsigned char ch`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint32_t CanProto_MessageTypeToId(uint8_t message_type)`

- 위치: `services/can_proto.c:64`
- 역할: `CanProto_MessageTypeToId`는 CAN 프로토콜 encoder/decoder 구현부다. 내부 메시지와 실제 CAN frame 사이를 변환하기 전에, node ID와 payload layout을 먼저 검증한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint8_t message_type`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanProto_IdToMessageType(uint32_t id, uint8_t *out_message_type)`

- 위치: `services/can_proto.c:85`
- 역할: `CanProto_IdToMessageType`는 CAN 프로토콜 encoder/decoder 구현부다. 내부 메시지와 실제 CAN frame 사이를 변환하기 전에, node ID와 payload layout을 먼저 검증한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint32_t id`, `uint8_t *out_message_type`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t CanProto_Init(CanProto *proto, const CanProtoConfig *config)`

- 위치: `services/can_proto.c:120`
- 역할: 로컬 node identity로 protocol codec을 초기화한다. codec은 caller 설정을 복사하는 동시에, 작은 decode 통계 집합도 함께 추적한다.
- 파라미터: `CanProto *proto`, `const CanProtoConfig *config`
- 로컬 변수: 없음
- 접근 상태/필드: `proto->local_node_id`, `config->local_node_id`, `proto->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `uint8_t CanProto_EncodeMessage(CanProto *proto, const CanMessage *message, CanEncodedFrameList *out_list)`

- 위치: `services/can_proto.c:138`
- 역할: 논리 메시지 하나를 하나 이상의 CAN frame으로 encode한다. version과 node ID, payload 길이, text 제약을 여기서 검증한 뒤, transport 계층에 frame을 넘긴다.
- 파라미터: `CanProto *proto`, `const CanMessage *message`, `CanEncodedFrameList *out_list`
- 로컬 변수: `CanFrame *frame`, `uint8_t text_length`, `uint32_t frame_id`
- 접근 상태/필드: `proto->initialized`, `message->version`, `message->source_node_id`, `message->target_node_id`, `message->message_type`, `out_list->frames`, `frame->id`, `message->text_length`, `message->text`, `frame->dlc`, `frame->data`, `message->text_type`, `out_list->count`, `message->request_id`, `message->payload_length`, `message->payload`, `message->flags`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanProto_IsValidNodeId`, `CanProto_MessageTypeToId`, `CanProto_ClearFrame`, `CanProto_IsPrintableAscii`, `memcpy`

#### `CanProtoDecodeStatus CanProto_DecodeFrame(CanProto *proto, const CanFrame *frame, CanMessage *out_message)`

- 위치: `services/can_proto.c:217`
- 역할: 수신한 raw frame 하나를 논리 메시지로 decode한다. unsupported ID와 malformed payload는 여기서 걸러서, 상위 계층이 검증된 protocol 데이터만 처리하게 만든다.
- 파라미터: `CanProto *proto`, `const CanFrame *frame`, `CanMessage *out_message`
- 로컬 변수: `uint8_t message_type`, `uint8_t text_length`
- 접근 상태/필드: `proto->initialized`, `frame->is_extended_id`, `frame->is_remote_frame`, `proto->decode_unsupported_count`, `frame->id`, `proto->decode_ignored_count`, `frame->dlc`, `frame->data`, `proto->decode_invalid_count`, `out_message->version`, `out_message->message_type`, `out_message->source_node_id`, `out_message->target_node_id`, `out_message->text_type`, `out_message->text_length`, `out_message->payload_kind`, `out_message->text`, `proto->decode_ok_count`, `out_message->request_id`, `out_message->flags`, `out_message->payload_length`, `out_message->payload`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanProto_ClearMessage`, `CanProto_IdToMessageType`, `CanProto_IsValidNodeId`, `memcpy`, `CanProto_IsPrintableAscii`

### `services/can_service.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: CAN request/response service 인터페이스다. 이 계층은 raw transport와 protocol 모듈 위에, request 추적과 timeout 처리, 메시지 queue를 추가한다.

자료구조/열거형

#### `CanPendingRequest` (struct)

응답을 기다리는 추적 대상 송신 요청 하나를 나타낸다. service는 target과 command, timing 정보를 저장해 두고, 나중에 response를 매칭하거나 timeout 결과를 만들어 낸다.

- `uint8_t in_use`
- `uint8_t request_id`
- `uint8_t target_node_id`
- `uint8_t command_code`
- `uint32_t start_tick_ms`
- `uint32_t timeout_ms`

#### `CanTransport` (struct)

application 모듈이 사용하는 전체 CAN service context다. protocol과 transport, pending request, incoming message, result queue를 하나의 cohesive object로 묶어 관리한다.

- `uint8_t initialized`
- `uint8_t tx_in_flight`
- `uint8_t last_error`
- `CanHw hw`
- `CanFrame tx_queue[CAN_TRANSPORT_TX_QUEUE_SIZE]`
- `uint8_t tx_head`
- `uint8_t tx_tail`
- `uint8_t tx_count`
- `CanFrame rx_queue[CAN_TRANSPORT_RX_QUEUE_SIZE]`
- `uint8_t rx_head`
- `uint8_t rx_tail`
- `uint8_t rx_count`
- `CanFrame current_tx_frame`
- `uint32_t hw_tx_ok_count_seen`
- `uint32_t hw_tx_error_count_seen`

#### `CanService` (struct)

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint8_t next_request_id`
- `uint8_t last_error`
- `uint32_t default_timeout_ms`
- `uint32_t current_tick_ms`
- `CanProto proto`
- `CanTransport transport`
- `CanPendingRequest pending_table[CAN_SERVICE_PENDING_SIZE]`
- `CanMessage incoming_queue[CAN_SERVICE_QUEUE_SIZE]`
- `uint8_t incoming_head`
- `uint8_t incoming_tail`
- `uint8_t incoming_count`
- `CanServiceResult result_queue[CAN_SERVICE_QUEUE_SIZE]`
- `uint8_t result_head`
- `uint8_t result_tail`
- `uint8_t result_count`

### `services/can_service.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다.

#### `static void CanService_ClearMessage(CanMessage *message)`

- 위치: `services/can_service.c:16`
- 역할: 작은 reset helper들을 한곳에 두어 queue와 pending-table 코드를 단순화한다. 객체 초기화를 중앙화해 두면, service가 여러 곳에서 memset 로직을 반복하지 않고 저장 공간을 재사용할 수 있다.
- 파라미터: `CanMessage *message`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static void CanService_ClearResult(CanServiceResult *result)`

- 위치: `services/can_service.c:24`
- 역할: `CanService_ClearResult`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanServiceResult *result`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static void CanService_ClearPending(CanPendingRequest *pending)`

- 위치: `services/can_service.c:32`
- 역할: `CanService_ClearPending`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanPendingRequest *pending`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static uint8_t CanService_NextIndex(uint8_t index, uint8_t capacity)`

- 위치: `services/can_service.c:40`
- 역할: `CanService_NextIndex`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint8_t index`, `uint8_t capacity`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanService_IsValidTarget(uint8_t node_id)`

- 위치: `services/can_service.c:51`
- 역할: `CanService_IsValidTarget`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint8_t node_id`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanService_IsAcceptedTarget(const CanService *service, uint8_t target_node_id)`

- 위치: `services/can_service.c:61`
- 역할: `CanService_IsAcceptedTarget`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanService *service`, `uint8_t target_node_id`
- 로컬 변수: 없음
- 접근 상태/필드: `service->local_node_id`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanService_IsPrintableAscii(const char *text)`

- 위치: `services/can_service.c:76`
- 역할: `CanService_IsPrintableAscii`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const char *text`
- 로컬 변수: `unsigned char ch`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void CanTransport_ClearFrame(CanFrame *frame)`

- 위치: `services/can_service.c:98`
- 역할: `CanTransport_ClearFrame`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanFrame *frame`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static uint8_t CanTransport_TxIsFull(const CanTransport *transport)`

- 위치: `services/can_service.c:106`
- 역할: `CanTransport_TxIsFull`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanTransport *transport`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->tx_count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanTransport_RxIsFull(const CanTransport *transport)`

- 위치: `services/can_service.c:111`
- 역할: `CanTransport_RxIsFull`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanTransport *transport`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->rx_count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanTransport_TxPeek(const CanTransport *transport, CanFrame *out_frame)`

- 위치: `services/can_service.c:116`
- 역할: `CanTransport_TxPeek`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanTransport *transport`, `CanFrame *out_frame`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->tx_count`, `transport->tx_queue`, `transport->tx_head`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void CanTransport_TxDropFront(CanTransport *transport)`

- 위치: `services/can_service.c:127`
- 역할: `CanTransport_TxDropFront`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->tx_count`, `transport->tx_queue`, `transport->tx_head`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_ClearFrame`, `CanService_NextIndex`

#### `static void CanTransport_OnTxComplete(CanTransport *transport)`

- 위치: `services/can_service.c:139`
- 역할: `CanTransport_OnTxComplete`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->hw`, `transport->hw_tx_ok_count_seen`, `transport->tx_in_flight`, `transport->current_tx_frame`, `transport->hw_tx_error_count_seen`, `transport->last_error`, `hw.tx_ok_count`, `hw.tx_error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_TxDropFront`, `CanTransport_ClearFrame`

#### `static uint8_t CanTransport_RxPush(CanTransport *transport, const CanFrame *frame)`

- 위치: `services/can_service.c:171`
- 역할: `CanTransport_RxPush`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`, `const CanFrame *frame`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->rx_queue`, `transport->rx_tail`, `transport->rx_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_RxIsFull`, `CanService_NextIndex`

#### `static void CanTransport_DrainHwRx(CanTransport *transport)`

- 위치: `services/can_service.c:189`
- 역할: `CanTransport_DrainHwRx`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`
- 로컬 변수: `CanFrame frame`
- 접근 상태/필드: `transport->hw`, `transport->last_error`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_TryPopRx`, `CanTransport_RxPush`

#### `static void CanTransport_ProcessTx(CanTransport *transport)`

- 위치: `services/can_service.c:207`
- 역할: `CanTransport_ProcessTx`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`
- 로컬 변수: `CanFrame frame`
- 접근 상태/필드: `transport->tx_in_flight`, `transport->hw`, `transport->tx_count`, `transport->last_error`, `transport->current_tx_frame`, `transport->hw_tx_ok_count_seen`, `transport->hw_tx_error_count_seen`, `hw.tx_ok_count`, `hw.tx_error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_IsTxBusy`, `CanTransport_OnTxComplete`, `CanTransport_TxPeek`, `CanHw_StartTx`, `CanTransport_TxDropFront`

#### `static uint8_t CanTransport_Init(CanTransport *transport, uint8_t local_node_id)`

- 위치: `services/can_service.c:249`
- 역할: `CanTransport_Init`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`, `uint8_t local_node_id`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->hw`, `transport->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanHw_InitDefault`

#### `static void CanTransport_Task(CanTransport *transport, uint32_t now_ms)`

- 위치: `services/can_service.c:266`
- 역할: `CanTransport_Task`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->initialized`, `transport->hw`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanHw_Task`, `CanTransport_DrainHwRx`, `CanTransport_ProcessTx`

#### `static uint8_t CanTransport_SendFrame(CanTransport *transport, const CanFrame *frame)`

- 위치: `services/can_service.c:278`
- 역할: `CanTransport_SendFrame`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`, `const CanFrame *frame`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->initialized`, `transport->last_error`, `transport->tx_queue`, `transport->tx_tail`, `transport->tx_count`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_TxIsFull`, `CanService_NextIndex`

#### `static uint8_t CanTransport_PopRx(CanTransport *transport, CanFrame *out_frame)`

- 위치: `services/can_service.c:301`
- 역할: `CanTransport_PopRx`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanTransport *transport`, `CanFrame *out_frame`
- 로컬 변수: 없음
- 접근 상태/필드: `transport->initialized`, `transport->rx_count`, `transport->rx_queue`, `transport->rx_head`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_ClearFrame`, `CanService_NextIndex`

#### `static uint8_t CanService_IncomingQueuePush(CanService *service, const CanMessage *message)`

- 위치: `services/can_service.c:320`
- 역할: `CanService_IncomingQueuePush`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `const CanMessage *message`
- 로컬 변수: 없음
- 접근 상태/필드: `service->incoming_count`, `service->last_error`, `service->incoming_queue`, `service->incoming_tail`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_NextIndex`

#### `static uint8_t CanService_ResultQueuePush(CanService *service, const CanServiceResult *result)`

- 위치: `services/can_service.c:339`
- 역할: `CanService_ResultQueuePush`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `const CanServiceResult *result`
- 로컬 변수: 없음
- 접근 상태/필드: `service->result_count`, `service->last_error`, `service->result_queue`, `service->result_tail`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_NextIndex`

#### `static uint8_t CanService_AllocateRequestId(CanService *service)`

- 위치: `services/can_service.c:358`
- 역할: `CanService_AllocateRequestId`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`
- 로컬 변수: `uint8_t request_id`
- 접근 상태/필드: `service->next_request_id`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static int32_t CanService_FindFreePendingSlot(CanService *service)`

- 위치: `services/can_service.c:376`
- 역할: `CanService_FindFreePendingSlot`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`
- 로컬 변수: `uint8_t index`
- 접근 상태/필드: `service->pending_table`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static int32_t CanService_FindPendingByResponse(CanService *service, const CanMessage *message)`

- 위치: `services/can_service.c:391`
- 역할: `CanService_FindPendingByResponse`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `const CanMessage *message`
- 로컬 변수: `uint8_t index`
- 접근 상태/필드: `service->pending_table`, `message->request_id`, `message->source_node_id`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t CanService_SendMessage(CanService *service, const CanMessage *message)`

- 위치: `services/can_service.c:418`
- 역할: `CanService_SendMessage`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `const CanMessage *message`
- 로컬 변수: `CanEncodedFrameList encoded_list`, `uint8_t index`
- 접근 상태/필드: `service->initialized`, `service->last_error`, `service->proto`, `service->transport`, `encoded_list.count`, `encoded_list.frames`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanProto_EncodeMessage`, `CanTransport_SendFrame`

#### `static void CanService_FillTimeoutResult(const CanPendingRequest *pending, uint8_t local_node_id, CanServiceResult *out_result)`

- 위치: `services/can_service.c:450`
- 역할: `CanService_FillTimeoutResult`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanPendingRequest *pending`, `uint8_t local_node_id`, `CanServiceResult *out_result`
- 로컬 변수: 없음
- 접근 상태/필드: `out_result->kind`, `out_result->request_id`, `pending->request_id`, `out_result->source_node_id`, `pending->target_node_id`, `out_result->target_node_id`, `out_result->command_code`, `pending->command_code`, `out_result->result_code`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_ClearResult`

#### `static void CanService_FillResponseResult(const CanPendingRequest *pending, const CanMessage *message, CanServiceResult *out_result)`

- 위치: `services/can_service.c:463`
- 역할: `CanService_FillResponseResult`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const CanPendingRequest *pending`, `const CanMessage *message`, `CanServiceResult *out_result`
- 로컬 변수: 없음
- 접근 상태/필드: `out_result->kind`, `out_result->request_id`, `message->request_id`, `out_result->source_node_id`, `message->source_node_id`, `out_result->target_node_id`, `message->target_node_id`, `out_result->command_code`, `pending->command_code`, `out_result->result_code`, `message->payload`, `out_result->detail_code`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_ClearResult`

#### `static void CanService_ProcessResponse(CanService *service, const CanMessage *message)`

- 위치: `services/can_service.c:477`
- 역할: `CanService_ProcessResponse`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `const CanMessage *message`
- 로컬 변수: `int32_t slot_index`, `CanServiceResult result`
- 접근 상태/필드: `service->pending_table`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_FindPendingByResponse`, `CanService_FillResponseResult`, `CanService_ResultQueuePush`

#### `static void CanService_ProcessDecodedMessage(CanService *service, const CanMessage *message)`

- 위치: `services/can_service.c:493`
- 역할: `CanService_ProcessDecodedMessage`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `const CanMessage *message`
- 로컬 변수: 없음
- 접근 상태/필드: `message->message_type`, `message->target_node_id`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_ProcessResponse`, `CanService_IsAcceptedTarget`, `CanService_IncomingQueuePush`

#### `static void CanService_ProcessRx(CanService *service)`

- 위치: `services/can_service.c:509`
- 역할: `CanService_ProcessRx`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`
- 로컬 변수: `CanFrame frame`, `CanMessage message`, `CanProtoDecodeStatus decode_status`
- 접근 상태/필드: `service->transport`, `service->proto`, `service->last_error`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_PopRx`, `CanProto_DecodeFrame`, `CanService_ProcessDecodedMessage`

#### `static void CanService_ProcessTimeouts(CanService *service)`

- 위치: `services/can_service.c:529`
- 역할: `CanService_ProcessTimeouts`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`
- 로컬 변수: `uint8_t index`, `CanServiceResult result`
- 접근 상태/필드: `service->pending_table`, `service->current_tick_ms`, `service->local_node_id`
- 사용 전역/static: 없음
- 직접 호출 함수: `Infra_TimeIsExpired`, `CanService_FillTimeoutResult`, `CanService_ResultQueuePush`

#### `uint8_t CanService_Init(CanService *service, uint8_t local_node_id, uint32_t default_timeout_ms)`

- 위치: `services/can_service.c:561`
- 역할: protocol과 transport, request 추적 상태를 초기화한다. service는 빈 queue와 새 request ID 흐름으로 시작하여, 상위 계층이 바로 작업을 제출할 수 있게 만든다.
- 파라미터: `CanService *service`, `uint8_t local_node_id`, `uint32_t default_timeout_ms`
- 로컬 변수: `CanProtoConfig proto_config`, `uint8_t index`
- 접근 상태/필드: `service->local_node_id`, `service->default_timeout_ms`, `service->next_request_id`, `service->pending_table`, `service->proto`, `service->last_error`, `service->transport`, `service->initialized`, `proto_config.local_node_id`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanService_ClearPending`, `CanProto_Init`, `CanTransport_Init`

#### `void CanService_Task(CanService *service, uint32_t now_ms)`

- 위치: `services/can_service.c:605`
- 역할: transport 작업을 진행시키고 새 RX 트래픽을 decode하며 timeout을 확인한다. 이 주기 task는 AppCore가 사용하는 CAN request/response service의, 중앙 실행 지점이다.
- 파라미터: `CanService *service`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `service->initialized`, `service->current_tick_ms`, `service->transport`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_Task`, `CanService_ProcessRx`, `CanService_ProcessTimeouts`

#### `void CanService_FlushTx(CanService *service, uint32_t now_ms)`

- 위치: `services/can_service.c:618`
- 역할: `CanService_FlushTx`는 pending request 관리가 들어간 CAN service 구현부다. request ID를 추적하고 수신 트래픽을 decode하며, timeout 또는 response 결과를 app 계층에 제공한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanService *service`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `service->initialized`, `service->current_tick_ms`, `service->transport`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanTransport_Task`

#### `uint8_t CanService_SendCommand(CanService *service, uint8_t target_node_id, uint8_t command_code, uint8_t arg0, uint8_t arg1, uint8_t need_response)`

- 위치: `services/can_service.c:634`
- 역할: CAN 전송용 논리 command 메시지 하나를 제출한다. 응답을 추적해야 하는 command는 먼저 pending slot을 확보한 뒤, encode된 메시지를 transport queue에 넣는다.
- 파라미터: `CanService *service`, `uint8_t target_node_id`, `uint8_t command_code`, `uint8_t arg0`, `uint8_t arg1`, `uint8_t need_response`
- 로컬 변수: `CanMessage message`, `int32_t slot_index`, `uint8_t request_id`
- 접근 상태/필드: `service->initialized`, `service->last_error`, `service->local_node_id`, `service->pending_table`, `service->current_tick_ms`, `service->default_timeout_ms`, `message.version`, `message.message_type`, `message.source_node_id`, `message.target_node_id`, `message.payload_kind`, `message.payload_length`, `message.payload`, `message.request_id`, `message.flags`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_IsValidTarget`, `memset`, `CanService_FindFreePendingSlot`, `CanService_AllocateRequestId`, `CanService_SendMessage`

#### `uint8_t CanService_SendResponse(CanService *service, uint8_t target_node_id, uint8_t request_id, uint8_t result_code, uint8_t detail_code)`

- 위치: `services/can_service.c:710`
- 역할: 이전에 추적하던 요청에 대한 protocol response를 보낸다. caller는 request ID와 result code만 제공하고, 메시지 조립과 전송은 service가 담당한다.
- 파라미터: `CanService *service`, `uint8_t target_node_id`, `uint8_t request_id`, `uint8_t result_code`, `uint8_t detail_code`
- 로컬 변수: `CanMessage message`
- 접근 상태/필드: `service->initialized`, `service->local_node_id`, `message.version`, `message.message_type`, `message.source_node_id`, `message.target_node_id`, `message.request_id`, `message.payload_kind`, `message.payload_length`, `message.payload`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_IsValidTarget`, `memset`, `CanService_SendMessage`

#### `uint8_t CanService_SendEvent(CanService *service, uint8_t target_node_id, uint8_t event_code, uint8_t arg0, uint8_t arg1)`

- 위치: `services/can_service.c:746`
- 역할: 응답 추적이 없는 event 형태 CAN 메시지 하나를 보낸다. 이 경로는 pending slot 기반 request/response 흐름 대신, 비동기 알림을 보낼 때 사용한다.
- 파라미터: `CanService *service`, `uint8_t target_node_id`, `uint8_t event_code`, `uint8_t arg0`, `uint8_t arg1`
- 로컬 변수: `CanMessage message`
- 접근 상태/필드: `service->initialized`, `service->local_node_id`, `message.version`, `message.message_type`, `message.source_node_id`, `message.target_node_id`, `message.payload_kind`, `message.payload_length`, `message.payload`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_IsValidTarget`, `memset`, `CanService_SendMessage`

#### `uint8_t CanService_SendText(CanService *service, uint8_t target_node_id, uint8_t text_type, const char *text)`

- 위치: `services/can_service.c:782`
- 역할: 짧은 printable text payload 하나를 CAN으로 보낸다. 검증은 여기서 수행하므로, 상위 계층은 target node와 보낼 메시지만 제공하면 된다.
- 파라미터: `CanService *service`, `uint8_t target_node_id`, `uint8_t text_type`, `const char *text`
- 로컬 변수: `CanMessage message`, `size_t text_length`
- 접근 상태/필드: `service->initialized`, `service->last_error`, `service->local_node_id`, `message.version`, `message.message_type`, `message.source_node_id`, `message.target_node_id`, `message.payload_kind`, `message.text_type`, `message.text_length`, `message.text`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_IsValidTarget`, `CanService_IsPrintableAscii`, `strlen`, `memset`, `memcpy`, `CanService_SendMessage`

#### `uint8_t CanService_PopReceivedMessage(CanService *service, CanMessage *out_message)`

- 위치: `services/can_service.c:830`
- 역할: decode된 비-response 메시지 하나를 app 계층으로 꺼낸다. incoming command와 event, text는 모두, protocol 및 target 검증 뒤 이 queue를 통해 AppCore에 도달한다.
- 파라미터: `CanService *service`, `CanMessage *out_message`
- 로컬 변수: 없음
- 접근 상태/필드: `service->incoming_count`, `service->incoming_queue`, `service->incoming_head`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_ClearMessage`, `CanService_NextIndex`

#### `uint8_t CanService_PopResult(CanService *service, CanServiceResult *out_result)`

- 위치: `services/can_service.c:849`
- 역할: 완료된 request result 하나를 꺼내 operator 피드백에 사용한다. 이 결과는 응답과 매칭된 결과이거나, acknowledgement를 기대한 command의 timeout 합성 결과일 수 있다.
- 파라미터: `CanService *service`, `CanServiceResult *out_result`
- 로컬 변수: 없음
- 접근 상태/필드: `service->result_count`, `service->result_queue`, `service->result_head`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_ClearResult`, `CanService_NextIndex`

### `services/can_module.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: app 계층에서 바라보는 CAN 편의 계층이다. 이 모듈은 high-level request를 버퍼링하여, AppCore가 service 내부 구조나 timing 세부사항에 의존하지 않게 한다.

자료구조/열거형

#### `CanModuleRequestKind` (enum)

- `CAN_MODULE_REQUEST_COMMAND = 0`
- `CAN_MODULE_REQUEST_RESPONSE`
- `CAN_MODULE_REQUEST_EVENT`
- `CAN_MODULE_REQUEST_TEXT`

#### `CanModuleRequest` (struct)

제출을 기다리는 app 수준 CAN 작업 항목 하나다. AppCore는 이 작은 request를 만들기만 하고, 실제 service/transport 계층으로의 유입 조절은 module에 맡긴다.

- `uint8_t kind`
- `uint8_t target_node_id`
- `uint8_t code0`
- `uint8_t code1`
- `uint8_t code2`
- `uint8_t need_response`
- `char text[CAN_TEXT_MAX_LEN + 1U]`

#### `CanModuleConfig` (struct)

startup 시 선택되는 정적 CAN module 설정이다. module은 이 값을 사용해 node identity와 기본 timeout, scheduler tick마다 제출할 작업량을 정한다.

- `uint8_t local_node_id`
- `uint8_t default_target_node_id`
- `uint32_t default_timeout_ms`
- `uint8_t max_submit_per_tick`

#### `CanModule` (struct)

app 계층에서 바라보는 CAN module 상태다. 공개 request queue와, 실제 protocol 작업을 수행하는 내부 CAN service 객체를 함께 묶는다.

- `uint8_t initialized`
- `uint8_t local_node_id`
- `uint8_t default_target_node_id`
- `uint8_t max_submit_per_tick`
- `uint8_t last_activity`
- `CanService service`
- `InfraQueue request_queue`
- `CanModuleRequest request_storage[CAN_MODULE_REQUEST_QUEUE_SIZE]`

### `services/can_module.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`
- 모듈 역할: app 계층을 위한 CAN request 버퍼링 구현부다. scheduler tick마다 CanService로 넘어가는 작업량을 조절하여, 애플리케이션 코드가 transport보다 의도 표현에 집중하게 만든다.

#### `static InfraStatus CanModule_PushRequest(CanModule *module, const CanModuleRequest *request)`

- 위치: `services/can_module.c:16`
- 역할: app 수준 CAN 요청 하나를 소프트웨어 buffer에 올린다. 이렇게 하면 app 코드가 service 가용성과 분리되고, module이 CAN service로의 작업 유입을 점진적으로 조절할 수 있다.
- 파라미터: `CanModule *module`, `const CanModuleRequest *request`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->request_queue`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_Push`

#### `static void CanModule_SubmitPending(CanModule *module)`

- 위치: `services/can_module.c:31`
- 역할: queue에 쌓인 app 요청을 아래쪽 CAN service로 옮긴다. tick당 제출량을 제한해 두어, 바쁜 콘솔 사이클 하나가 transport 작업을 한 번에 독점하지 못하게 한다.
- 파라미터: `CanModule *module`
- 로컬 변수: `CanModuleRequest request`, `uint8_t submitted`, `uint8_t send_ok`
- 접근 상태/필드: `module->initialized`, `module->max_submit_per_tick`, `module->request_queue`, `module->service`, `module->last_activity`, `request.kind`, `request.target_node_id`, `request.code0`, `request.code1`, `request.code2`, `request.need_response`, `request.text`, `service.current_tick_ms`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_Peek`, `CanService_SendCommand`, `CanService_SendResponse`, `CanService_SendEvent`, `CanService_SendText`, `InfraQueue_Pop`, `CanService_FlushTx`

#### `InfraStatus CanModule_Init(CanModule *module, const CanModuleConfig *config)`

- 위치: `services/can_module.c:111`
- 역할: app 계층용 CAN module과 request queue를 초기화한다. 내부 CanService 객체도 여기서 함께 생성해 두어, 이후 AppCore task가 추가 setup 없이 작업을 제출할 수 있게 한다.
- 파라미터: `CanModule *module`, `const CanModuleConfig *config`
- 로컬 변수: 없음
- 접근 상태/필드: `module->local_node_id`, `config->local_node_id`, `module->default_target_node_id`, `config->default_target_node_id`, `module->max_submit_per_tick`, `config->max_submit_per_tick`, `module->request_queue`, `module->request_storage`, `module->service`, `config->default_timeout_ms`, `module->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `InfraQueue_Init`, `CanService_Init`

#### `void CanModule_Task(CanModule *module, uint32_t now_ms)`

- 위치: `services/can_module.c:151`
- 역할: CAN service를 진행시키고 pending app 요청을 제출한다. 활성 역할에서 CAN 통신을 유지하기 위해, AppCore가 주기적으로 호출하면 되는 유일한 진입점이다.
- 파라미터: `CanModule *module`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->last_activity`, `module->service`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_Task`, `CanModule_SubmitPending`

#### `InfraStatus CanModule_QueueCommand(CanModule *module, uint8_t target_node_id, uint8_t command_code, uint8_t arg0, uint8_t arg1, uint8_t need_response)`

- 위치: `services/can_module.c:168`
- 역할: app 계층에서 command 형태 CAN 요청 하나를 queue에 올린다. 이 요청은 이후 task tick이 도착해, 아래쪽 CAN service로 밀어 넣을 때까지 module queue에 머문다.
- 파라미터: `CanModule *module`, `uint8_t target_node_id`, `uint8_t command_code`, `uint8_t arg0`, `uint8_t arg1`, `uint8_t need_response`
- 로컬 변수: `CanModuleRequest request`
- 접근 상태/필드: `request.kind`, `request.target_node_id`, `request.code0`, `request.code1`, `request.code2`, `request.need_response`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanModule_PushRequest`

#### `InfraStatus CanModule_QueueResponse(CanModule *module, uint8_t target_node_id, uint8_t request_id, uint8_t result_code, uint8_t detail_code)`

- 위치: `services/can_module.c:192`
- 역할: app 수준 command 처리 중 생성된 CAN response 하나를 queue에 올린다. AppCore는 수신 요청이 로컬 노드의 원격 acknowledgement를 요구할 때, 이 함수를 사용한다.
- 파라미터: `CanModule *module`, `uint8_t target_node_id`, `uint8_t request_id`, `uint8_t result_code`, `uint8_t detail_code`
- 로컬 변수: `CanModuleRequest request`
- 접근 상태/필드: `request.kind`, `request.target_node_id`, `request.code0`, `request.code1`, `request.code2`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanModule_PushRequest`

#### `InfraStatus CanModule_QueueEvent(CanModule *module, uint8_t target_node_id, uint8_t event_code, uint8_t arg0, uint8_t arg1)`

- 위치: `services/can_module.c:214`
- 역할: 나중에 제출할 비동기 event 메시지 하나를 queue에 올린다. module은 이 경로를 통해, app 계층의 command/response 흐름과 event 생성 방식도 일관되게 유지한다.
- 파라미터: `CanModule *module`, `uint8_t target_node_id`, `uint8_t event_code`, `uint8_t arg0`, `uint8_t arg1`
- 로컬 변수: `CanModuleRequest request`
- 접근 상태/필드: `request.kind`, `request.target_node_id`, `request.code0`, `request.code1`, `request.code2`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `CanModule_PushRequest`

#### `InfraStatus CanModule_QueueText(CanModule *module, uint8_t target_node_id, const char *text)`

- 위치: `services/can_module.c:236`
- 역할: CAN으로 보낼 짧은 text payload 하나를 queue에 올린다. 이 경로는 UART 콘솔에서 나온, operator 기원 메시지를 전송할 때 특히 유용하다.
- 파라미터: `CanModule *module`, `uint8_t target_node_id`, `const char *text`
- 로컬 변수: `CanModuleRequest request`
- 접근 상태/필드: `request.kind`, `request.target_node_id`, `request.text`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `strncpy`, `CanModule_PushRequest`

#### `uint8_t CanModule_TryPopIncoming(CanModule *module, CanMessage *out_message)`

- 위치: `services/can_module.c:255`
- 역할: `CanModule_TryPopIncoming`는 app 계층을 위한 CAN request 버퍼링 구현부다. scheduler tick마다 CanService로 넘어가는 작업량을 조절하여, 애플리케이션 코드가 transport보다 의도 표현에 집중하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanModule *module`, `CanMessage *out_message`
- 로컬 변수: 없음
- 접근 상태/필드: `module->service`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_PopReceivedMessage`

#### `uint8_t CanModule_TryPopResult(CanModule *module, CanServiceResult *out_result)`

- 위치: `services/can_module.c:265`
- 역할: `CanModule_TryPopResult`는 app 계층을 위한 CAN request 버퍼링 구현부다. scheduler tick마다 CanService로 넘어가는 작업량을 조절하여, 애플리케이션 코드가 transport보다 의도 표현에 집중하게 만든다. 맥락에서 동작하는 helper/API다.
- 파라미터: `CanModule *module`, `CanServiceResult *out_result`
- 로컬 변수: 없음
- 접근 상태/필드: `module->service`
- 사용 전역/static: 없음
- 직접 호출 함수: `CanService_PopResult`

### `services/lin_module.h`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: portable LIN 프로토콜 상태기계 인터페이스다. master와 slave 역할이 같은 API를 재사용하고, runtime_io가 아래쪽에서 하드웨어 전용 callback 바인딩을 제공한다.

자료구조/열거형

#### `LinRole` (enum)

- `LIN_ROLE_MASTER = 1`
- `LIN_ROLE_SLAVE = 2`

#### `LinEventId` (enum)

- `LIN_EVENT_NONE = 0`
- `LIN_EVENT_PID_OK`
- `LIN_EVENT_RX_DONE`
- `LIN_EVENT_TX_DONE`
- `LIN_EVENT_ERROR`

#### `LinZone` (enum)

- `LIN_ZONE_SAFE = 0`
- `LIN_ZONE_WARNING`
- `LIN_ZONE_DANGER`
- `LIN_ZONE_EMERGENCY`

#### `LinBinding` (struct)

portable LIN 모듈이 사용하는 하드웨어 바인딩 callback 표다. Runtime IO가 이 테이블을 채워 넣어, 상태기계가 SDK에 직접 연결되지 않고도 실제 버스 동작을 요청하게 한다.

- `LinHwInitFn init_fn`
- `LinHwMasterSendHeaderFn master_send_header_fn`
- `LinHwStartReceiveFn start_receive_fn`
- `LinHwStartSendFn start_send_fn`
- `LinHwGotoIdleFn goto_idle_fn`
- `LinHwSetTimeoutFn set_timeout_fn`
- `LinHwServiceTickFn service_tick_fn`
- `void *context`

#### `LinStatusFrame` (struct)

LIN으로 교환되는 해석된 센서 상태 구조체다. 이 프레임은 ADC 값과 semantic zone, 그리고 latch 상태를 slave2와 master policy 사이에 전달한다.

- `uint16_t adc_value`
- `uint8_t zone`
- `uint8_t emergency_latched`
- `uint8_t valid`
- `uint8_t fresh`

#### `LinConfig` (struct)

하나의 모듈 인스턴스에 대한 정적 LIN 프로토콜 설정이다. PID와 token 값, timing, 하드웨어 callback을 모두 묶어, 상태기계가 시작되기 전에 한 번에 넘긴다.

- `uint8_t role`
- `uint8_t pid_status`
- `uint8_t pid_ok`
- `uint8_t ok_token`
- `uint8_t status_frame_size`
- `uint8_t ok_frame_size`
- `uint16_t timeout_ticks`
- `uint32_t poll_period_ms`
- `LinBinding binding`

#### `LinModule` (struct)

master 또는 slave 모드에서 쓰는 전체 LIN 모듈 상태다. protocol flag와 현재 buffer, 그리고 application 계층과 공유하는 최신 상태 프레임을 함께 보관한다.

- `uint8_t initialized`
- `LinConfig config`
- `volatile uint8_t state`
- `volatile uint32_t flags`
- `volatile uint8_t current_pid`
- `uint32_t last_poll_ms`
- `volatile uint8_t ok_tx_pending`
- `volatile uint8_t ok_token_pending`
- `uint8_t rx_buffer[8]`
- `uint8_t tx_buffer[8]`
- `LinStatusFrame latest_status`
- `LinStatusFrame slave_status_cache`

### `services/lin_module.c`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다.

#### `static void LinModule_GotoIdle(LinModule *module)`

- 위치: `services/lin_module.c:26`
- 역할: LIN 상태기계를 중립 idle 상태로 되돌린다. 이 helper는 하드웨어 바인딩에도 활성 전송 상태를 정리하라고 요청하여, 이후 transaction이 깨끗하게 시작되도록 한다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `module->config`, `module->state`, `module->current_pid`, `config.binding`
- 사용 전역/static: 없음
- 직접 호출 함수: `goto_idle_fn`

#### `static void LinModule_SetTimeout(LinModule *module)`

- 위치: `services/lin_module.c:42`
- 역할: `LinModule_SetTimeout`는 portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `module->config`, `config.binding`, `config.timeout_ticks`
- 사용 전역/static: 없음
- 직접 호출 함수: `set_timeout_fn`

#### `static void LinModule_PrepareStatusTx(const LinStatusFrame *status, uint8_t *buffer, uint8_t size)`

- 위치: `services/lin_module.c:58`
- 역할: cache된 status frame을 raw LIN payload buffer로 포장한다. slave 모드는 master가 status PID를 poll하고, 최신 센서 정보를 기대할 때 이 helper를 사용한다.
- 파라미터: `const LinStatusFrame *status`, `uint8_t *buffer`, `uint8_t size`
- 로컬 변수: `uint16_t adc_value`
- 접근 상태/필드: `status->adc_value`, `status->zone`, `status->emergency_latched`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `static void LinModule_ParseStatusRx(LinModule *module)`

- 위치: `services/lin_module.c:89`
- 역할: 수신한 LIN status payload를 모듈 상태로 decode한다. master 모드는 결과를 latest_status에 저장해, app 계층이 나중에 fresh 센서 업데이트를 소비할 수 있게 한다.
- 파라미터: `LinModule *module`
- 로컬 변수: `uint16_t adc_value`
- 접근 상태/필드: `module->rx_buffer`, `module->latest_status`, `latest_status.adc_value`, `latest_status.zone`, `latest_status.emergency_latched`, `latest_status.valid`, `latest_status.fresh`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static InfraStatus LinModule_MasterStart(LinModule *module, uint8_t pid)`

- 위치: `services/lin_module.c:117`
- 역할: header를 보내서 master 측 LIN transaction 하나를 시작한다. 이 함수는 idle 상태에서만 성공하도록 제한해, 상태기계가 한 번에 하나의 status/token 교환만 다루게 한다.
- 파라미터: `LinModule *module`, `uint8_t pid`
- 로컬 변수: 없음
- 접근 상태/필드: `module->config`, `module->state`, `module->current_pid`, `config.binding`
- 사용 전역/static: 없음
- 직접 호출 함수: `master_send_header_fn`, `LinModule_GotoIdle`

#### `InfraStatus LinModule_Init(LinModule *module, const LinConfig *config)`

- 위치: `services/lin_module.c:145`
- 역할: portable LIN 상태기계를 초기화한다. 먼저 설정을 복사하고, 그다음 하드웨어 바인딩에 concrete peripheral 초기화를 요청한다.
- 파라미터: `LinModule *module`, `const LinConfig *config`
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: `module->config`, `module->state`, `module->initialized`, `config.binding`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `init_fn`

#### `void LinModule_OnBaseTick(LinModule *module)`

- 위치: `services/lin_module.c:173`
- 역할: `LinModule_OnBaseTick`는 portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->config`, `config.binding`
- 사용 전역/static: 없음
- 직접 호출 함수: `service_tick_fn`

#### `void LinModule_OnEvent(LinModule *module, LinEventId event_id, uint8_t current_pid)`

- 위치: `services/lin_module.c:191`
- 역할: runtime_io가 전달한 하드웨어 event 하나를 소비한다. master 모드는 fast task용 flag를 기록하고, slave 모드는 일치하는 PID에 대해 바로 RX/TX 작업을 시작할 수 있다.
- 파라미터: `LinModule *module`, `LinEventId event_id`, `uint8_t current_pid`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->current_pid`, `module->config`, `module->state`, `module->rx_buffer`, `module->tx_buffer`, `module->ok_tx_pending`, `module->flags`, `module->slave_status_cache`, `module->ok_token_pending`, `config.role`, `config.pid_status`, `config.binding`, `config.status_frame_size`, `config.pid_ok`, `config.ok_token`, `config.ok_frame_size`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_SetTimeout`, `start_receive_fn`, `LinModule_GotoIdle`, `start_send_fn`, `LinModule_PrepareStatusTx`

#### `void LinModule_TaskFast(LinModule *module, uint32_t now_ms)`

- 위치: `services/lin_module.c:338`
- 역할: master 측 LIN 상태기계를 빠르게 진행시킨다. 이 task는 기록된 PID, RX, TX, error event에 반응하여, 느린 poll 주기 사이에도 status polling이 responsive하게 유지되도록 한다.
- 파라미터: `LinModule *module`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->config`, `module->flags`, `module->current_pid`, `module->ok_tx_pending`, `module->state`, `config.role`, `config.pid_ok`
- 사용 전역/static: 없음
- 직접 호출 함수: `LinModule_GotoIdle`, `LinModule_ParseStatusRx`

#### `void LinModule_TaskPoll(LinModule *module, uint32_t now_ms)`

- 위치: `services/lin_module.c:384`
- 역할: 주기적인 master 측 LIN 작업을 시작한다. poll task는 현재 application 필요에 따라, status 요청과 queue에 쌓인 OK token 전송 중 하나를 선택한다.
- 파라미터: `LinModule *module`, `uint32_t now_ms`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->config`, `module->last_poll_ms`, `module->ok_tx_pending`, `config.role`, `config.poll_period_ms`, `config.pid_ok`, `config.pid_status`
- 사용 전역/static: 없음
- 직접 호출 함수: `Infra_TimeIsDue`, `LinModule_MasterStart`

#### `InfraStatus LinModule_RequestOk(LinModule *module)`

- 위치: `services/lin_module.c:416`
- 역할: 다음 master poll 슬롯에 보낼 OK token 전송을 queue에 올린다. 승인 로직은 즉시 보내지 않고 이 경로를 사용해, 버스 동작이 일반 상태기계 흐름 안에서 진행되게 한다.
- 파라미터: `LinModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->config`, `module->ok_tx_pending`, `module->tx_buffer`, `config.role`, `config.ok_token`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t LinModule_GetLatestStatus(const LinModule *module, LinStatusFrame *out_status)`

- 위치: `services/lin_module.c:433`
- 역할: `LinModule_GetLatestStatus`는 portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const LinModule *module`, `LinStatusFrame *out_status`
- 로컬 변수: 없음
- 접근 상태/필드: `module->latest_status`, `latest_status.valid`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t LinModule_ConsumeFreshStatus(LinModule *module, LinStatusFrame *out_status)`

- 위치: `services/lin_module.c:444`
- 역할: `LinModule_ConsumeFreshStatus`는 portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`, `LinStatusFrame *out_status`
- 로컬 변수: 없음
- 접근 상태/필드: `module->latest_status`, `latest_status.fresh`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void LinModule_SetSlaveStatus(LinModule *module, const LinStatusFrame *status)`

- 위치: `services/lin_module.c:456`
- 역할: `LinModule_SetSlaveStatus`는 portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`, `const LinStatusFrame *status`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->slave_status_cache`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t LinModule_ConsumeSlaveOkToken(LinModule *module)`

- 위치: `services/lin_module.c:466`
- 역할: `LinModule_ConsumeSlaveOkToken`는 portable LIN 상태기계 구현부다. protocol 흐름과 fresh status 저장, token 처리를 담당하고, 보드 전용 동작은 runtime_io에 의존한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `LinModule *module`
- 로컬 변수: `uint8_t pending`
- 접근 상태/필드: `module->ok_token_pending`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `services/uart_types.h`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`
- 모듈 역할: service 계층이 공유하는 UART 전송 자료구조 모음이다. 이 헤더의 타입들은 RX 버퍼링과 TX queueing, 그리고 콘솔 모듈이 들고 있는 전체 service 상태를 표현한다.

상수/매크로

- `UART_RX_LINE_SIZE` = `64U`
- `UART_RX_PENDING_SIZE` = `32U`
- `UART_TX_CHUNK_SIZE` = `128U`
- `UART_TX_QUEUE_SIZE` = `8U`
- `UART_DEFAULT_TIMEOUT_MS` = `100U`

자료구조/열거형

#### `UartErrorCode` (enum)

- `UART_ERROR_NONE = 0`
- `UART_ERROR_HW_INIT`
- `UART_ERROR_RX_DRIVER`
- `UART_ERROR_RX_PENDING_OVERFLOW`
- `UART_ERROR_RX_LINE_OVERFLOW`
- `UART_ERROR_TX_QUEUE_FULL`
- `UART_ERROR_TX_DRIVER`
- `UART_ERROR_TX_TIMEOUT`

#### `UartRxPendingRing` (struct)

UART callback이 받은 바이트를 저장하는 ring buffer다. 저수준 driver는 바이트를 빠르게 여기에 밀어 넣고, line 파싱은 나중에 일반 task context에서 수행하게 된다.

- `uint8_t buffer[UART_RX_PENDING_SIZE]`
- `volatile uint16_t head`
- `volatile uint16_t tail`
- `volatile uint8_t overflow`
- `volatile uint32_t overflow_count`

#### `UartLineBuffer` (struct)

콘솔 입력을 위해 현재 조립 중인 line buffer다. UartService는 printable 문자를 여기에 붙여 나가다가, 완전한 line 또는 overflow를 감지하면 처리를 넘긴다.

- `char buffer[UART_RX_LINE_SIZE]`
- `uint16_t length`
- `uint8_t line_ready`
- `uint8_t overflow`

#### `UartTxChunk` (struct)

- `uint8_t data[UART_TX_CHUNK_SIZE]`
- `uint16_t length`

#### `UartTxContext` (struct)

UART service가 관리하는 buffered transmit 상태다. 긴 텍스트 출력은 chunk로 잘라 두어, console이 app을 막지 않고 여러 render 갱신을 queue에 올릴 수 있게 한다.

- `char current_buffer[UART_TX_CHUNK_SIZE + 1U]`
- `uint16_t current_length`
- `uint8_t busy`
- `uint32_t start_ms`
- `uint32_t timeout_ms`
- `InfraQueue queue`
- `UartTxChunk queue_storage[UART_TX_QUEUE_SIZE]`

#### `UartService` (struct)

콘솔 계층이 소유하는 전체 UART service 상태다. 저수준 RX/TX 상태와 오류 추적, 그리고 operator 인터페이스가 쓰는 현재 line buffer를 함께 가진다.

- `uint8_t initialized`
- `uint32_t instance`
- `uint8_t rx_byte`
- `UartRxPendingRing rx_pending`
- `UartLineBuffer rx_line`
- `UartTxContext tx`
- `uint8_t error_flag`
- `uint32_t error_count`
- `UartErrorCode error_code`

### `services/uart_service.h`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`
- 모듈 역할: line 지향 UART service API다. 상위 계층은 driver를 몰라도, buffered transmit과 line 입력, 간단한 오류 복구를 이 인터페이스로 처리한다.

### `services/uart_service.c`

- 대표 프로젝트: `S32K_LinCan_master`
- 사용 프로젝트: `S32K_LinCan_master`
- 모듈 역할: 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다.

#### `static void UartService_SetError(UartService *service, UartErrorCode code)`

- 위치: `services/uart_service.c:21`
- 역할: service 수준 UART 오류 하나를 기록한다. 오류 bookkeeping을 한곳에 모아 두면, 모든 실패 경로가 같은 flag, code, counter 의미를 유지한다.
- 파라미터: `UartService *service`, `UartErrorCode code`
- 로컬 변수: 없음
- 접근 상태/필드: `service->error_flag`, `service->error_code`, `service->error_count`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static void UartService_ResetRx(UartService *service)`

- 위치: `services/uart_service.c:38`
- 역할: 버퍼링된 수신 상태를 모두 초기화한다. startup과 recovery, overflow 처리 때 사용하여, 다음 line이 깨끗한 parser 상태에서 시작되게 만든다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->rx_pending`, `service->rx_line`, `rx_pending.head`, `rx_pending.tail`, `rx_pending.overflow`, `rx_pending.overflow_count`, `rx_line.length`, `rx_line.line_ready`, `rx_line.overflow`, `rx_line.buffer`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static InfraStatus UartService_ResetTx(UartService *service)`

- 위치: `services/uart_service.c:56`
- 역할: `UartService_ResetTx`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->tx`, `tx.current_length`, `tx.current_buffer`, `tx.busy`, `tx.start_ms`, `tx.timeout_ms`, `tx.queue`, `tx.queue_storage`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_Init`

#### `static void UartService_Reset(UartService *service)`

- 위치: `services/uart_service.c:75`
- 역할: `UartService_Reset`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `UartService_ResetRx`, `UartService_ResetTx`, `UartService_SetError`

#### `static uint16_t UartService_NextPendingIndex(uint16_t index)`

- 위치: `services/uart_service.c:90`
- 역할: `UartService_NextPendingIndex`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `uint16_t index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static uint8_t UartService_IsPendingEmpty(const UartService *service)`

- 위치: `services/uart_service.c:101`
- 역할: `UartService_IsPendingEmpty`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->rx_pending`, `rx_pending.head`, `rx_pending.tail`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static InfraStatus UartService_PopPendingByte(UartService *service, uint8_t *out_byte)`

- 위치: `services/uart_service.c:111`
- 역할: `UartService_PopPendingByte`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `UartService *service`, `uint8_t *out_byte`
- 로컬 변수: 없음
- 접근 상태/필드: `service->rx_pending`, `rx_pending.buffer`, `rx_pending.head`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_IsPendingEmpty`, `UartService_NextPendingIndex`

#### `static void UartService_OnRxByte(UartService *service, uint8_t rx_byte)`

- 위치: `services/uart_service.c:133`
- 역할: 수신 바이트 하나를 현재 line buffer에 넣는다. parser는 newline 종료와 backspace 편집, printable 필터링과 overflow 감지를 이 한곳에서 처리한다.
- 파라미터: `UartService *service`, `uint8_t rx_byte`
- 로컬 변수: `UartLineBuffer *line`
- 접근 상태/필드: `service->rx_line`, `line->line_ready`, `line->length`, `line->buffer`, `line->overflow`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `static InfraStatus UartService_RequestTxBytes(UartService *service, const uint8_t *data, uint16_t length)`

- 위치: `services/uart_service.c:196`
- 역할: 임의의 바이트를 나중에 보낼 UART 전송 queue에 올린다. 긴 텍스트 블록은 제한된 크기의 chunk로 나뉘어, driver가 service 계층에서 관리하기 쉬운 전송만 보게 된다.
- 파라미터: `UartService *service`, `const uint8_t *data`, `uint16_t length`
- 로컬 변수: `UartTxChunk chunk`, `uint16_t offset`, `uint16_t chunk_length`, `InfraStatus status`
- 접근 상태/필드: `service->tx`, `chunk.data`, `chunk.length`, `tx.queue`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `memcpy`, `InfraQueue_Push`, `UartService_SetError`

#### `InfraStatus UartService_Init(UartService *service)`

- 위치: `services/uart_service.c:241`
- 역할: UART service와 저수준 하드웨어 바인딩을 초기화한다. 먼저 소프트웨어 버퍼를 reset해 두어, 하드웨어 실패가 나도 service가 정의된 진단 상태를 유지하게 한다.
- 파라미터: `UartService *service`
- 로컬 변수: `UartHwStatus status`
- 접근 상태/필드: `service->error_flag`, `service->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_Reset`, `UartHw_InitDefault`, `UartService_SetError`

#### `InfraStatus UartService_Recover(UartService *service)`

- 위치: `services/uart_service.c:272`
- 역할: 치명적인 UART 오류 이후 service를 다시 초기화한다. console은 전체 펌웨어 reset 대신, 수동 `recover` 명령에서 이 경로를 사용한다.
- 파라미터: `UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_Init`

#### `void UartService_ProcessRx(UartService *service)`

- 위치: `services/uart_service.c:282`
- 역할: pending RX 바이트를 비우며 다음 완성 line을 만든다. ISR 작업량은 작게 유지하면서도, line parsing은 더 명확한 일반 scheduler context에서 수행되게 한다.
- 파라미터: `UartService *service`
- 로컬 변수: `uint8_t rx_byte`
- 접근 상태/필드: `service->initialized`, `service->rx_pending`, `service->rx_line`, `rx_pending.overflow`, `rx_line.overflow`, `rx_line.line_ready`
- 사용 전역/static: 없음
- 직접 호출 함수: `UartService_ResetRx`, `UartService_SetError`, `UartService_PopPendingByte`, `UartService_OnRxByte`

#### `void UartService_ProcessTx(UartService *service, uint32_t now_ms)`

- 위치: `services/uart_service.c:321`
- 역할: 현재 transmit 작업을 진행시키거나 다음 작업을 시작한다. timeout 감지도 여기서 수행하여, scheduler를 막지 않고 stalled driver transfer를 표시할 수 있게 한다.
- 파라미터: `UartService *service`, `uint32_t now_ms`
- 로컬 변수: `InfraStatus status`, `UartTxChunk chunk`, `uint32_t bytes_remaining`, `UartHwStatus hw_status`
- 접근 상태/필드: `service->initialized`, `service->tx`, `tx.busy`, `tx.queue`, `chunk.length`, `tx.current_buffer`, `chunk.data`, `tx.current_length`, `tx.start_ms`, `tx.timeout_ms`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_Pop`, `UartService_SetError`, `memcpy`, `UartHw_StartTransmit`, `Infra_TimeIsExpired`, `UartHw_GetTransmitStatus`

#### `InfraStatus UartService_RequestTx(UartService *service, const char *text)`

- 위치: `services/uart_service.c:410`
- 역할: NUL 종료 문자열 하나를 전송 queue에 올린다. 상위 console 계층은 모든 terminal 갱신에서, 일반 렌더링과 메시지 출력 경로로 이 함수를 사용한다.
- 파라미터: `UartService *service`, `const char *text`
- 로컬 변수: `uint16_t length`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `strlen`, `UartService_RequestTxBytes`

#### `uint8_t UartService_HasLine(const UartService *service)`

- 위치: `services/uart_service.c:428`
- 역할: `UartService_HasLine`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->rx_line`, `rx_line.line_ready`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus UartService_ReadLine(UartService *service, char *out_buffer, uint16_t max_length)`

- 위치: `services/uart_service.c:443`
- 역할: 현재 완성된 line을 caller 버퍼로 복사한다. line을 읽어가면 RX 조립 상태도 함께 reset되어, 다음 operator 명령이 빈 buffer에서 시작된다.
- 파라미터: `UartService *service`, `char *out_buffer`, `uint16_t max_length`
- 로컬 변수: `uint16_t copy_length`
- 접근 상태/필드: `service->rx_line`, `rx_line.line_ready`, `rx_line.length`, `rx_line.buffer`
- 사용 전역/static: 없음
- 직접 호출 함수: `memcpy`, `UartService_ResetRx`

#### `InfraStatus UartService_GetCurrentInputText(const UartService *service, char *out_buffer, uint16_t max_length)`

- 위치: `services/uart_service.c:473`
- 역할: `UartService_GetCurrentInputText`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`, `char *out_buffer`, `uint16_t max_length`
- 로컬 변수: `uint16_t copy_length`
- 접근 상태/필드: `service->rx_line`, `rx_line.length`, `rx_line.buffer`
- 사용 전역/static: 없음
- 직접 호출 함수: `memcpy`

#### `uint16_t UartService_GetCurrentInputLength(const UartService *service)`

- 위치: `services/uart_service.c:499`
- 역할: `UartService_GetCurrentInputLength`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->rx_line`, `rx_line.length`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint16_t UartService_GetTxQueueCount(const UartService *service)`

- 위치: `services/uart_service.c:509`
- 역할: `UartService_GetTxQueueCount`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->tx`, `tx.queue`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_GetCount`

#### `uint16_t UartService_GetTxQueueCapacity(const UartService *service)`

- 위치: `services/uart_service.c:519`
- 역할: `UartService_GetTxQueueCapacity`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->tx`, `tx.queue`
- 사용 전역/static: 없음
- 직접 호출 함수: `InfraQueue_GetCapacity`

#### `uint8_t UartService_IsTxBusy(const UartService *service)`

- 위치: `services/uart_service.c:529`
- 역할: `UartService_IsTxBusy`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->tx`, `tx.busy`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t UartService_HasError(const UartService *service)`

- 위치: `services/uart_service.c:539`
- 역할: `UartService_HasError`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->error_flag`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `UartErrorCode UartService_GetErrorCode(const UartService *service)`

- 위치: `services/uart_service.c:549`
- 역할: `UartService_GetErrorCode`는 저수준 하드웨어 계층 위에서 동작하는 UART service 구현부다. 문자를 line으로 조립하고 출력 텍스트를 chunk로 나누며, 전송 오류를 모듈 친화적인 형태로 추적한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const UartService *service`
- 로컬 변수: 없음
- 접근 상태/필드: `service->error_code`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `services/adc_module.h`

- 대표 프로젝트: `S32K_Lin_slave`
- 사용 프로젝트: `S32K_Lin_slave`
- 모듈 역할: ADC 샘플링과 zone 해석을 위한 공개 인터페이스다. app 계층은 보드 callback과 threshold, latch 관리 대신 하나의 의미 기반 센서 모듈만 보게 된다.

자료구조/열거형

#### `AdcZone` (enum)

- `ADC_ZONE_SAFE = 0`
- `ADC_ZONE_WARNING`
- `ADC_ZONE_DANGER`
- `ADC_ZONE_EMERGENCY`

#### `AdcConfig` (struct)

portable 모듈에 전달되는 보드 전용 ADC 설정 구조체다. Runtime IO가 callback과 threshold를 채워 넣어, ADC 모듈이 generated peripheral symbol과 분리되도록 만든다.

- `AdcHwInitFn init_fn`
- `AdcHwSampleFn sample_fn`
- `void *hw_context`
- `uint32_t sample_period_ms`
- `uint16_t range_max`
- `uint16_t safe_max`
- `uint16_t warning_max`
- `uint16_t emergency_min`
- `uint8_t blocking_mode`

#### `AdcSnapshot` (struct)

application 계층이 보는 최신 해석 ADC sample이다. 이 snapshot은 raw 값과 semantic zone, latch 상태, 오류 플래그를 하나의 구조체에 묶어 쉽게 게시할 수 있게 한다.

- `uint16_t raw_value`
- `uint8_t zone`
- `uint8_t emergency_latched`
- `uint8_t has_sample`
- `uint8_t sample_error`

#### `AdcModule` (struct)

하나의 펌웨어 이미지가 가지는 전체 ADC 모듈 상태다. 설정과 최신 snapshot, 그리고 주기 ADC task가 사용하는 sample timestamp를 함께 보관한다.

- `uint8_t initialized`
- `AdcConfig config`
- `AdcSnapshot snapshot`
- `uint32_t last_sample_ms`

### `services/adc_module.c`

- 대표 프로젝트: `S32K_Lin_slave`
- 사용 프로젝트: `S32K_Lin_slave`
- 모듈 역할: ADC 샘플링과 zone 분류 구현부다. 이 모듈은 raw sample을 해석된 상태로 바꾸고, 승인 절차에서 사용하는 emergency latch를 유지한다.

#### `static uint8_t AdcModule_ClassifyZone(const AdcConfig *config, uint16_t raw_value)`

- 위치: `services/adc_module.c:16`
- 역할: raw ADC sample 하나를 semantic zone으로 매핑한다. threshold 비교 순서를 여기 모아두어, task 경로는 sample과 latch 유지에 집중할 수 있게 한다.
- 파라미터: `const AdcConfig *config`, `uint16_t raw_value`
- 로컬 변수: 없음
- 접근 상태/필드: `config->safe_max`, `config->warning_max`, `config->emergency_min`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus AdcModule_Init(AdcModule *module, const AdcConfig *config)`

- 위치: `services/adc_module.c:41`
- 역할: 보드 callback과 threshold로 ADC 모듈을 초기화한다. 먼저 설정을 검증한 뒤, 필요한 하드웨어 준비를 platform binding에 요청한다.
- 파라미터: `AdcModule *module`, `const AdcConfig *config`
- 로컬 변수: `InfraStatus status`
- 접근 상태/필드: `module->config`, `module->initialized`, `config.sample_fn`, `config.range_max`, `config.init_fn`, `config.hw_context`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `init_fn`

#### `void AdcModule_Task(AdcModule *module, uint32_t now_ms)`

- 위치: `services/adc_module.c:76`
- 역할: 주기적으로 ADC를 샘플링하고 현재 snapshot을 갱신한다. emergency latch 동작을 여기서 강제하여, 상위 계층이 센서 상태를 항상 같은 방식으로 해석하도록 만든다.
- 파라미터: `AdcModule *module`, `uint32_t now_ms`
- 로컬 변수: `InfraStatus status`, `uint16_t raw_value`, `uint16_t range_ceiling`
- 접근 상태/필드: `module->initialized`, `module->last_sample_ms`, `module->config`, `module->snapshot`, `config.sample_period_ms`, `config.sample_fn`, `config.hw_context`, `snapshot.sample_error`, `config.range_max`, `snapshot.raw_value`, `snapshot.zone`, `snapshot.has_sample`, `snapshot.emergency_latched`
- 사용 전역/static: 없음
- 직접 호출 함수: `Infra_TimeIsDue`, `sample_fn`, `AdcModule_ClassifyZone`

#### `InfraStatus AdcModule_GetSnapshot(const AdcModule *module, AdcSnapshot *out_snapshot)`

- 위치: `services/adc_module.c:118`
- 역할: `AdcModule_GetSnapshot`는 ADC 샘플링과 zone 분류 구현부다. 이 모듈은 raw sample을 해석된 상태로 바꾸고, 승인 절차에서 사용하는 emergency latch를 유지한다. 맥락에서 동작하는 helper/API다.
- 파라미터: `const AdcModule *module`, `AdcSnapshot *out_snapshot`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->snapshot`, `snapshot.has_sample`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `InfraStatus AdcModule_ClearEmergencyLatch(AdcModule *module)`

- 위치: `services/adc_module.c:134`
- 역할: 승인 이후 emergency latch를 지우려고 시도한다. 최신 zone이 더 이상 emergency가 아닐 때만 latch를 지워, 너무 이른 복구를 막는다.
- 파라미터: `AdcModule *module`
- 로컬 변수: 없음
- 접근 상태/필드: `module->initialized`, `module->snapshot`, `snapshot.has_sample`, `snapshot.zone`, `snapshot.emergency_latched`
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

## Platform SDK

### `platform/s32k_sdk/isosdk_adc.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

자료구조/열거형

#### `IsoSdkAdcContext` (struct)

- `uint8_t initialized`

### `platform/s32k_sdk/isosdk_adc.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

파일 전역 상태

- `s_iso_sdk_adc_chan_config`

#### `uint8_t IsoSdk_AdcIsSupported(void)`

- 위치: `platform/s32k_sdk/isosdk_adc.c:14`
- 역할: ADC 기능이 현재 빌드/보드에서 지원되는지 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_AdcInit(IsoSdkAdcContext *context)`

- 위치: `platform/s32k_sdk/isosdk_adc.c:19`
- 역할: ADC 드라이버 컨텍스트를 초기화한다.
- 파라미터: `IsoSdkAdcContext *context`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`, `s_iso_sdk_adc_chan_config.interruptEnable`
- 사용 전역/static: `s_iso_sdk_adc_chan_config`
- 직접 호출 함수: `memset`, `ADC_DRV_ConfigConverter`, `ADC_DRV_AutoCalibration`

#### `uint8_t IsoSdk_AdcSample(IsoSdkAdcContext *context, uint16_t *out_raw_value)`

- 위치: `platform/s32k_sdk/isosdk_adc.c:36`
- 역할: ADC 한 번 샘플링해서 raw 값을 돌려준다.
- 파라미터: `IsoSdkAdcContext *context`, `uint16_t *out_raw_value`
- 로컬 변수: `uint16_t raw_value`
- 접근 상태/필드: `context->initialized`
- 사용 전역/static: `s_iso_sdk_adc_chan_config`
- 직접 호출 함수: `ADC_DRV_ConfigChan`, `ADC_DRV_WaitConvDone`, `ADC_DRV_GetChanResult`

#### `uint8_t IsoSdk_AdcIsSupported(void)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_adc.c:61`
- 역할: ADC 기능이 현재 빌드/보드에서 지원되는지 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_AdcInit(IsoSdkAdcContext *context)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_adc.c:66`
- 역할: ADC 드라이버 컨텍스트를 초기화한다.
- 파라미터: `IsoSdkAdcContext *context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_AdcSample(IsoSdkAdcContext *context, uint16_t *out_raw_value)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_adc.c:72`
- 역할: ADC 한 번 샘플링해서 raw 값을 돌려준다.
- 파라미터: `IsoSdkAdcContext *context`, `uint16_t *out_raw_value`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `platform/s32k_sdk/isosdk_board.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

### `platform/s32k_sdk/isosdk_board.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

#### `uint8_t IsoSdk_BoardInit(void)`

- 위치: `platform/s32k_sdk/isosdk_board.c:10`
- 역할: 보드 핀과 기본 peripheral을 초기화한다.
- 파라미터: 없음
- 로컬 변수: `status_t status`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `CLOCK_SYS_Init`, `CLOCK_SYS_UpdateConfiguration`, `PINS_DRV_Init`

#### `void IsoSdk_BoardEnableLinTransceiver(void)`

- 위치: `platform/s32k_sdk/isosdk_board.c:33`
- 역할: LIN 트랜시버 enable 핀을 활성화한다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `PINS_DRV_SetPinsDirection`, `PINS_DRV_SetPins`

#### `uint32_t IsoSdk_BoardGetRgbLedRedPin(void)`

- 위치: `platform/s32k_sdk/isosdk_board.c:48`
- 역할: 빨간 LED pin 번호를 돌려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint32_t IsoSdk_BoardGetRgbLedGreenPin(void)`

- 위치: `platform/s32k_sdk/isosdk_board.c:53`
- 역할: 초록 LED pin 번호를 돌려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_BoardGetRgbLedActiveOnLevel(void)`

- 위치: `platform/s32k_sdk/isosdk_board.c:58`
- 역할: LED active polarity를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_BoardReadSlave1ButtonPressed(void)`

- 위치: `platform/s32k_sdk/isosdk_board.c:63`
- 역할: 로컬 버튼이 눌렸는지 읽는다.
- 파라미터: 없음
- 로컬 변수: `GPIO_Type *gpio_port`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `PINS_DRV_ReadPins`

#### `void IsoSdk_GpioWritePin(void *gpio_port, uint32_t pin, uint8_t level)`

- 위치: `platform/s32k_sdk/isosdk_board.c:71`
- 역할: 단일 GPIO pin 출력 레벨을 쓴다.
- 파라미터: `void *gpio_port`, `uint32_t pin`, `uint8_t level`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `PINS_DRV_WritePin`

#### `void IsoSdk_GpioSetPinsDirectionMask(void *gpio_port, uint32_t pin_mask)`

- 위치: `platform/s32k_sdk/isosdk_board.c:81`
- 역할: GPIO pin direction mask를 출력으로 설정한다.
- 파라미터: `void *gpio_port`, `uint32_t pin_mask`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `PINS_DRV_SetPinsDirection`

### `platform/s32k_sdk/isosdk_board_profile.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: 보드 배선과 active level은 이 파일 하나에서만 관리한다. 이후 보드가 바뀌면 상위 runtime이나 isosdk_board 구현 대신 이 프로파일 매핑만 수정하는 것을 기본 원칙으로 삼는다.

상수/매크로

- `ISOSDK_BOARD_PROFILE_SLAVE1_BUTTON_PORT` = `((void *)PTC)`
- `ISOSDK_BOARD_PROFILE_SLAVE1_BUTTON_PIN` = `12U`
- `ISOSDK_BOARD_PROFILE_SLAVE1_BUTTON_MASK` = `(1UL << ISOSDK_BOARD_PROFILE_SLAVE1_BUTTON_PIN)`
- `ISOSDK_BOARD_PROFILE_RGB_LED_PORT` = `((void *)PTD)`
- `ISOSDK_BOARD_PROFILE_RGB_LED_RED_PIN` = `15U`
- `ISOSDK_BOARD_PROFILE_RGB_LED_GREEN_PIN` = `16U`
- `ISOSDK_BOARD_PROFILE_RGB_LED_ACTIVE_ON_LEVEL` = `0U`
- `ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_PORT` = `PTE`
- `ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_PIN` = `9U`
- `ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_MASK` = `(1UL << ISOSDK_BOARD_PROFILE_LIN_XCVR_ENABLE_PIN)`

### `platform/s32k_sdk/isosdk_can.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

자료구조/열거형

#### `IsoSdkCanTransferState` (enum)

- `ISOSDK_CAN_TRANSFER_DONE = 0`
- `ISOSDK_CAN_TRANSFER_BUSY`
- `ISOSDK_CAN_TRANSFER_ERROR`

#### `IsoSdkCanEventId` (enum)

- `ISOSDK_CAN_EVENT_NONE = 0`
- `ISOSDK_CAN_EVENT_RX_DONE`
- `ISOSDK_CAN_EVENT_TX_DONE`
- `ISOSDK_CAN_EVENT_ERROR`

### `platform/s32k_sdk/isosdk_can.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

파일 전역 상태

- `s_iso_sdk_can_rx_msg`
- `s_iso_sdk_can_event_cb`

#### `static void IsoSdk_CanDispatchEvent(uint8_t event_id, uint8_t mb_index)`

- 위치: `platform/s32k_sdk/isosdk_can.c:18`
- 역할: `IsoSdk_CanDispatchEvent`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint8_t event_id`, `uint8_t mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `s_iso_sdk_can_event_cb`
- 직접 호출 함수: `s_iso_sdk_can_event_cb`

#### `static void IsoSdk_CanSdkEventCallback(uint8_t instance, flexcan_event_type_t eventType, uint32_t buffIdx, flexcan_state_t *flexcanState)`

- 위치: `platform/s32k_sdk/isosdk_can.c:28`
- 역할: `IsoSdk_CanSdkEventCallback`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint8_t instance`, `flexcan_event_type_t eventType`, `uint32_t buffIdx`, `flexcan_state_t *flexcanState`
- 로컬 변수: `uint8_t event_id`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanDispatchEvent`

#### `static void IsoSdk_CanSdkErrorCallback(uint8_t instance, flexcan_event_type_t eventType, flexcan_state_t *flexcanState)`

- 위치: `platform/s32k_sdk/isosdk_can.c:59`
- 역할: `IsoSdk_CanSdkErrorCallback`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint8_t instance`, `flexcan_event_type_t eventType`, `flexcan_state_t *flexcanState`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanDispatchEvent`

#### `static void IsoSdk_CanInitDataInfo(flexcan_data_info_t *data_info, uint8_t dlc, uint8_t is_extended_id, uint8_t is_remote_frame)`

- 위치: `platform/s32k_sdk/isosdk_can.c:69`
- 역할: `IsoSdk_CanInitDataInfo`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `flexcan_data_info_t *data_info`, `uint8_t dlc`, `uint8_t is_extended_id`, `uint8_t is_remote_frame`
- 로컬 변수: 없음
- 접근 상태/필드: `data_info->msg_id_type`, `data_info->is_remote`, `data_info->data_length`, `data_info->fd_enable`, `data_info->enable_brs`, `data_info->fd_padding`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`

#### `uint8_t IsoSdk_CanIsSupported(void)`

- 위치: `platform/s32k_sdk/isosdk_can.c:88`
- 역할: CAN 하드웨어 지원 여부를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanGetDefaultInstance(void)`

- 위치: `platform/s32k_sdk/isosdk_can.c:93`
- 역할: 기본 CAN controller instance 번호를 돌려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanInitController(uint8_t instance)`

- 위치: `platform/s32k_sdk/isosdk_can.c:98`
- 역할: CAN controller를 초기화한다.
- 파라미터: `uint8_t instance`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `FLEXCAN_DRV_Init`

#### `void IsoSdk_CanInstallEventCallback(IsoSdkCanEventCallback event_cb, void *event_context)`

- 위치: `platform/s32k_sdk/isosdk_can.c:105`
- 역할: CAN SDK event callback과 context를 등록한다.
- 파라미터: `IsoSdkCanEventCallback event_cb`, `void *event_context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `s_iso_sdk_can_event_cb`
- 직접 호출 함수: `FLEXCAN_DRV_InstallEventCallback`, `FLEXCAN_DRV_InstallErrorCallback`

#### `uint8_t IsoSdk_CanInitTxMailbox(uint8_t instance, uint8_t tx_mb_index)`

- 위치: `platform/s32k_sdk/isosdk_can.c:119`
- 역할: TX mailbox를 초기화한다.
- 파라미터: `uint8_t instance`, `uint8_t tx_mb_index`
- 로컬 변수: `flexcan_data_info_t data_info`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanInitDataInfo`, `FLEXCAN_DRV_ConfigTxMb`

#### `uint8_t IsoSdk_CanInitRxMailbox(uint8_t instance, uint8_t rx_mb_index)`

- 위치: `platform/s32k_sdk/isosdk_can.c:127`
- 역할: RX mailbox를 초기화한다.
- 파라미터: `uint8_t instance`, `uint8_t rx_mb_index`
- 로컬 변수: `flexcan_data_info_t data_info`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanInitDataInfo`, `FLEXCAN_DRV_ConfigRxMb`

#### `uint8_t IsoSdk_CanConfigRxAcceptAll(uint8_t instance, uint8_t rx_mb_index)`

- 위치: `platform/s32k_sdk/isosdk_can.c:135`
- 역할: RX mailbox가 모든 ID를 받도록 설정한다.
- 파라미터: `uint8_t instance`, `uint8_t rx_mb_index`
- 로컬 변수: `status_t status`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `FLEXCAN_DRV_SetRxMaskType`, `FLEXCAN_DRV_SetRxIndividualMask`

#### `uint8_t IsoSdk_CanStartReceive(uint8_t instance, uint8_t rx_mb_index)`

- 위치: `platform/s32k_sdk/isosdk_can.c:147`
- 역할: 지정 RX mailbox에서 비동기 수신을 시작한다.
- 파라미터: `uint8_t instance`, `uint8_t rx_mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `s_iso_sdk_can_rx_msg`
- 직접 호출 함수: `memset`, `FLEXCAN_DRV_Receive`

#### `IsoSdkCanTransferState IsoSdk_CanGetTransferState(uint8_t instance, uint8_t mb_index)`

- 위치: `platform/s32k_sdk/isosdk_can.c:153`
- 역할: TX/RX mailbox의 전송 상태를 조회한다.
- 파라미터: `uint8_t instance`, `uint8_t mb_index`
- 로컬 변수: `status_t status`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `FLEXCAN_DRV_GetTransferStatus`

#### `uint8_t IsoSdk_CanReadRxFrame(uint32_t now_ms, uint32_t *out_id, uint8_t *out_dlc, uint8_t *out_is_extended_id, uint8_t *out_is_remote_frame, uint8_t *out_data, uint8_t data_capacity)`

- 위치: `platform/s32k_sdk/isosdk_can.c:171`
- 역할: 최근 수신된 raw CAN frame을 읽어 out 파라미터에 복사한다.
- 파라미터: `uint32_t now_ms`, `uint32_t *out_id`, `uint8_t *out_dlc`, `uint8_t *out_is_extended_id`, `uint8_t *out_is_remote_frame`, `uint8_t *out_data`, `uint8_t data_capacity`
- 로컬 변수: `uint8_t dlc`
- 접근 상태/필드: `s_iso_sdk_can_rx_msg.dataLen`, `s_iso_sdk_can_rx_msg.msgId`, `s_iso_sdk_can_rx_msg.data`
- 사용 전역/static: `s_iso_sdk_can_rx_msg`
- 직접 호출 함수: `memcpy`

#### `uint8_t IsoSdk_CanSend(uint8_t instance, uint8_t tx_mb_index, uint32_t id, uint8_t dlc, const uint8_t *data, uint8_t is_extended_id, uint8_t is_remote_frame)`

- 위치: `platform/s32k_sdk/isosdk_can.c:203`
- 역할: raw CAN frame을 실제 controller로 전송한다.
- 파라미터: `uint8_t instance`, `uint8_t tx_mb_index`, `uint32_t id`, `uint8_t dlc`, `const uint8_t *data`, `uint8_t is_extended_id`, `uint8_t is_remote_frame`
- 로컬 변수: `flexcan_data_info_t data_info`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_CanInitDataInfo`, `FLEXCAN_DRV_Send`

#### `uint8_t IsoSdk_CanIsSupported(void)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:228`
- 역할: CAN 하드웨어 지원 여부를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanGetDefaultInstance(void)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:233`
- 역할: 기본 CAN controller instance 번호를 돌려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanInitController(uint8_t instance)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:238`
- 역할: CAN controller를 초기화한다.
- 파라미터: `uint8_t instance`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void IsoSdk_CanInstallEventCallback(IsoSdkCanEventCallback event_cb, void *event_context)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:244`
- 역할: CAN SDK event callback과 context를 등록한다.
- 파라미터: `IsoSdkCanEventCallback event_cb`, `void *event_context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanInitTxMailbox(uint8_t instance, uint8_t tx_mb_index)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:251`
- 역할: TX mailbox를 초기화한다.
- 파라미터: `uint8_t instance`, `uint8_t tx_mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanInitRxMailbox(uint8_t instance, uint8_t rx_mb_index)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:258`
- 역할: RX mailbox를 초기화한다.
- 파라미터: `uint8_t instance`, `uint8_t rx_mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanConfigRxAcceptAll(uint8_t instance, uint8_t rx_mb_index)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:265`
- 역할: RX mailbox가 모든 ID를 받도록 설정한다.
- 파라미터: `uint8_t instance`, `uint8_t rx_mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanStartReceive(uint8_t instance, uint8_t rx_mb_index)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:272`
- 역할: 지정 RX mailbox에서 비동기 수신을 시작한다.
- 파라미터: `uint8_t instance`, `uint8_t rx_mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `IsoSdkCanTransferState IsoSdk_CanGetTransferState(uint8_t instance, uint8_t mb_index)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:279`
- 역할: TX/RX mailbox의 전송 상태를 조회한다.
- 파라미터: `uint8_t instance`, `uint8_t mb_index`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanReadRxFrame(uint32_t now_ms, uint32_t *out_id, uint8_t *out_dlc, uint8_t *out_is_extended_id, uint8_t *out_is_remote_frame, uint8_t *out_data, uint8_t data_capacity)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:286`
- 역할: 최근 수신된 raw CAN frame을 읽어 out 파라미터에 복사한다.
- 파라미터: `uint32_t now_ms`, `uint32_t *out_id`, `uint8_t *out_dlc`, `uint8_t *out_is_extended_id`, `uint8_t *out_is_remote_frame`, `uint8_t *out_data`, `uint8_t data_capacity`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_CanSend(uint8_t instance, uint8_t tx_mb_index, uint32_t id, uint8_t dlc, const uint8_t *data, uint8_t is_extended_id, uint8_t is_remote_frame)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_can.c:304`
- 역할: raw CAN frame을 실제 controller로 전송한다.
- 파라미터: `uint8_t instance`, `uint8_t tx_mb_index`, `uint32_t id`, `uint8_t dlc`, `const uint8_t *data`, `uint8_t is_extended_id`, `uint8_t is_remote_frame`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `platform/s32k_sdk/isosdk_lin.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

자료구조/열거형

#### `IsoSdkLinRole` (enum)

- `ISOSDK_LIN_ROLE_MASTER = 1`
- `ISOSDK_LIN_ROLE_SLAVE = 2`

#### `IsoSdkLinEventId` (enum)

- `ISOSDK_LIN_EVENT_NONE = 0`
- `ISOSDK_LIN_EVENT_PID_OK`
- `ISOSDK_LIN_EVENT_RX_DONE`
- `ISOSDK_LIN_EVENT_TX_DONE`
- `ISOSDK_LIN_EVENT_ERROR`

#### `IsoSdkLinContext` (struct)

- `uint8_t initialized`
- `uint16_t timeout_ticks`
- `uint8_t role`
- `IsoSdkLinEventCallback event_cb`
- `void *event_context`

### `platform/s32k_sdk/isosdk_lin.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

#### `static void IsoSdk_LinDispatchEvent(uint8_t event_id, uint8_t current_pid)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:13`
- 역할: `IsoSdk_LinDispatchEvent`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint8_t event_id`, `uint8_t current_pid`
- 로컬 변수: 없음
- 접근 상태/필드: `s_iso_sdk_lin_context->event_cb`, `s_iso_sdk_lin_context->event_context`
- 사용 전역/static: 없음
- 직접 호출 함수: `event_cb`

#### `static void IsoSdk_LinSdkCallback(uint32_t instance, void *lin_state_ptr)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:26`
- 역할: `IsoSdk_LinSdkCallback`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `uint32_t instance`, `void *lin_state_ptr`
- 로컬 변수: `lin_state_t *lin_state`, `uint8_t event_id`, `uint8_t current_pid`
- 접근 상태/필드: `lin_state->currentId`, `lin_state->timeoutCounterFlag`, `lin_state->currentEventId`, `s_iso_sdk_lin_context->initialized`, `s_iso_sdk_lin_context->timeout_ticks`
- 사용 전역/static: 없음
- 직접 호출 함수: `IsoSdk_LinDispatchEvent`, `LIN_DRV_SetTimeoutCounter`

#### `uint8_t IsoSdk_LinIsSupported(void)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:92`
- 역할: LIN 하드웨어 지원 여부를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_LinInit(IsoSdkLinContext *context, uint8_t role, uint16_t timeout_ticks, IsoSdkLinEventCallback event_cb, void *event_context)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:97`
- 역할: LIN driver 컨텍스트와 callback을 초기화한다.
- 파라미터: `IsoSdkLinContext *context`, `uint8_t role`, `uint16_t timeout_ticks`, `IsoSdkLinEventCallback event_cb`, `void *event_context`
- 로컬 변수: `status_t status`
- 접근 상태/필드: `context->timeout_ticks`, `context->role`, `context->event_cb`, `context->event_context`, `context->initialized`, `ISOSDK_SDK_LIN_INIT_CONFIG.nodeFunction`, `ISOSDK_SDK_LIN_INIT_CONFIG.autobaudEnable`
- 사용 전역/static: 없음
- 직접 호출 함수: `memset`, `LIN_DRV_Init`, `LIN_DRV_InstallCallback`

#### `uint8_t IsoSdk_LinMasterSendHeader(IsoSdkLinContext *context, uint8_t pid)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:134`
- 역할: master가 PID header를 전송하게 한다.
- 파라미터: `IsoSdkLinContext *context`, `uint8_t pid`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `LIN_DRV_MasterSendHeader`

#### `uint8_t IsoSdk_LinStartReceive(IsoSdkLinContext *context, uint8_t *buffer, uint8_t length)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:144`
- 역할: LIN response 수신을 시작한다.
- 파라미터: `IsoSdkLinContext *context`, `uint8_t *buffer`, `uint8_t length`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `LIN_DRV_ReceiveFrameData`

#### `uint8_t IsoSdk_LinStartSend(IsoSdkLinContext *context, const uint8_t *buffer, uint8_t length)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:155`
- 역할: LIN response 송신을 시작한다.
- 파라미터: `IsoSdkLinContext *context`, `const uint8_t *buffer`, `uint8_t length`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `LIN_DRV_SendFrameData`

#### `void IsoSdk_LinGotoIdle(IsoSdkLinContext *context)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:166`
- 역할: LIN peripheral을 idle 상태로 되돌린다.
- 파라미터: `IsoSdkLinContext *context`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `LIN_DRV_GotoIdleState`

#### `void IsoSdk_LinSetTimeout(IsoSdkLinContext *context, uint16_t timeout_ticks)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:176`
- 역할: LIN timeout tick 값을 바꾼다.
- 파라미터: `IsoSdkLinContext *context`, `uint16_t timeout_ticks`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`, `context->timeout_ticks`
- 사용 전역/static: 없음
- 직접 호출 함수: `LIN_DRV_SetTimeoutCounter`

#### `void IsoSdk_LinServiceTick(IsoSdkLinContext *context)`

- 위치: `platform/s32k_sdk/isosdk_lin.c:187`
- 역할: base tick마다 LIN timeout/service bookkeeping을 진행한다.
- 파라미터: `IsoSdkLinContext *context`
- 로컬 변수: 없음
- 접근 상태/필드: `context->initialized`
- 사용 전역/static: 없음
- 직접 호출 함수: `LIN_DRV_TimeoutService`

#### `uint8_t IsoSdk_LinIsSupported(void)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:199`
- 역할: LIN 하드웨어 지원 여부를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_LinInit(IsoSdkLinContext *context, uint8_t role, uint16_t timeout_ticks, IsoSdkLinEventCallback event_cb, void *event_context)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:204`
- 역할: LIN driver 컨텍스트와 callback을 초기화한다.
- 파라미터: `IsoSdkLinContext *context`, `uint8_t role`, `uint16_t timeout_ticks`, `IsoSdkLinEventCallback event_cb`, `void *event_context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_LinMasterSendHeader(IsoSdkLinContext *context, uint8_t pid)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:218`
- 역할: master가 PID header를 전송하게 한다.
- 파라미터: `IsoSdkLinContext *context`, `uint8_t pid`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_LinStartReceive(IsoSdkLinContext *context, uint8_t *buffer, uint8_t length)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:225`
- 역할: LIN response 수신을 시작한다.
- 파라미터: `IsoSdkLinContext *context`, `uint8_t *buffer`, `uint8_t length`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_LinStartSend(IsoSdkLinContext *context, const uint8_t *buffer, uint8_t length)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:233`
- 역할: LIN response 송신을 시작한다.
- 파라미터: `IsoSdkLinContext *context`, `const uint8_t *buffer`, `uint8_t length`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void IsoSdk_LinGotoIdle(IsoSdkLinContext *context)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:241`
- 역할: LIN peripheral을 idle 상태로 되돌린다.
- 파라미터: `IsoSdkLinContext *context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void IsoSdk_LinSetTimeout(IsoSdkLinContext *context, uint16_t timeout_ticks)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:246`
- 역할: LIN timeout tick 값을 바꾼다.
- 파라미터: `IsoSdkLinContext *context`, `uint16_t timeout_ticks`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `void IsoSdk_LinServiceTick(IsoSdkLinContext *context)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_lin.c:252`
- 역할: base tick마다 LIN timeout/service bookkeeping을 진행한다.
- 파라미터: `IsoSdkLinContext *context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `platform/s32k_sdk/isosdk_tick.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

### `platform/s32k_sdk/isosdk_tick.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

#### `uint8_t IsoSdk_TickInit(IsoSdkTickHandler handler)`

- 위치: `platform/s32k_sdk/isosdk_tick.c:9`
- 역할: LPTMR 기반 tick 인터럽트를 초기화한다.
- 파라미터: `IsoSdkTickHandler handler`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LPTMR_DRV_Init`, `INT_SYS_InstallHandler`, `INT_SYS_EnableIRQ`, `LPTMR_DRV_StartCounter`

#### `void IsoSdk_TickClearCompareFlag(void)`

- 위치: `platform/s32k_sdk/isosdk_tick.c:23`
- 역할: tick compare flag를 클리어한다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LPTMR_DRV_ClearCompareFlag`

### `platform/s32k_sdk/isosdk_uart.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

자료구조/열거형

#### `IsoSdkUartEventId` (enum)

- `ISOSDK_UART_EVENT_RX_FULL = 0`
- `ISOSDK_UART_EVENT_ERROR`

#### `IsoSdkUartTxState` (enum)

- `ISOSDK_UART_TX_STATE_DONE = 0`
- `ISOSDK_UART_TX_STATE_BUSY`
- `ISOSDK_UART_TX_STATE_ERROR`

### `platform/s32k_sdk/isosdk_uart.c`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`

파일 전역 상태

- `s_iso_sdk_uart_event_cb`

#### `static void IsoSdk_UartRxCallback(void *driver_state, uart_event_t event, void *user_data)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:14`
- 역할: `IsoSdk_UartRxCallback`의 소스 구현을 기준으로 동작을 정리한 항목이다.
- 파라미터: `void *driver_state`, `uart_event_t event`, `void *user_data`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: `s_iso_sdk_uart_event_cb`
- 직접 호출 함수: `s_iso_sdk_uart_event_cb`

#### `uint8_t IsoSdk_UartIsSupported(void)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:36`
- 역할: UART 하드웨어 지원 여부를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint32_t IsoSdk_UartGetDefaultInstance(void)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:41`
- 역할: 기본 UART instance 번호를 돌려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_UartInit(uint32_t instance, IsoSdkUartEventCallback event_cb, void *event_context)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:46`
- 역할: UART driver와 callback을 초기화한다.
- 파라미터: `uint32_t instance`, `IsoSdkUartEventCallback event_cb`, `void *event_context`
- 로컬 변수: `status_t status`
- 접근 상태/필드: 없음
- 사용 전역/static: `s_iso_sdk_uart_event_cb`
- 직접 호출 함수: `LPUART_DRV_Init`, `LPUART_DRV_InstallRxCallback`

#### `uint8_t IsoSdk_UartStartReceiveByte(uint32_t instance, uint8_t *out_byte)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:67`
- 역할: 1바이트 RX를 시작한다.
- 파라미터: `uint32_t instance`, `uint8_t *out_byte`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LPUART_DRV_ReceiveData`

#### `uint8_t IsoSdk_UartContinueReceiveByte(uint32_t instance, uint8_t *io_byte)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:78`
- 역할: 다음 1바이트 RX를 이어서 시작한다.
- 파라미터: `uint32_t instance`, `uint8_t *io_byte`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LPUART_DRV_SetRxBuffer`

#### `uint8_t IsoSdk_UartStartTransmit(uint32_t instance, const uint8_t *data, uint16_t length)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:89`
- 역할: 버퍼 전송을 시작한다.
- 파라미터: `uint32_t instance`, `const uint8_t *data`, `uint16_t length`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LPUART_DRV_SendData`

#### `IsoSdkUartTxState IsoSdk_UartGetTransmitState(uint32_t instance, uint32_t *bytes_remaining)`

- 위치: `platform/s32k_sdk/isosdk_uart.c:99`
- 역할: 현재 UART TX 상태와 남은 바이트를 조회한다.
- 파라미터: `uint32_t instance`, `uint32_t *bytes_remaining`
- 로컬 변수: `status_t status`, `uint32_t remaining`
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: `LPUART_DRV_GetTransmitStatus`

#### `uint8_t IsoSdk_UartIsSupported(void)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:126`
- 역할: UART 하드웨어 지원 여부를 알려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint32_t IsoSdk_UartGetDefaultInstance(void)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:131`
- 역할: 기본 UART instance 번호를 돌려준다.
- 파라미터: 없음
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_UartInit(uint32_t instance, IsoSdkUartEventCallback event_cb, void *event_context)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:136`
- 역할: UART driver와 callback을 초기화한다.
- 파라미터: `uint32_t instance`, `IsoSdkUartEventCallback event_cb`, `void *event_context`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_UartStartReceiveByte(uint32_t instance, uint8_t *out_byte)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:146`
- 역할: 1바이트 RX를 시작한다.
- 파라미터: `uint32_t instance`, `uint8_t *out_byte`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_UartContinueReceiveByte(uint32_t instance, uint8_t *io_byte)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:153`
- 역할: 다음 1바이트 RX를 이어서 시작한다.
- 파라미터: `uint32_t instance`, `uint8_t *io_byte`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `uint8_t IsoSdk_UartStartTransmit(uint32_t instance, const uint8_t *data, uint16_t length)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:160`
- 역할: 버퍼 전송을 시작한다.
- 파라미터: `uint32_t instance`, `const uint8_t *data`, `uint16_t length`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

#### `IsoSdkUartTxState IsoSdk_UartGetTransmitState(uint32_t instance, uint32_t *bytes_remaining)` (동일 이름의 조건부 대체 구현)

- 위치: `platform/s32k_sdk/isosdk_uart.c:168`
- 역할: 현재 UART TX 상태와 남은 바이트를 조회한다.
- 파라미터: `uint32_t instance`, `uint32_t *bytes_remaining`
- 로컬 변수: 없음
- 접근 상태/필드: 없음
- 사용 전역/static: 없음
- 직접 호출 함수: 없음

### `platform/s32k_sdk/isosdk_sdk_bindings.h`

- 대표 프로젝트: `S32K_Can_slave`
- 사용 프로젝트: `S32K_Can_slave`, `S32K_LinCan_master`, `S32K_Lin_slave`
- 모듈 역할: SDK generated 변수명은 이 파일에서만 직접 참조한다. Config Tools 재생성으로 이름이 바뀌어도 상위 IsoSdk 구현은 이 매핑만 수정하면 유지되도록 의도한 내부 전용 바인딩 헤더다.

상수/매크로

- `ISOSDK_SDK_CLOCK_CONFIGS` = `g_clockManConfigsArr`
- `ISOSDK_SDK_CLOCK_CONFIG_COUNT` = `CLOCK_MANAGER_CONFIG_CNT`
- `ISOSDK_SDK_CLOCK_CALLBACKS` = `g_clockManCallbacksArr`
- `ISOSDK_SDK_CLOCK_CALLBACK_COUNT` = `CLOCK_MANAGER_CALLBACK_CNT`
- `ISOSDK_SDK_PIN_CONFIG_COUNT` = `NUM_OF_CONFIGURED_PINS0`
- `ISOSDK_SDK_PIN_CONFIGS` = `g_pin_mux_InitConfigArr0`
- `ISOSDK_SDK_LPTMR_INSTANCE` = `INST_LPTMR_1`
- `ISOSDK_SDK_LPTMR_CONFIG` = `lptmr_1_config0`
- `ISOSDK_SDK_LPTMR_IRQ` = `LPTMR0_IRQn`
- `ISOSDK_SDK_HAS_CAN` = `1`
- `ISOSDK_SDK_CAN_INSTANCE` = `INST_FLEXCAN_CONFIG_1`
- `ISOSDK_SDK_CAN_STATE` = `flexcanState0`
- `ISOSDK_SDK_CAN_INIT_CONFIG` = `flexcanInitConfig0`
- `ISOSDK_SDK_HAS_LIN` = `1`
- `ISOSDK_SDK_LIN_INSTANCE` = `INST_LIN2`
- `ISOSDK_SDK_LIN_STATE` = `lin2_State`
- `ISOSDK_SDK_LIN_INIT_CONFIG` = `lin2_InitConfig0`
- `ISOSDK_SDK_HAS_ADC` = `1`
- `ISOSDK_SDK_ADC_INSTANCE` = `INST_ADC_CONFIG_1`
- `ISOSDK_SDK_ADC_CONVERTER_CONFIG` = `adc_config_1_ConvConfig0`
- `ISOSDK_SDK_ADC_CHANNEL_CONFIG` = `adc_config_1_ChnConfig0`
- `ISOSDK_SDK_HAS_UART` = `1`
- `ISOSDK_SDK_UART_INSTANCE` = `INST_LPUART_1`
- `ISOSDK_SDK_UART_STATE` = `lpUartState1`
- `ISOSDK_SDK_UART_INIT_CONFIG` = `lpuart_1_InitConfig0`

