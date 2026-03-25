# RE_2 함수 작동방식 상세 문서

## 문서 범위
- 이 문서는 `re_2`의 공통 코드베이스를 기준으로 작성했다.
- 실제 기준 프로젝트는 `S32K_LinCan_master`이며, `S32K_Lin_slave`, `S32K_Can_slave`는 거의 동일한 소스에 `APP_ACTIVE_ROLE`만 다르게 적용한 형태다.
- 따라서 아래 함수 설명은 세 프로젝트 모두에 공통 적용되며, 역할 차이가 있는 함수만 `master/slave1/slave2`로 구분해 설명한다.

## 읽는 방법
- 요청한 형식에 맞춰 `반환형 함수명(인자) { ... }` 형태로 정리했다.
- 실제 C 코드가 아니라, 함수의 내부 처리 순서와 의미를 설명하기 위한 분석용 의사 코드 형식이다.
- 반복 패턴이 같은 단순 setter, clear helper, index helper는 함수군으로 묶어서 설명했다.

## 전체 호출 흐름 요약
1. `main()`이 `Runtime_Init()`을 호출한다.
2. `Runtime_Init()`이 보드, tick, AppCore, task table을 초기화한다.
3. `Runtime_Run()`이 무한 루프에 들어가 `RuntimeTask_RunDue()`를 계속 호출한다.
4. `RuntimeTask_RunDue()`는 `uart -> lin_fast -> button -> can -> adc -> lin_poll -> led -> render -> heartbeat` 순서로 due task를 실행한다.
5. 각 task는 `AppCore_TaskXXX()`로 들어가고, 여기서 역할에 따라 `AppMaster`, `AppSlave1`, `AppSlave2`, `CanModule`, `LinModule`, `AdcModule`, `LedModule`, `AppConsole`로 다시 분기된다.

## 함수 연결 지도
- `main -> Runtime_Init -> AppCore_Init -> 역할별 Init`
- `Runtime_Run -> RuntimeTask_RunDue -> AppCore_TaskCan/Lin/Uart/...`
- `AppCore_TaskCan -> AppConsole 큐 소비 -> CanModule_Task -> CanService_Task -> CanHw/CanProto`
- `AppCore_TaskLinFast/Poll -> LinModule_TaskFast/TaskPoll -> RuntimeIo LIN binding`
- `AppCore_TaskAdc -> AppSlave2_TaskAdc -> AdcModule_Task -> RuntimeIo ADC binding`
- `AppCore_TaskRender -> AppConsole_Render -> UartService_RequestTx -> UartHw_StartTransmit`

---

## 1. 엔트리 / 런타임 레이어

### `int main(void)`
```c
int main(void) {
    // 작동방식
    // 1. Runtime_Init()으로 시스템 전체를 초기화한다.
    // 2. 초기화가 성공하면 Runtime_Run()으로 진입한다.
    // 3. 이후 제어권은 사실상 super-loop에 넘어가며 main은 돌아오지 않는다.
    //
    // 왜 이렇게 동작하는가
    // - 이 프로젝트는 RTOS가 아니라 cooperative super-loop 구조이기 때문이다.
    // - main은 얇게 유지하고, 실제 조립 책임은 runtime 계층으로 모아 유지보수를 쉽게 한다.
    //
    // 의미
    // - main은 "앱 실행 진입점"이지, 정책을 담는 곳이 아니다.
}
```

### `InfraStatus Runtime_Init(void)`
```c
InfraStatus Runtime_Init(void) {
    // 작동방식
    // 1. 전역 runtime 컨텍스트 g_runtime을 미초기화 상태로 되돌린다.
    // 2. RuntimeIo_BoardInit()으로 클록, 핀, LIN 트랜시버 enable 등 보드 자원을 준비한다.
    // 3. RuntimeTick_Init()으로 공통 시간축을 만든다.
    // 4. AppCore_Init()으로 현재 역할(master/slave1/slave2)에 맞는 상위 모듈을 초기화한다.
    // 5. RuntimeTick_RegisterHook(AppCore_OnTickIsr)로 base tick이 LIN 타이밍 서비스까지 전달되게 한다.
    // 6. Runtime_BuildTaskTable()로 주기 task 표를 만든다.
    // 7. RuntimeTask_ResetTable()로 모든 task의 last_run_ms를 같은 시작 시점으로 맞춘다.
    // 8. 정상 완료 시 initialized 플래그와 init_status를 성공 상태로 기록한다.
    //
    // 왜 이렇게 동작하는가
    // - 보드 -> 시간 -> 앱 순으로 올라가야 의존성 순서가 깨지지 않는다.
    // - task table은 마지막에 만들어야, AppCore가 실제로 살아 있는 상태에서 스케줄에 들어갈 수 있다.
    // - tick hook도 LIN 모듈 초기화 이후에 등록해야 ISR이 미완성 객체를 건드리지 않는다.
    //
    // 의미
    // - 이 함수가 끝나면 시스템은 "주기 호출만 하면 동작 가능한 상태"가 된다.
}
```

### `void Runtime_Run(void)`
```c
void Runtime_Run(void) {
    // 작동방식
    // 1. runtime 초기화 성공 여부를 확인한다.
    // 2. 실패 상태면 Runtime_FaultLoop()로 진입해 무한 정지한다.
    // 3. 성공 상태면 무한 루프에서 RuntimeTask_RunDue()를 반복 호출한다.
    //
    // 왜 이렇게 동작하는가
    // - 초기화 실패 후 억지로 루프를 돌리면 하드웨어 미초기화 상태에서 예측 불가능한 동작이 나온다.
    // - cooperative 구조에서는 중앙 루프가 단순할수록 전체 시스템 타이밍을 이해하기 쉽다.
    //
    // 의미
    // - 운영 단계의 실제 심장은 이 함수의 for(;;) 루프다.
}
```

### `const AppCore *Runtime_GetApp(void)`
```c
const AppCore *Runtime_GetApp(void) {
    // 작동방식
    // 1. 전역 runtime 안에 들어 있는 AppCore 주소를 읽기 전용으로 반환한다.
    //
    // 왜 이렇게 동작하는가
    // - 진단이나 외부 점검 코드가 현재 app 상태를 볼 수 있게 하기 위해서다.
    // - scheduler 소유권은 runtime이 유지하고, 외부에는 조회만 허용하려는 의도다.
    //
    // 의미
    // - 시스템 내부 상태의 관찰 창 역할을 한다.
}
```

### `static void Runtime_BuildTaskTable(RuntimeContext *runtime)`
```c
static void Runtime_BuildTaskTable(RuntimeContext *runtime) {
    // 작동방식
    // 1. 9개의 task 엔트리를 고정 순서로 채운다.
    // 2. 각 엔트리에 이름, 주기, adapter 함수, AppCore context를 연결한다.
    // 3. 실제 task 함수는 Runtime_TaskXxx -> AppCore_TaskXxx로 이어진다.
    //
    // 왜 이렇게 동작하는가
    // - task 순서를 코드로 고정해 두면 시스템 전체 흐름을 읽기 쉽다.
    // - runtime은 generic scheduler 인터페이스를 유지하고, AppCore는 역할 중심 함수만 알게 된다.
    //
    // 의미
    // - 시스템 타이밍 구조를 데이터 테이블로 고정하는 함수다.
}
```

### `void RuntimeTask_ResetTable(RuntimeTaskEntry *table, uint32_t task_count, uint32_t start_ms)`
```c
void RuntimeTask_ResetTable(RuntimeTaskEntry *table, uint32_t task_count, uint32_t start_ms) {
    // 작동방식
    // 1. task 배열을 순회한다.
    // 2. 모든 엔트리의 last_run_ms를 같은 start_ms로 맞춘다.
    //
    // 왜 이렇게 동작하는가
    // - 각 task가 제각각 쓰레기 값을 갖지 않게 하기 위해서다.
    // - 모든 주기를 동일한 기준 시점에서 계산하게 하면 초기 동작이 예측 가능해진다.
    //
    // 의미
    // - 스케줄러 시간축의 기준점을 맞추는 함수다.
}
```

### `void RuntimeTask_RunDue(RuntimeTaskEntry *table, uint32_t task_count, uint32_t now_ms)`
```c
void RuntimeTask_RunDue(RuntimeTaskEntry *table, uint32_t task_count, uint32_t now_ms) {
    // 작동방식
    // 1. task 테이블을 앞에서부터 순회한다.
    // 2. task_fn이 없거나 period가 0이면 건너뛴다.
    // 3. Infra_TimeIsDue()로 현재 시각에 실행할 차례인지 판단한다.
    // 4. due 상태면 last_run_ms를 now_ms로 갱신한 뒤 callback을 호출한다.
    //
    // 왜 이렇게 동작하는가
    // - 이 프로젝트는 선점형 스케줄러가 아니라 "도래한 작업만 순서대로 실행"하는 구조이기 때문이다.
    // - last_run_ms = now_ms 방식은 간단하지만, 긴 task가 있으면 드리프트가 생길 수 있다는 특징도 함께 가진다.
    //
    // 의미
    // - super-loop의 실제 작업 분배기다.
}
```

### `InfraStatus RuntimeTick_Init(void)`
```c
InfraStatus RuntimeTick_Init(void) {
    // 작동방식
    // 1. millisecond 카운터, base interrupt 카운터, microsecond 누적기를 0으로 초기화한다.
    // 2. hook 슬롯 배열을 모두 비운다.
    // 3. LPTMR를 500us 기준 인터럽트 소스로 초기화한다.
    // 4. IRQ handler를 설치하고 인터럽트를 enable한 뒤 카운터를 시작한다.
    //
    // 왜 이렇게 동작하는가
    // - 상위 모듈들은 "millisecond 시간"을 원하지만, 실제 하드웨어는 더 짧은 base tick으로 동작하기 때문이다.
    // - hook 배열을 비워두면 어떤 모듈이 ISR 구독을 했는지 명확해진다.
    //
    // 의미
    // - 전체 펌웨어의 공통 시간 기준과 ISR fan-out 기반을 만든다.
}
```

### `InfraStatus RuntimeTick_RegisterHook(RuntimeTickHook hook, void *context)`
```c
InfraStatus RuntimeTick_RegisterHook(RuntimeTickHook hook, void *context) {
    // 작동방식
    // 1. 고정 크기 hook 슬롯 배열을 순회한다.
    // 2. 비어 있는 슬롯을 찾으면 callback과 context를 저장한다.
    // 3. 비어 있는 슬롯이 없으면 INFRA_STATUS_FULL을 반환한다.
    //
    // 왜 이렇게 동작하는가
    // - ISR 안에서 동적 할당 없이 예측 가능한 fan-out만 허용하기 위해서다.
    // - LIN처럼 base tick이 필요한 모듈을 runtime과 느슨하게 연결할 수 있다.
    //
    // 의미
    // - 인터럽트 경로를 직접 하드코딩하지 않도록 만드는 연결점이다.
}
```

### `static void RuntimeTick_IrqHandler(void)`
```c
static void RuntimeTick_IrqHandler(void) {
    // 작동방식
    // 1. base tick 카운트를 증가시킨다.
    // 2. 500us 누적치를 더하고, 1000us 이상이면 ms 카운터를 증가시킨다.
    // 3. 등록된 hook들을 순서대로 호출한다.
    // 4. 마지막에 LPTMR compare flag를 clear한다.
    //
    // 왜 이렇게 동작하는가
    // - 짧은 하드웨어 tick을 상위 계층이 쓰기 쉬운 ms 시간으로 변환하기 위해서다.
    // - hook 호출 후 flag clear 순서를 유지해 인터럽트 재진입 타이밍을 제어한다.
    //
    // 의미
    // - 시간 갱신과 ISR 알림을 동시에 수행하는 가장 낮은 레벨의 심장박동이다.
}
```

### `InfraStatus RuntimeIo_BoardInit(void)`
```c
InfraStatus RuntimeIo_BoardInit(void) {
    // 작동방식
    // 1. CLOCK_SYS_Init / CLOCK_SYS_UpdateConfiguration으로 클록 트리를 올린다.
    // 2. PINS_DRV_Init으로 핀 mux를 설정한다.
    // 3. LIN transceiver enable 핀이 있을 경우 enable 상태로 만든다.
    //
    // 왜 이렇게 동작하는가
    // - 통신과 GPIO 모듈이 동작하기 전, 보드 전체가 사용 가능한 상태여야 하기 때문이다.
    // - 상위 모듈이 pin/clock 초기화 책임을 몰라도 되도록 runtime_io가 흡수한다.
    //
    // 의미
    // - portable 모듈들이 보드 준비 완료 상태를 전제로 동작하게 만드는 준비 단계다.
}
```

### `uint8_t RuntimeIo_GetActiveRole(void)` / `uint8_t RuntimeIo_GetLocalNodeId(void)`
```c
uint8_t RuntimeIo_GetActiveRole(void)
uint8_t RuntimeIo_GetLocalNodeId(void) {
    // 작동방식
    // 1. 컴파일 타임 설정인 APP_ACTIVE_ROLE을 읽는다.
    // 2. 역할에 따라 MASTER / SLAVE1 / SLAVE2 노드 ID를 매핑한다.
    //
    // 왜 이렇게 동작하는가
    // - 같은 코드베이스를 역할별 이미지로 재사용하기 위해서다.
    // - AppCore는 "내가 어떤 역할인지"만 알면, 나머지 정책 모듈을 선택할 수 있다.
    //
    // 의미
    // - 세 프로젝트를 하나의 공통 코드 구조로 유지하게 해주는 핵심 분기점이다.
}
```

### `InfraStatus RuntimeIo_GetSlave1LedConfig(...)`, `RuntimeIo_GetSlave2LedConfig(...)`, `RuntimeIo_GetSlave2AdcConfig(...)`, `RuntimeIo_GetMasterLinConfig(...)`, `RuntimeIo_GetSlaveLinConfig(...)`
```c
InfraStatus RuntimeIo_GetXxxConfig(...)
{
    // 작동방식
    // 1. 각 portable 모듈이 이해할 수 있는 config 구조체를 0으로 초기화한다.
    // 2. 보드 전용 핀 번호, threshold, PID, token, callback 포인터를 채워 넣는다.
    // 3. 실제 SDK 인스턴스가 없으면 UNSUPPORTED를 반환한다.
    //
    // 왜 이렇게 동작하는가
    // - app/portable 모듈이 SDK 전용 심볼과 generated 구조체를 직접 알지 않게 하기 위해서다.
    // - 보드 바인딩 교체 시 수정 범위를 runtime_io로 제한하려는 목적이다.
    //
    // 의미
    // - 하드웨어 세부사항을 공통 모듈이 이해할 수 있는 "설정 객체"로 번역하는 계층이다.
}
```

---

## 2. AppCore 오케스트레이션 레이어

### `void AppCore_SetModeText(...)`, `AppCore_SetButtonText(...)`, `AppCore_SetAdcText(...)`, `AppCore_SetCanInputText(...)`, `AppCore_SetLinInputText(...)`, `AppCore_SetLinLinkText(...)`, `AppCore_SetResultText(...)`
```c
void AppCore_SetXxxText(AppCore *app, const char *text) {
    // 작동방식
    // 1. 내부 공통 helper인 AppCore_SetText() 또는 AppConsole_SetResultText()를 호출한다.
    // 2. 역할별 UI 상태 문자열 버퍼를 갱신한다.
    //
    // 왜 이렇게 동작하는가
    // - 문자열 갱신 규칙을 한곳에 모아두면 중복 snprintf 패턴이 줄어든다.
    // - 정책 함수들은 "무엇을 보여줄지"만 결정하고, 버퍼 관리 세부사항은 AppCore가 맡는다.
    //
    // 의미
    // - 각 모듈 이벤트를 사용자 관찰 가능한 화면 상태로 연결하는 최소 단위 함수군이다.
}
```

### `const char *AppCore_GetLinZoneText(uint8_t zone)`
```c
const char *AppCore_GetLinZoneText(uint8_t zone) {
    // 작동방식
    // 1. LIN zone enum 값을 safe/warning/danger/emergency 문자열로 매핑한다.
    //
    // 왜 이렇게 동작하는가
    // - 정책과 UI에서 같은 enum을 반복 해석하지 않게 하기 위해서다.
    //
    // 의미
    // - 상태 숫자를 사람이 읽는 표현으로 바꿔 주는 작은 번역기다.
}
```

### `InfraStatus AppCore_InitConsoleCan(AppCore *app)`
```c
InfraStatus AppCore_InitConsoleCan(AppCore *app) {
    // 작동방식
    // 1. AppConsole_Init()으로 UART 기반 콘솔을 초기화한다.
    // 2. CanModuleConfig를 채워 CanModule_Init()을 호출한다.
    // 3. 성공 시 console_enabled, can_enabled 플래그를 켠다.
    //
    // 왜 이렇게 동작하는가
    // - master와 slave1이 공통적으로 콘솔 + CAN을 쓰므로 이 초기화 시퀀스를 재사용하려는 목적이다.
    // - 역할 정책 모듈이 통신 인프라 생성 책임까지 중복으로 갖지 않게 한다.
    //
    // 의미
    // - 상호작용 노드의 공통 기반을 세우는 helper다.
}
```

### `InfraStatus AppCore_Init(AppCore *app)`
```c
InfraStatus AppCore_Init(AppCore *app) {
    // 작동방식
    // 1. AppCore 전체를 memset으로 비운다.
    // 2. RuntimeIo에서 active role과 local node id를 읽는다.
    // 3. 이전 LIN 상태 비교용 필드를 0xFF로 초기화한다.
    // 4. AppCore_InitDefaultTexts()로 역할별 기본 화면 문자열을 채운다.
    // 5. 역할에 따라 AppMaster_Init / AppSlave1_Init / AppSlave2_Init 중 하나를 호출한다.
    // 6. 성공 시 initialized 플래그를 켠다.
    //
    // 왜 이렇게 동작하는가
    // - AppCore는 모든 역할이 공유하는 상위 컨텍스트이므로, 역할 판별과 공통 초기화가 먼저 와야 한다.
    // - 역할별 세부 hardware/module 조립은 policy 파일로 내려 보내 책임을 분리한다.
    //
    // 의미
    // - 전체 애플리케이션 레이어의 진짜 시작점이다.
}
```

### `static void AppCore_HandleCanIncoming(AppCore *app, const CanMessage *message)`
```c
static void AppCore_HandleCanIncoming(AppCore *app, const CanMessage *message) {
    // 작동방식
    // 1. message_type이 EVENT면 결과 텍스트를 만들어 화면에 표시한다.
    // 2. TEXT면 송신자와 메시지를 UI에 표시한다.
    // 3. COMMAND면 역할에 따라 AppSlave1_HandleCanCommand 또는 AppMaster_HandleCanCommand로 전달한다.
    // 4. remote command 수신 사실을 결과 창에 표시한다.
    // 5. NEED_RESPONSE 플래그가 있으면 CanModule_QueueResponse()로 응답을 예약한다.
    //
    // 왜 이렇게 동작하는가
    // - CAN service는 transport 수준까지만 책임지고, "이 메시지가 역할상 무슨 의미인가"는 AppCore가 알아야 하기 때문이다.
    // - command/event/text를 한곳에서 갈라야 정책 처리 경로가 분명해진다.
    //
    // 의미
    // - CAN 수신 데이터가 역할 정책으로 넘어가기 직전의 중앙 관문이다.
}
```

### `void AppCore_OnTickIsr(void *context)`
```c
void AppCore_OnTickIsr(void *context) {
    // 작동방식
    // 1. context를 AppCore로 해석한다.
    // 2. LIN이 활성화된 역할이면 LinModule_OnBaseTick()을 호출한다.
    //
    // 왜 이렇게 동작하는가
    // - LIN timeout service는 base tick에 맞춰 자주 진행되어야 하므로 ISR 가까운 곳에서 처리해야 한다.
    //
    // 의미
    // - runtime tick과 LIN 상태기계를 연결하는 ISR bridge다.
}
```

### `void AppCore_TaskCan(AppCore *app, uint32_t now_ms)`
```c
void AppCore_TaskCan(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // 1. 콘솔 큐에 쌓인 open/close/off/test/text/event 명령을 모두 꺼낸다.
    // 2. 각 명령을 CanModule_QueueCommand/Event/Text로 변환한다.
    // 3. CanModule_Task()를 호출해 실제 CAN service/transport를 진행시킨다.
    // 4. 완료된 result들을 꺼내 사람이 읽는 텍스트로 바꿔 UI에 반영한다.
    // 5. 수신된 incoming message들을 꺼내 AppCore_HandleCanIncoming()으로 전달한다.
    // 6. 활동이 있었는지 can_last_activity와 can_task_count를 갱신한다.
    //
    // 왜 이렇게 동작하는가
    // - 사용자 입력, 송신 진행, 응답 처리, 수신 정책 반영을 한 주기 안에서 정리해야 CAN 흐름이 끊기지 않는다.
    // - 콘솔과 CAN 하위 모듈을 직접 붙이지 않고 AppCore가 중간 허브 역할을 한다.
    //
    // 의미
    // - CAN 경로 전체를 orchestration 하는 핵심 task다.
}
```

### `void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms)`
```c
void AppCore_TaskLinFast(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // 1. LinModule_TaskFast()로 master/slave LIN 상태기계를 진행시킨다.
    // 2. master 역할이면 fresh status를 소비해 AppMaster_HandleFreshLinStatus()로 넘긴다.
    // 3. slave2 역할이면 AppSlave2_HandleLinOkToken()으로 승인 token 도착 여부를 처리한다.
    //
    // 왜 이렇게 동작하는가
    // - LIN은 poll 주기보다 더 빠르게 상태 전이를 처리해야 응답성이 유지된다.
    // - status 소비와 token 소비를 이 빠른 경로에 두어 승인/해제 지연을 줄인다.
    //
    // 의미
    // - LIN 상태기계의 "반응 속도"를 담당하는 빠른 주기 task다.
}
```

### `void AppCore_TaskLinPoll(AppCore *app, uint32_t now_ms)`
```c
void AppCore_TaskLinPoll(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // 1. master + console 역할이면 local ok 입력을 소비해 AppMaster_RequestOk()를 시작한다.
    // 2. LIN 활성 상태면 LinModule_TaskPoll()로 status poll 또는 OK token poll을 시작한다.
    // 3. master면 AppMaster_AfterLinPoll()로 승인 중 OK token 재요청 여부를 확인한다.
    //
    // 왜 이렇게 동작하는가
    // - LIN 버스 시작 동작은 poll 주기에 맞춰 제어하는 편이 단순하고 안전하다.
    // - fast task는 전이 처리, poll task는 버스 시작 요청이라는 역할 분리를 가진다.
    //
    // 의미
    // - LIN 버스에서 "무엇을 다음에 물어볼지"를 정하는 스케줄 task다.
}
```

### `void AppCore_TaskUart(...)`, `AppCore_TaskButton(...)`, `AppCore_TaskLed(...)`, `AppCore_TaskAdc(...)`, `AppCore_TaskRender(...)`, `AppCore_TaskHeartbeat(...)`
```c
void AppCore_TaskXxx(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // - TaskUart  : AppConsole_Task() 호출로 UART 입력/출력을 진행한다.
    // - TaskButton: slave1일 때만 AppSlave1_TaskButton()을 호출한다.
    // - TaskLed   : slave1 LED task와 slave2 LED task를 역할에 따라 실행한다.
    // - TaskAdc   : slave2일 때만 AppSlave2_TaskAdc()를 호출한다.
    // - TaskRender: 현재 AppCore 상태를 콘솔 view 문자열로 조합한 뒤 AppConsole_Render()를 호출한다.
    // - TaskHeartbeat: 단순 alive 카운터를 증가시킨다.
    //
    // 왜 이렇게 동작하는가
    // - AppCore는 역할별 세부 구현을 숨기면서도 scheduler 입장에서는 일정한 task 표면을 제공해야 하기 때문이다.
    //
    // 의미
    // - runtime 스케줄러와 역할별 정책 모듈 사이의 접착제다.
}
```

---

## 3. 역할별 정책 레이어

### `InfraStatus AppMaster_Init(AppCore *app)`
```c
InfraStatus AppMaster_Init(AppCore *app) {
    // 작동방식
    // 1. AppCore_InitConsoleCan()으로 콘솔과 CAN 경로를 올린다.
    // 2. RuntimeIo_GetMasterLinConfig()로 master LIN 설정을 받는다.
    // 3. LinModule_Init() 성공 시 RuntimeIo_AttachLinModule()로 callback 목적지를 연결한다.
    // 4. LIN 초기화 실패 시 화면에 binding req를 표시한다.
    //
    // 왜 이렇게 동작하는가
    // - master는 coordinator이므로 CAN과 LIN 둘 다 필요하다.
    // - LIN callback은 전역 SDK callback에서 AppCore 내부 모듈로 전달되어야 하므로 attach가 추가로 필요하다.
    //
    // 의미
    // - 시스템 판단 노드의 통신 기반을 세우는 함수다.
}
```

### `void AppMaster_RequestOk(AppCore *app)`
```c
void AppMaster_RequestOk(AppCore *app) {
    // 작동방식
    // 1. LIN이 비활성 상태면 즉시 거절 메시지를 띄운다.
    // 2. latest LIN status가 아직 없으면 waiting 상태로 남긴다.
    // 3. 현재 zone이 emergency면 승인 요청을 거절한다.
    // 4. 이미 latch가 해제되어 있으면 "already clear"로 끝낸다.
    // 5. 그 외에는 slave1 ok 대기 플래그를 세우고 LinModule_RequestOk()로 OK token 전송을 예약한다.
    //
    // 왜 이렇게 동작하는가
    // - master는 단순 전달자가 아니라, 센서 상태가 안전 영역으로 벗어났는지 검증하는 승인자이기 때문이다.
    // - 즉시 송신하지 않고 예약만 하는 이유는 LIN bus 시작을 poll task 흐름 안에 유지하기 위해서다.
    //
    // 의미
    // - "버튼이 눌렸다"를 "실제로 해제 가능한가"로 바꾸는 정책 판단 함수다.
}
```

### `void AppMaster_HandleFreshLinStatus(AppCore *app, const LinStatusFrame *status)`
```c
void AppMaster_HandleFreshLinStatus(AppCore *app, const LinStatusFrame *status) {
    // 작동방식
    // 1. ADC 값, zone, lock 상태를 사람이 읽는 문자열로 바꿔 UI에 반영한다.
    // 2. zone == emergency 또는 lock != 0이면 emergency_active로 본다.
    // 3. emergency 상태가 이전과 달라졌다면 상태 전이 메시지를 출력한다.
    // 4. emergency 진입 시 slave1에 CAN_CMD_EMERGENCY를 보내고, 승인 대기 플래그를 초기화한다.
    // 5. LIN zone/lock 값이 바뀌었으면 [lin] 상태 메시지를 남긴다.
    // 6. 승인 대기 중인데 zone이 다시 emergency로 올라가면 요청을 취소한다.
    // 7. 승인 대기 중이고 zone이 emergency가 아니며 latch가 해제되었으면 slave1에 CAN_CMD_OK를 보낸다.
    //
    // 왜 이렇게 동작하는가
    // - master는 센서 값 자체보다 "상태 전이"를 감지해 현장 노드(slave1)를 제어해야 한다.
    // - latch가 해제되기 전까지는 slave1을 승인 완료 상태로 돌려보내면 안 된다.
    //
    // 의미
    // - 전체 시스템 정책의 핵심 결정 함수다.
}
```

### `void AppMaster_HandleCanCommand(...)`
```c
void AppMaster_HandleCanCommand(AppCore *app, const CanMessage *message, uint8_t *out_response_code) {
    // 작동방식
    // 1. slave1에서 온 CAN_CMD_OK인지 확인한다.
    // 2. 맞다면 버튼 입력 상태를 UI에 반영한다.
    // 3. AppMaster_RequestOk()를 호출해 승인 절차를 시작한다.
    // 4. response code를 CAN_RES_OK로 채운다.
    //
    // 왜 이렇게 동작하는가
    // - slave1의 버튼은 단순 입력이고, 승인 로직은 반드시 master에서만 수행해야 하기 때문이다.
    //
    // 의미
    // - 현장 버튼 이벤트가 중앙 승인 로직으로 들어오는 입구다.
}
```

### `void AppMaster_AfterLinPoll(AppCore *app)`
```c
void AppMaster_AfterLinPoll(AppCore *app) {
    // 작동방식
    // 1. slave1 승인 대기 상태가 아니면 아무것도 하지 않는다.
    // 2. latest LIN status를 읽는다.
    // 3. zone은 안전하지만 latch가 아직 남아 있으면 LinModule_RequestOk()를 다시 요청한다.
    //
    // 왜 이렇게 동작하는가
    // - LIN에서 token 하나가 유실되거나 poll 타이밍이 엇갈려도 해제 흐름이 끊기지 않게 하려는 재시도 로직이다.
    //
    // 의미
    // - 승인 절차를 끝까지 밀어 붙이는 보조 함수다.
}
```

### `InfraStatus AppSlave1_Init(AppCore *app)`
```c
InfraStatus AppSlave1_Init(AppCore *app) {
    // 작동방식
    // 1. AppCore_InitConsoleCan()으로 콘솔과 CAN 경로를 초기화한다.
    // 2. RuntimeIo_GetSlave1LedConfig()로 LED 배선 정보를 가져온다.
    // 3. LedModule_Init() 성공 시 slave1_mode를 NORMAL로 둔다.
    //
    // 왜 이렇게 동작하는가
    // - slave1은 현장 반응 노드이므로 CAN 수신 + LED 출력 + 버튼 입력이 기본이다.
    //
    // 의미
    // - 현장 액추에이터 노드의 시작점이다.
}
```

### `void AppSlave1_HandleCanCommand(...)`
```c
void AppSlave1_HandleCanCommand(AppCore *app, const CanMessage *message, uint8_t *out_response_code) {
    // 작동방식
    // 1. CAN_CMD_EMERGENCY면 mode를 EMERGENCY로 바꾸고 빨간 LED 고정을 건다.
    // 2. CAN_CMD_OK면 mode를 ACK_BLINK로 바꾸고 초록 blink 시퀀스를 시작한다.
    // 3. CAN_CMD_OFF면 mode를 NORMAL로 돌리고 LED를 끈다.
    // 4. OPEN/CLOSE/TEST는 실제 동작보다 수신 사실 기록과 response 반환에 집중한다.
    // 5. 각 경우마다 UI 텍스트와 response code를 갱신한다.
    //
    // 왜 이렇게 동작하는가
    // - slave1은 정책 판단 노드가 아니라 master 지시를 현장 표시/행동으로 바꾸는 노드이기 때문이다.
    //
    // 의미
    // - 원격 CAN 명령을 로컬 LED/모드 상태로 해석하는 함수다.
}
```

### `void AppSlave1_TaskButton(AppCore *app, uint32_t now_ms)`
```c
void AppSlave1_TaskButton(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // 1. RuntimeIo_ReadSlave1ButtonPressed()로 현재 raw 버튼 상태를 읽는다.
    // 2. 직전 샘플과 같으면 same_sample_count를 증가시키고, 다르면 카운터를 초기화한다.
    // 3. 일정 횟수 이상 같은 값이 반복되어야 stable pressed 상태로 인정한다.
    // 4. 안정된 눌림이 감지되고 현재 mode가 EMERGENCY일 때만 CAN_CMD_OK를 master로 보낸다.
    //
    // 왜 이렇게 동작하는가
    // - 기계식 버튼은 bounce가 있기 때문에 raw 값을 그대로 쓰면 잘못된 승인 요청이 올라갈 수 있다.
    // - emergency 상태가 아닐 때는 버튼 입력이 의미 없으므로 무시한다.
    //
    // 의미
    // - 현장 물리 입력을 신뢰 가능한 승인 요청 이벤트로 변환하는 함수다.
}
```

### `void AppSlave1_TaskLed(AppCore *app, uint32_t now_ms)`
```c
void AppSlave1_TaskLed(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // 1. LedModule_Task()로 현재 LED 패턴 상태기계를 진행한다.
    // 2. ACK_BLINK 모드인데 LED 패턴이 OFF로 끝났으면 slave1_mode를 NORMAL로 되돌린다.
    // 3. UI도 normal/waiting으로 복귀시킨다.
    //
    // 왜 이렇게 동작하는가
    // - 승인 완료 표현은 유한한 blink 시퀀스로 끝나야 하고, 끝난 뒤에는 자동으로 대기 상태로 돌아가야 한다.
    //
    // 의미
    // - slave1의 상태 복귀를 책임지는 후처리 함수다.
}
```

### `InfraStatus AppSlave2_Init(AppCore *app)`
```c
InfraStatus AppSlave2_Init(AppCore *app) {
    // 작동방식
    // 1. slave2 LED config를 받아 LedModule_Init()을 수행한다.
    // 2. ADC config를 받아 AdcModule_Init()을 수행한다.
    // 3. LIN slave config를 받아 LinModule_Init()을 수행하고 callback 목적지를 attach한다.
    // 4. 실패한 항목은 binding req 텍스트로 표시한다.
    //
    // 왜 이렇게 동작하는가
    // - slave2는 센서 노드이므로 ADC + LIN + LED 세 축이 함께 초기화되어야 완전한 동작을 한다.
    //
    // 의미
    // - 센서 상태 제공 노드의 시작 함수다.
}
```

### `void AppSlave2_HandleLinOkToken(AppCore *app)`
```c
void AppSlave2_HandleLinOkToken(AppCore *app) {
    // 작동방식
    // 1. LinModule_ConsumeSlaveOkToken()으로 master가 보낸 승인 token을 소비한다.
    // 2. token이 있고 ADC latch clear가 가능하면 AdcModule_ClearEmergencyLatch()를 호출한다.
    // 3. 성공 시 UI에 ok token 수신 사실을 기록한다.
    //
    // 왜 이렇게 동작하는가
    // - latch는 센서가 실제로 emergency에서 벗어난 뒤, master 승인까지 받아야만 해제되어야 하기 때문이다.
    //
    // 의미
    // - 중앙 승인 신호를 실제 latch 해제로 바꾸는 함수다.
}
```

### `void AppSlave2_TaskAdc(AppCore *app, uint32_t now_ms)`
```c
void AppSlave2_TaskAdc(AppCore *app, uint32_t now_ms) {
    // 작동방식
    // 1. AdcModule_Task()로 주기 샘플링을 수행한다.
    // 2. AdcModule_GetSnapshot()으로 최신 raw value, zone, latch 상태를 읽는다.
    // 3. 이를 콘솔용 문자열로 변환해 adc_text에 반영한다.
    // 4. 같은 내용을 LinStatusFrame으로 포장해 LinModule_SetSlaveStatus()에 넣는다.
    // 5. LED가 있다면 latch 상태와 zone에 따라 빨강 blink/초록/노랑/빨강 패턴을 적용한다.
    //
    // 왜 이렇게 동작하는가
    // - slave2는 센서 값을 단순 저장만 하는 게 아니라, LIN 제공용 상태 cache와 로컬 시각화까지 동시에 책임진다.
    //
    // 의미
    // - 센서 획득, 버스 게시, 로컬 표시가 한곳에서 만나는 핵심 함수다.
}
```

---

## 4. 콘솔 / UART 레이어

### `InfraStatus AppConsole_Init(AppConsole *console, uint8_t node_id)`
```c
InfraStatus AppConsole_Init(AppConsole *console, uint8_t node_id) {
    // 작동방식
    // 1. AppConsole 전체를 초기화하고 node_id를 기록한다.
    // 2. CAN 명령 큐를 만든다.
    // 3. UartService_Init()으로 UART transport를 준비한다.
    // 4. task/source/result/value view에 기본 placeholder 문자열을 채운다.
    // 5. full refresh를 요청하고 initialized 상태로 둔다.
    //
    // 왜 이렇게 동작하는가
    // - 실제 센서/통신 데이터가 오기 전에도 사용자에게 구조화된 화면을 보여주기 위해서다.
    //
    // 의미
    // - UART 기반 HMI의 부트스트랩 함수다.
}
```

### `static void AppConsole_HandleLine(AppConsole *console, const char *line)`
```c
static void AppConsole_HandleLine(AppConsole *console, const char *line) {
    // 작동방식
    // 1. help/hello/ping/status/ok 같은 로컬 명령은 즉시 처리한다.
    // 2. 그 외 명령은 tokenize해서 open/close/off/test/text/event 형식인지 검사한다.
    // 3. 대상 node, 숫자 인자, text 길이를 검증한다.
    // 4. 검증이 끝나면 AppConsoleCanCommand 구조체로 바꿔 큐에 넣는다.
    //
    // 왜 이렇게 동작하는가
    // - UART 입력은 사람이 치므로 오류가 많다. 그래서 가능한 한 상위에서 엄격히 걸러야 한다.
    // - 직접 CAN 전송하지 않고 큐에 넣는 이유는 콘솔 입력 처리와 통신 task를 분리하기 위해서다.
    //
    // 의미
    // - 사람의 텍스트 입력을 시스템 명령 객체로 번역하는 parser다.
}
```

### `static void AppConsole_QueueCanCommand(AppConsole *console, const AppConsoleCanCommand *command)`
```c
static void AppConsole_QueueCanCommand(AppConsole *console, const AppConsoleCanCommand *command) {
    // 작동방식
    // 1. 콘솔 내부 CAN 명령 큐에 command를 push한다.
    // 2. 성공하면 [queued] 상태 메시지를 결과 창에 남긴다.
    // 3. 실패하면 queue full을 표시한다.
    //
    // 왜 이렇게 동작하는가
    // - 사용자 입력을 즉시 transport에 밀어 넣으면 scheduler 구조가 깨지고 입력 처리가 통신 지연에 묶인다.
    //
    // 의미
    // - 콘솔과 AppCore/CAN task 사이의 비동기 경계다.
}
```

### `void AppConsole_Task(AppConsole *console, uint32_t now_ms)`
```c
void AppConsole_Task(AppConsole *console, uint32_t now_ms) {
    // 작동방식
    // 1. UartService_ProcessRx()로 누적된 바이트를 line으로 조립한다.
    // 2. UART 오류가 있으면 error 상태로 들어가고, recover 명령만 허용한다.
    // 3. 오류가 없고 완성된 line이 있으면 UartService_ReadLine()으로 읽어 AppConsole_HandleLine()에 넘긴다.
    // 4. 마지막에 UartService_ProcessTx()로 출력 큐를 진행한다.
    //
    // 왜 이렇게 동작하는가
    // - RX와 TX를 하나의 task에서 같이 돌려야 콘솔이 끊기지 않는다.
    // - error 상태에서도 recover 경로를 열어 두어 전체 재부팅 없이 UART만 복구할 수 있게 한다.
    //
    // 의미
    // - 콘솔 모듈의 중앙 실행 함수다.
}
```

### `void AppConsole_Render(AppConsole *console)`
```c
void AppConsole_Render(AppConsole *console) {
    // 작동방식
    // 1. error 상태가 아니면 현재 입력 중인 텍스트를 input view에 반영한다.
    // 2. layout이 아직 그려지지 않았거나 full refresh가 필요하면 전체 레이아웃을 그린다.
    // 3. 그 후 dirty line만 다시 출력한다.
    //
    // 왜 이렇게 동작하는가
    // - 매번 전체 화면을 다시 그리면 UART 트래픽이 커지고 반응성이 떨어진다.
    // - layout과 내용 줄 갱신을 분리하면 텍스트 UI가 더 가볍게 움직인다.
    //
    // 의미
    // - 콘솔의 dirty render 엔진이다.
}
```

### `uint8_t AppConsole_TryPopCanCommand(...)`, `uint8_t AppConsole_ConsumeLocalOk(...)`
```c
uint8_t AppConsole_TryPopCanCommand(...)
uint8_t AppConsole_ConsumeLocalOk(...) {
    // 작동방식
    // - TryPopCanCommand: 파싱된 CAN 요청을 AppCore가 가져갈 수 있게 큐에서 하나 꺼낸다.
    // - ConsumeLocalOk   : 로컬 ok 입력 플래그를 1회성 이벤트처럼 소비한다.
    //
    // 왜 이렇게 동작하는가
    // - 콘솔은 입력 생성자이고, AppCore는 실제 처리자이므로 이벤트 소비 인터페이스가 필요하다.
    //
    // 의미
    // - 콘솔과 AppCore 사이의 pull 기반 이벤트 API다.
}
```

### `InfraStatus UartService_Init(UartService *service)`
```c
InfraStatus UartService_Init(UartService *service) {
    // 작동방식
    // 1. 소프트웨어 RX/TX 상태를 reset한다.
    // 2. TX 큐를 만든다.
    // 3. UartHw_InitDefault()로 실제 UART driver와 callback을 연결한다.
    // 4. 성공 시 initialized 플래그를 켠다.
    //
    // 왜 이렇게 동작하는가
    // - 하드웨어 초기화 실패 시에도 service 구조체 자체는 진단 가능한 상태를 유지해야 하기 때문이다.
    //
    // 의미
    // - UART를 "라인 기반 서비스"로 쓸 수 있게 만드는 시작 함수다.
}
```

### `void UartService_ProcessRx(UartService *service)`
```c
void UartService_ProcessRx(UartService *service) {
    // 작동방식
    // 1. ISR 쪽 pending ring overflow 여부를 먼저 검사한다.
    // 2. overflow면 RX 상태를 reset하고 오류를 기록한다.
    // 3. overflow가 아니면 pending byte를 하나씩 꺼내 UartService_OnRxByte()로 line buffer에 반영한다.
    // 4. line overflow가 발생하면 reset + error 처리한다.
    // 5. 완성된 line이 생기면 더 이상 읽지 않고 다음 단계로 넘긴다.
    //
    // 왜 이렇게 동작하는가
    // - ISR은 바이트만 쌓고, 문자열 해석은 일반 task 문맥에서 해야 시스템 전체가 단순해진다.
    //
    // 의미
    // - 바이트 스트림을 명령 단위 line으로 조립하는 핵심 함수다.
}
```

### `void UartService_ProcessTx(UartService *service, uint32_t now_ms)`
```c
void UartService_ProcessTx(UartService *service, uint32_t now_ms) {
    // 작동방식
    // 1. 현재 busy가 아니면 TX 큐에서 chunk 하나를 꺼낸다.
    // 2. 꺼낸 chunk를 current_buffer로 복사하고 UartHw_StartTransmit()을 호출한다.
    // 3. busy 상태면 timeout 여부를 먼저 검사한다.
    // 4. timeout이 아니면 UartHw_GetTransmitStatus()로 완료/진행/실패를 판별한다.
    // 5. 완료면 busy를 해제하고 다음 chunk로 넘어갈 준비를 한다.
    //
    // 왜 이렇게 동작하는가
    // - 긴 텍스트 출력을 잘게 나눠야 scheduler가 큰 문자열 하나 때문에 오래 막히지 않는다.
    // - timeout을 service가 직접 감시해야 driver가 멈춘 상황도 탐지할 수 있다.
    //
    // 의미
    // - UART 출력 큐를 실시간으로 비워 주는 송신 엔진이다.
}
```

### `InfraStatus UartService_RequestTx(UartService *service, const char *text)`
```c
InfraStatus UartService_RequestTx(UartService *service, const char *text) {
    // 작동방식
    // 1. 문자열 길이를 계산한다.
    // 2. UartService_RequestTxBytes()로 여러 chunk로 나눠 TX 큐에 넣는다.
    //
    // 왜 이렇게 동작하는가
    // - 콘솔 렌더러는 문자열만 던지고, 실제 분할 전송은 서비스 계층이 맡아야 역할이 깔끔해진다.
    //
    // 의미
    // - 상위 모듈이 UART에 텍스트를 보내는 표준 입구다.
}
```

### `InfraStatus UartService_ReadLine(UartService *service, char *out_buffer, uint16_t max_length)`
```c
InfraStatus UartService_ReadLine(UartService *service, char *out_buffer, uint16_t max_length) {
    // 작동방식
    // 1. 완성된 line이 있는지 확인한다.
    // 2. caller 버퍼 길이에 맞춰 안전하게 복사한다.
    // 3. 복사 후 RX 조립 상태를 reset한다.
    //
    // 왜 이렇게 동작하는가
    // - 한 번 소비된 line은 다시 읽히면 안 되므로, 읽기와 reset을 묶어 원샷 이벤트처럼 만든다.
    //
    // 의미
    // - 콘솔 parser가 사용할 완성 명령 추출 함수다.
}
```

### `status_t UartHw_InitDefault(...)`, `UartHw_StartTransmit(...)`, `UartHw_GetTransmitStatus(...)`, `void UartHw_RxCallback(...)`
```c
status_t UartHw_Xxx(...) {
    // 작동방식
    // - InitDefault      : generated UART 인스턴스를 초기화하고 RX callback을 설치한다.
    // - StartTransmit    : 현재 chunk 전송을 SDK driver에 넘긴다.
    // - GetTransmitStatus: driver의 전송 완료 여부를 조회한다.
    // - RxCallback       : 인터럽트/driver 이벤트에서 바이트를 pending ring에 쌓고 다음 1바이트 수신을 이어 간다.
    //
    // 왜 이렇게 동작하는가
    // - 하드웨어 의존 로직을 service 밖으로 분리해, service가 line/queue 책임만 지게 하기 위해서다.
    //
    // 의미
    // - UART service와 실제 LPUART driver 사이의 어댑터 계층이다.
}
```

---

## 5. ADC / LED 레이어

### `InfraStatus AdcModule_Init(AdcModule *module, const AdcConfig *config)`
```c
InfraStatus AdcModule_Init(AdcModule *module, const AdcConfig *config) {
    // 작동방식
    // 1. 모듈을 0으로 초기화하고 config를 복사한다.
    // 2. sample_fn 존재 여부와 range_max를 검증한다.
    // 3. init_fn이 있으면 보드별 ADC 초기화를 요청한다.
    // 4. 성공 시 initialized 플래그를 켠다.
    //
    // 왜 이렇게 동작하는가
    // - ADC 모듈은 portable해야 하므로, 실제 초기화는 callback으로 위임한다.
    //
    // 의미
    // - 샘플링 엔진의 설정을 굳히는 함수다.
}
```

### `void AdcModule_Task(AdcModule *module, uint32_t now_ms)`
```c
void AdcModule_Task(AdcModule *module, uint32_t now_ms) {
    // 작동방식
    // 1. sample_period_ms가 아직 안 지났으면 바로 반환한다.
    // 2. sample_fn으로 raw ADC 값을 읽는다.
    // 3. 범위를 range_max-1 이하로 clamp한다.
    // 4. AdcModule_ClassifyZone()으로 safe/warning/danger/emergency를 계산한다.
    // 5. snapshot에 raw, zone, has_sample, sample_error를 기록한다.
    // 6. zone이 emergency면 emergency_latched를 1로 유지한다.
    //
    // 왜 이렇게 동작하는가
    // - upper layer는 raw 값보다 "해석된 상태 + latch"를 원한다.
    // - emergency가 한 번 들어오면 명시적 승인 전까지 기억해야 안전 로직이 성립한다.
    //
    // 의미
    // - ADC 값을 정책 판단 가능한 상태 객체로 바꾸는 함수다.
}
```

### `InfraStatus AdcModule_ClearEmergencyLatch(AdcModule *module)`
```c
InfraStatus AdcModule_ClearEmergencyLatch(AdcModule *module) {
    // 작동방식
    // 1. 유효 샘플이 있는지 확인한다.
    // 2. 최신 zone이 아직 emergency면 BUSY를 반환한다.
    // 3. emergency가 아니면 latch를 0으로 지운다.
    //
    // 왜 이렇게 동작하는가
    // - 승인 token이 들어왔더라도 센서가 아직 위험 상태면 latch를 풀면 안 되기 때문이다.
    //
    // 의미
    // - 안전 조건과 승인 조건을 동시에 만족할 때만 잠금을 푸는 관문이다.
}
```

### `InfraStatus LedModule_Init(...)`, `void LedModule_SetPattern(...)`, `void LedModule_StartGreenAckBlink(...)`, `void LedModule_Task(...)`
```c
void/InfraStatus LedModule_Xxx(...) {
    // 작동방식
    // - Init                 : 핀 방향을 설정하고 기본 OFF 상태를 적용한다.
    // - SetPattern           : steady 또는 반복 blink 패턴으로 즉시 전환한다.
    // - StartGreenAckBlink   : 유한 횟수의 초록 blink 시퀀스를 시작한다.
    // - Task                 : blink 패턴의 phase를 토글하고, finite blink가 끝나면 OFF로 돌아간다.
    //
    // 왜 이렇게 동작하는가
    // - app 정책은 빨강/초록/노랑/ack blink 같은 의미만 알고 싶고, 실제 GPIO 수준은 숨겨야 하기 때문이다.
    // - 승인 완료처럼 "잠깐만 보여야 하는" 상태를 finite blink로 분리해야 자동 복귀가 가능하다.
    //
    // 의미
    // - 의미 기반 LED 상태를 실제 출력 상태기계로 바꿔 주는 모듈이다.
}
```

---

## 6. LIN 레이어

### `InfraStatus LinModule_Init(LinModule *module, const LinConfig *config)`
```c
InfraStatus LinModule_Init(LinModule *module, const LinConfig *config) {
    // 작동방식
    // 1. module을 0으로 초기화하고 config를 복사한다.
    // 2. state를 IDLE로 둔다.
    // 3. binding.init_fn()을 호출해 실제 LIN peripheral을 초기화한다.
    // 4. 성공 시 initialized 플래그를 켠다.
    //
    // 왜 이렇게 동작하는가
    // - protocol 상태기계는 portable해야 하고, 실제 LIN 드라이버 초기화는 runtime_io callback이 맡아야 한다.
    //
    // 의미
    // - master/slave 공용 LIN 상태기계의 출발점이다.
}
```

### `static void LinModule_GotoIdle(LinModule *module)`
```c
static void LinModule_GotoIdle(LinModule *module) {
    // 작동방식
    // 1. binding.goto_idle_fn()이 있으면 하드웨어에도 idle 진입을 요청한다.
    // 2. 내부 state를 IDLE로 되돌리고 current_pid를 지운다.
    //
    // 왜 이렇게 동작하는가
    // - 프로토콜 상태와 하드웨어 상태가 어긋나면 다음 transaction이 깨질 수 있기 때문이다.
    //
    // 의미
    // - LIN 상태기계의 공통 복귀 지점이다.
}
```

### `void LinModule_OnEvent(LinModule *module, LinEventId event_id, uint8_t current_pid)`
```c
void LinModule_OnEvent(LinModule *module, LinEventId event_id, uint8_t current_pid) {
    // 작동방식
    // 1. runtime_io가 번역한 PID_OK / RX_DONE / TX_DONE / ERROR 이벤트를 받는다.
    // 2. master 역할이면 event를 flag로만 기록해 fast task가 처리하게 한다.
    // 3. slave 역할이면 PID에 따라 즉시 status 송신 또는 ok token 수신을 시작한다.
    // 4. RX_DONE에서 ok token이 맞으면 ok_token_pending을 세운 뒤 idle로 돌아간다.
    //
    // 왜 이렇게 동작하는가
    // - master는 상태기계를 task 문맥에서 관리하는 편이 읽기 쉽고 안전하다.
    // - slave는 PID를 받는 즉시 반응해야 하므로 event callback 안에서 바로 RX/TX를 시작한다.
    //
    // 의미
    // - 하드웨어 이벤트가 프로토콜 상태 전이로 번역되는 핵심 접점이다.
}
```

### `void LinModule_TaskFast(LinModule *module, uint32_t now_ms)`
```c
void LinModule_TaskFast(LinModule *module, uint32_t now_ms) {
    // 작동방식
    // 1. master 역할이 아니면 바로 반환한다.
    // 2. ERROR flag가 있으면 pending OK 전송을 정리하고 idle로 복귀한다.
    // 3. WAIT_PID + PID_OK면 현재 PID가 status인지 ok인지 보고 RX/TX를 시작한다.
    // 4. WAIT_RX + RX_DONE이면 status payload를 decode하고 latest_status를 갱신한다.
    // 5. WAIT_TX + TX_DONE이면 OK token 송신 완료로 보고 ok_tx_pending을 내린다.
    //
    // 왜 이렇게 동작하는가
    // - poll 시작과 실제 전이 처리를 분리해야 버스 시작 주기와 응답 처리를 독립적으로 제어할 수 있다.
    //
    // 의미
    // - master LIN 프로토콜의 실제 상태기계 엔진이다.
}
```

### `void LinModule_TaskPoll(LinModule *module, uint32_t now_ms)`
```c
void LinModule_TaskPoll(LinModule *module, uint32_t now_ms) {
    // 작동방식
    // 1. master 역할이 아니면 반환한다.
    // 2. poll 주기가 아직 안 됐으면 반환한다.
    // 3. ok_tx_pending이 있으면 pid_ok header를 시작한다.
    // 4. 없으면 pid_status header를 시작한다.
    //
    // 왜 이렇게 동작하는가
    // - status 조회와 승인 token 송신 모두 master가 bus header를 여는 방식으로 시작되기 때문이다.
    // - OK token을 우선시키면 승인 지연을 줄일 수 있다.
    //
    // 의미
    // - master가 버스에서 다음에 수행할 transaction 종류를 선택하는 함수다.
}
```

### `InfraStatus LinModule_RequestOk(LinModule *module)`
```c
InfraStatus LinModule_RequestOk(LinModule *module) {
    // 작동방식
    // 1. master 역할인지 확인한다.
    // 2. ok_tx_pending 플래그를 세운다.
    // 3. tx_buffer[0]에 ok_token 값을 써 둔다.
    //
    // 왜 이렇게 동작하는가
    // - 즉시 전송하지 않고 poll 흐름에서 시작해야 LIN 상태기계가 한 번에 하나의 transaction만 관리할 수 있다.
    //
    // 의미
    // - 승인 token 송신 예약 함수다.
}
```

### `uint8_t LinModule_GetLatestStatus(...)`, `uint8_t LinModule_ConsumeFreshStatus(...)`, `void LinModule_SetSlaveStatus(...)`, `uint8_t LinModule_ConsumeSlaveOkToken(...)`
```c
uint8_t/void LinModule_Xxx(...) {
    // 작동방식
    // - GetLatestStatus      : 현재 cache된 status를 읽되 fresh 플래그는 건드리지 않는다.
    // - ConsumeFreshStatus   : fresh 상태일 때만 읽고 fresh를 0으로 내려 중복 소비를 막는다.
    // - SetSlaveStatus       : slave가 내보낼 최신 status cache를 갱신한다.
    // - ConsumeSlaveOkToken  : master가 보낸 승인 token 수신 여부를 1회성으로 소비한다.
    //
    // 왜 이렇게 동작하는가
    // - producer/slave와 consumer/master 사이를 "cache + one-shot fresh flag"로 나누면 polling 구조에서 중복 처리를 줄일 수 있다.
    //
    // 의미
    // - LIN 모듈이 app 계층과 데이터를 주고받는 상태 인터페이스다.
}
```

---

## 7. CAN 레이어

### `uint8_t CanProto_Init(CanProto *proto, const CanProtoConfig *config)`
```c
uint8_t CanProto_Init(CanProto *proto, const CanProtoConfig *config) {
    // 작동방식
    // 1. codec 상태를 비운다.
    // 2. local node id를 기록하고 initialized 상태로 둔다.
    //
    // 왜 이렇게 동작하는가
    // - encode/decode 통계와 노드 식별자를 codec 수준에서 유지하기 위해서다.
    //
    // 의미
    // - 논리 메시지와 CAN frame 변환기의 초기화 함수다.
}
```

### `uint8_t CanProto_EncodeMessage(...)`
```c
uint8_t CanProto_EncodeMessage(CanProto *proto, const CanMessage *message, CanEncodedFrameList *out_list) {
    // 작동방식
    // 1. version, source/target node id, message type를 검증한다.
    // 2. message_type에 따라 CAN 표준 ID를 결정한다.
    // 3. TEXT면 text length와 ASCII printable 여부를 검증하고 text 전용 frame layout으로 채운다.
    // 4. COMMAND/RESPONSE/EVENT면 8바이트 제어 frame layout으로 채운다.
    //
    // 왜 이렇게 동작하는가
    // - app 계층은 논리 메시지만 알고, wire format은 protocol 계층이 책임져야 하기 때문이다.
    // - TEXT와 CONTROL의 프레임 구조가 다르므로 encode 단계에서 분기해야 한다.
    //
    // 의미
    // - 시스템 내부 메시지를 실제 CAN payload로 바꾸는 함수다.
}
```

### `CanProtoDecodeStatus CanProto_DecodeFrame(...)`
```c
CanProtoDecodeStatus CanProto_DecodeFrame(CanProto *proto, const CanFrame *frame, CanMessage *out_message) {
    // 작동방식
    // 1. extended ID / remote frame 같은 지원하지 않는 프레임을 먼저 걸러낸다.
    // 2. 표준 ID를 message type으로 역매핑한다.
    // 3. TEXT면 header 길이, node id, text length, ASCII 유효성을 검사한다.
    // 4. CONTROL 계열이면 dlc, version, node id, payload kind를 검사한다.
    // 5. decode 결과에 따라 OK / INVALID / IGNORED / UNSUPPORTED를 반환한다.
    //
    // 왜 이렇게 동작하는가
    // - 상위 계층이 raw CAN frame을 직접 검증하게 하면 역할 분리가 무너진다.
    //
    // 의미
    // - wire format을 app이 이해하는 CanMessage로 복원하는 함수다.
}
```

### `uint8_t CanHw_InitDefault(...)`, `void CanHw_Task(...)`, `uint8_t CanHw_StartTx(...)`, `uint8_t CanHw_TryPopRx(...)`
```c
uint8_t/void CanHw_Xxx(...) {
    // 작동방식
    // - InitDefault: FlexCAN 인스턴스, TX/RX mailbox, accept-all mask, 첫 수신 시작을 설정한다.
    // - Task       : TX 완료 여부와 RX 완료 여부를 poll하고, RX frame을 소프트웨어 큐로 옮긴다.
    // - StartTx    : 현재 mailbox가 비어 있을 때 raw frame 전송을 시작한다.
    // - TryPopRx   : 소프트웨어 RX 큐에서 raw frame 하나를 꺼낸다.
    //
    // 왜 이렇게 동작하는가
    // - generated SDK driver와 transport/service 계층 사이를 절연해, 상위는 mailbox 세부사항을 몰라도 되게 하려는 목적이다.
    // - RX를 바로 상위로 넘기지 않고 큐에 넣어 polling 구조와 맞춘다.
    //
    // 의미
    // - CAN 하드웨어를 polling 친화적 raw frame 인터페이스로 감싼 계층이다.
}
```

### `uint8_t CanService_Init(CanService *service, uint8_t local_node_id, uint32_t default_timeout_ms)`
```c
uint8_t CanService_Init(CanService *service, uint8_t local_node_id, uint32_t default_timeout_ms) {
    // 작동방식
    // 1. service 전체를 비운다.
    // 2. local node id, default timeout, next request id를 설정한다.
    // 3. pending table을 모두 clear한다.
    // 4. CanProto_Init()과 CanTransport_Init()을 수행한다.
    // 5. 성공 시 initialized 상태가 된다.
    //
    // 왜 이렇게 동작하는가
    // - protocol, transport, pending request 관리가 한 객체 안에서 일관되게 시작해야 하기 때문이다.
    //
    // 의미
    // - request/response 추적이 가능한 CAN service의 시작 함수다.
}
```

### `void CanService_Task(CanService *service, uint32_t now_ms)`
```c
void CanService_Task(CanService *service, uint32_t now_ms) {
    // 작동방식
    // 1. 현재 tick 시각을 저장한다.
    // 2. CanTransport_Task()로 하드웨어 TX/RX를 진행한다.
    // 3. CanService_ProcessRx()로 raw frame을 decode한다.
    // 4. CanService_ProcessTimeouts()로 pending request timeout을 검사한다.
    //
    // 왜 이렇게 동작하는가
    // - CAN 하위 동작은 송신 진행, 수신 decode, timeout 감시가 함께 돌아야 완전한 service가 되기 때문이다.
    //
    // 의미
    // - CAN service의 중앙 펌프 함수다.
}
```

### `uint8_t CanService_SendCommand(...)`
```c
uint8_t CanService_SendCommand(CanService *service, uint8_t target_node_id, uint8_t command_code, uint8_t arg0, uint8_t arg1, uint8_t need_response) {
    // 작동방식
    // 1. target node id 유효성을 검사한다.
    // 2. CanMessage를 COMMAND 형식으로 구성한다.
    // 3. 응답이 필요한 단일 대상 명령이면 free pending slot을 찾고 request_id를 할당한다.
    // 4. CanService_SendMessage()로 protocol encode + transport queue 적재를 수행한다.
    // 5. pending 추적이 필요한 경우 timeout 정보와 함께 pending table에 기록한다.
    //
    // 왜 이렇게 동작하는가
    // - command는 단순 송신이 아니라, 필요시 응답과 timeout까지 추적해야 하기 때문이다.
    //
    // 의미
    // - CAN 제어 요청의 핵심 송신 API다.
}
```

### `uint8_t CanService_SendResponse(...)`, `CanService_SendEvent(...)`, `CanService_SendText(...)`
```c
uint8_t CanService_SendXxx(...) {
    // 작동방식
    // - SendResponse: request_id와 result_code를 담은 RESPONSE 메시지를 구성해 송신한다.
    // - SendEvent   : 비동기 알림용 EVENT 메시지를 구성해 송신한다.
    // - SendText    : printable ASCII 검사 후 짧은 텍스트 메시지를 구성해 송신한다.
    //
    // 왜 이렇게 동작하는가
    // - command/response/event/text는 의미와 추적 방식이 다르므로 service API도 분리되어야 한다.
    //
    // 의미
    // - CAN 상위 계층이 사용하는 메시지 종류별 표준 송신 API다.
}
```

### `uint8_t CanService_PopReceivedMessage(...)` / `uint8_t CanService_PopResult(...)`
```c
uint8_t CanService_PopReceivedMessage(...)
uint8_t CanService_PopResult(...) {
    // 작동방식
    // - PopReceivedMessage: decode와 target 검증을 통과한 COMMAND/EVENT/TEXT를 incoming queue에서 꺼낸다.
    // - PopResult         : RESPONSE 또는 TIMEOUT으로 생성된 result를 result queue에서 꺼낸다.
    //
    // 왜 이렇게 동작하는가
    // - app은 수신 메시지와 완료 결과를 다른 의미로 처리해야 하므로 큐도 분리한다.
    //
    // 의미
    // - service와 AppCore 사이의 데이터 배출 인터페이스다.
}
```

### `static void CanTransport_ProcessTx(...)`, `CanTransport_DrainHwRx(...)`, `CanService_ProcessResponse(...)`, `CanService_ProcessTimeouts(...)`
```c
static void CanXxx(...) {
    // 작동방식
    // - CanTransport_ProcessTx : in-flight TX 완료를 감시하고, 다음 frame을 hardware로 보낸다.
    // - CanTransport_DrainHwRx : hardware RX 큐를 software RX 큐로 옮긴다.
    // - CanService_ProcessResponse: response request_id와 pending table을 매칭해 result를 만든다.
    // - CanService_ProcessTimeouts: timeout이 지난 pending request를 result timeout으로 합성한다.
    //
    // 왜 이렇게 동작하는가
    // - transport와 service를 나누어야 raw frame 처리와 논리 request 추적을 독립적으로 이해할 수 있다.
    //
    // 의미
    // - CAN 내부 계층 분리의 핵심 helper들이다.
}
```

### `InfraStatus CanModule_Init(...)`, `void CanModule_Task(...)`, `InfraStatus CanModule_QueueCommand/Response/Event/Text(...)`
```c
InfraStatus/void CanModule_Xxx(...) {
    // 작동방식
    // - Init          : 내부 request queue와 CanService를 함께 초기화한다.
    // - Task          : 먼저 CanService_Task()를 돌리고, 그 다음 pending app request를 일정량만 제출한다.
    // - QueueCommand  : app 명령을 module 내부 큐에 저장한다.
    // - QueueResponse : 로컬 처리 결과를 나중에 송신할 response 요청으로 저장한다.
    // - QueueEvent    : 비동기 event 요청을 저장한다.
    // - QueueText     : 사용자/운영자 text 전송 요청을 저장한다.
    //
    // 왜 이렇게 동작하는가
    // - app이 service 가용성, 하드웨어 혼잡, timeout 구조를 몰라도 되게 하려면 중간 buffering 계층이 필요하다.
    // - max_submit_per_tick 제한은 바쁜 콘솔 사이클 하나가 CAN 전송을 과점하지 못하게 하는 안전장치다.
    //
    // 의미
    // - AppCore가 사용하는 "간단한 CAN 요청 큐 계층"이다.
}
```

---

## 8. 공통 버퍼 / 유틸리티 레이어

### `InfraStatus InfraQueue_Init(...)`, `InfraQueue_Push(...)`, `InfraQueue_Pop(...)`, `InfraQueue_Peek(...)`
```c
InfraStatus InfraQueue_Xxx(...) {
    // 작동방식
    // - Init : caller가 넘긴 저장 공간을 ring queue 메타데이터에 연결하고 전체 버퍼를 0으로 지운다.
    // - Push : tail 슬롯에 item 전체를 memcpy하고 tail/count를 전진시킨다.
    // - Pop  : head 슬롯의 item을 꺼내고 슬롯을 0으로 지운 뒤 head/count를 전진시킨다.
    // - Peek : head를 움직이지 않고 현재 선두 item만 읽는다.
    //
    // 왜 이렇게 동작하는가
    // - 이 프로젝트는 동적 메모리 없이 고정 크기 구조체 복사 기반 버퍼링을 택하고 있기 때문이다.
    // - Peek가 있어야 CanModule처럼 "보낼 수 있을 때만 pop"하는 패턴을 만들 수 있다.
    //
    // 의미
    // - 거의 모든 모듈이 재사용하는 공통 ring queue다.
}
```

### `static inline uint32_t Infra_TimeElapsedMs(...)` 와 시간 매크로들
```c
uint32_t Infra_TimeXxx(...) {
    // 작동방식
    // 1. wrap-around를 고려해 경과 시간을 계산한다.
    // 2. TimeIsDue / TimeIsExpired 류 판단에서 공통으로 사용된다.
    //
    // 왜 이렇게 동작하는가
    // - 32비트 tick 카운터는 언젠가 오버플로우하므로, 단순 비교보다 경과 시간 기반 계산이 안전하다.
    //
    // 의미
    // - 모든 주기 판단과 timeout 판정의 수학적 기반이다.
}
```

---

## 9. 왜 이런 구조로 나눴는가

### `runtime / app / module / runtime_io / infra` 분리 의미
```c
아키텍처 분리 {
    // runtime
    // - 시스템 시작 순서와 주기 스케줄만 책임진다.
    // - "언제 호출할지"를 결정한다.
    //
    // app
    // - 역할 정책을 책임진다.
    // - "무슨 의미인지"와 "무엇을 결정할지"를 담당한다.
    //
    // module (can/lin/adc/led/uart)
    // - 기능 자체를 캡슐화한다.
    // - "어떻게 동작할지"를 각 도메인별로 구현한다.
    //
    // runtime_io
    // - generated SDK와 portable 모듈 사이를 번역한다.
    // - "어떤 보드 자원에 연결되는지"를 담당한다.
    //
    // infra
    // - queue, tick, 시간 계산 같은 공통 기반을 제공한다.
    //
    // 왜 이렇게 나누는가
    // - 정책, 기능, 하드웨어 의존성이 섞이면 코드 읽기가 급격히 어려워진다.
    // - 특히 re_2처럼 역할별 프로젝트가 여러 개인 구조에서는 공통부와 역할부를 분리해야 유지보수가 가능하다.
}
```

### `master / slave1 / slave2` 역할 의미
```c
역할 분리 {
    // master
    // - LIN 상태를 읽고, 전체 시스템이 emergency인지 판단한다.
    // - 승인 요청을 검증하고 slave1/slave2에 명령을 내린다.
    //
    // slave1
    // - 현장 반응 노드다.
    // - CAN 명령을 LED/버튼 기반 사용자 상호작용으로 바꾼다.
    //
    // slave2
    // - 센서 상태 제공 노드다.
    // - ADC 값을 읽고 LIN status로 게시하며 latch를 유지한다.
    //
    // 왜 이렇게 나누는가
    // - 센서, 판단, 현장 반응을 분리하면 각 노드 책임이 단순해지고 확장도 쉬워진다.
}
```

## 10. 문서 활용 팁
- 전체 흐름을 가장 빨리 이해하려면 `main -> Runtime_Init -> AppCore_Init -> AppCore_TaskCan/LinFast/LinPoll -> AppMaster_HandleFreshLinStatus` 순으로 읽는 것이 좋다.
- slave1만 집중해서 보려면 `AppSlave1_HandleCanCommand -> AppSlave1_TaskButton -> AppSlave1_TaskLed -> LedModule_*` 순서가 좋다.
- slave2만 집중해서 보려면 `AppSlave2_TaskAdc -> AdcModule_Task -> LinModule_SetSlaveStatus -> AppSlave2_HandleLinOkToken` 순서가 좋다.
- CAN 내부는 `CanModule -> CanService -> CanProto/CanHw` 순서로, LIN 내부는 `AppCore_TaskLinFast/Poll -> LinModule_TaskFast/Poll -> RuntimeIo_Lin*` 순서로 보면 계층이 잘 보인다.

## 마지막 정리
- 이 코드베이스의 핵심은 `센서(slave2) -> 판단(master) -> 현장 반응(slave1)` 흐름이다.
- 함수 분리 또한 이 구조를 그대로 반영한다.
- 즉, `Runtime`은 시간을 나누고, `AppCore`는 흐름을 모으고, `역할 정책 함수`는 의미를 결정하고, `모듈 함수`는 실제 기능을 수행하며, `RuntimeIo`는 하드웨어에 연결한다.
- 이 관점으로 보면 함수 수가 많아 보여도, 각 함수는 대부분 "한 단계 아래 계층에 책임을 넘기기 위한 경계점" 역할을 한다.
