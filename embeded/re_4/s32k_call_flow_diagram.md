# S32K 박스/화살표 호출 다이어그램

이 문서는 [`s32k_call_flow_report.md`](./s32k_call_flow_report.md)를 바탕으로 다시 그린 다이어그램 전용 요약본이다.

- 목적: 함수 관계를 글보다 박스/화살표로 한눈에 보기
- 방식: `main -> runtime -> app -> services -> drivers -> isosdk` 흐름과 이벤트 흐름을 나눠서 표시
- 참고: 세부 caller/callee 전수표는 기존 [`s32k_call_flow_report.md`](./s32k_call_flow_report.md)에 그대로 있다.

## 전체 3노드 상호작용

```mermaid
flowchart LR
  subgraph SLAVE2["S32K_Lin_slave / Slave2"]
    S2_ADC["AppSlave2_TaskAdc<br/>ADC 샘플링"]
    S2_PUB["LinModule_SetSlaveStatus<br/>LIN 상태 게시"]
    S2_OK["AppSlave2_HandleLinOkToken<br/>OK token 소비"]
  end

  subgraph MASTER["S32K_LinCan_master / Master"]
    M_POLL["AppCore_TaskLinPoll<br/>LIN poll 시작"]
    M_FAST["AppCore_TaskLinFast<br/>LIN 결과 소비"]
    M_DECIDE["AppMaster_HandleFreshLinStatus<br/>emergency 판단"]
    M_CANRX["AppMaster_HandleCanCommand<br/>slave1 OK 요청 수신"]
    M_REQ["AppMaster_RequestOk<br/>승인 가능 여부 판단"]
  end

  subgraph SLAVE1["S32K_Can_slave / Slave1"]
    S1_CAN["AppSlave1_HandleCanCommand<br/>CAN 명령 반영"]
    S1_BTN["AppSlave1_TaskButton<br/>버튼 debounce"]
  end

  S2_ADC --> S2_PUB --> M_POLL
  M_POLL --> M_FAST --> M_DECIDE
  M_DECIDE -->|"CAN_CMD_EMERGENCY"| S1_CAN
  S1_CAN --> S1_BTN
  S1_BTN -->|"CAN_CMD_OK"| M_CANRX
  M_CANRX --> M_REQ -->|"LIN OK token"| S2_OK
  M_DECIDE -->|"latch 해제 확인 후 CAN_CMD_OK"| S1_CAN
```

## S32K_Can_slave

### 1. 초기화 흐름

```mermaid
flowchart TD
  A["main"] --> B["Runtime_Init"]

  B --> C["RuntimeIo_BoardInit"]
  C --> D["BoardHw_Init"]
  D --> E["IsoSdk_BoardInit"]

  B --> F["RuntimeTick_Init"]
  F --> G["TickHw_Init"]
  G --> H["IsoSdk_TickInit"]

  B --> I["AppCore_Init"]
  I --> J["RuntimeIo_GetLocalNodeId"]
  I --> K["AppCore_InitDefaultTexts"]
  I --> L["AppSlave1_Init"]

  L --> M["AppCore_InitConsoleCan"]
  M --> N["CanModule_Init"]
  N --> O["InfraQueue_Init"]
  N --> P["CanService_Init"]
  P --> Q["CanProto_Init"]
  P --> R["CanTransport_Init"]
  R --> S["CanHw_InitDefault"]
  S --> T["IsoSdk_CanInitController"]
  S --> U["IsoSdk_CanInitTxMailbox"]
  S --> V["IsoSdk_CanConfigRxAcceptAll"]
  S --> W["IsoSdk_CanInitRxMailbox"]
  S --> X["IsoSdk_CanStartReceive"]

  L --> Y["RuntimeIo_GetSlave1LedConfig"]
  Y --> Z["BoardHw_GetRgbLedConfig"]
  Z --> ZA["IsoSdk_BoardGetRgbLed*"]
  L --> ZB["LedModule_Init"]
  ZB --> ZC["IsoSdk_GpioSetPinsDirectionMask"]

  B --> ZD["Runtime_BuildTaskTable"]
  B --> ZE["RuntimeTask_ResetTable"]
```

### 2. Super-loop와 task 분기

```mermaid
flowchart TD
  A["main"] --> B["Runtime_Run"]
  B --> C["RuntimeTask_RunDue"]

  C --> D["Runtime_TaskButton"]
  D --> E["AppCore_TaskButton"]
  E --> F["AppSlave1_TaskButton"]

  C --> G["Runtime_TaskCan"]
  G --> H["AppCore_TaskCan"]
  H --> I["CanModule_Task"]
  I --> J["CanService_Task"]
  J --> K["CanTransport_Task"]
  K --> L["CanHw_Task"]

  C --> M["Runtime_TaskLed"]
  M --> N["AppCore_TaskLed"]
  N --> O["AppSlave1_TaskLed"]

  C --> P["Runtime_TaskHeartbeat"]
  P --> Q["AppCore_TaskHeartbeat"]
```

### 3. 버튼 입력이 master로 `CAN_CMD_OK` 가는 흐름

```mermaid
flowchart TD
  A["AppSlave1_TaskButton"] --> B["RuntimeIo_ReadSlave1ButtonPressed"]
  B --> C["BoardHw_ReadSlave1ButtonPressed"]
  C --> D["IsoSdk_BoardReadSlave1ButtonPressed"]

  A --> E{"stable pressed?<br/>mode == EMERGENCY?"}
  E -->|No| F["return"]
  E -->|Yes| G["AppCore_QueueCanCommandCode"]
  G --> H["CanModule_QueueCommand"]
  H --> I["request_queue 적재"]

  I --> J["다음 tick의 AppCore_TaskCan"]
  J --> K["CanModule_Task"]
  K --> L["CanModule_SubmitPending"]
  L --> M["CanService_SendCommand"]
  M --> N["CanService_SendMessage"]
  N --> O["CanProto_EncodeMessage"]
  N --> P["CanTransport_SendFrame"]
  L --> Q["CanService_FlushTx"]
  Q --> R["CanTransport_Task"]
  R --> S["CanTransport_ProcessTx"]
  S --> T["CanHw_StartTx"]
  T --> U["IsoSdk_CanSend"]
```

### 4. CAN 수신 명령이 LED/응답으로 반영되는 흐름

```mermaid
flowchart TD
  A["CanHw_Task"] --> B["IsoSdk_CanGetTransferState(RX)"]
  B --> C["CanHw_CopyIsoSdkRxToFrame"]
  C --> D["IsoSdk_CanReadRxFrame"]
  C --> E["CanHw_RxQueuePush"]

  E --> F["CanTransport_DrainHwRx"]
  F --> G["CanService_ProcessRx"]
  G --> H["CanProto_DecodeFrame"]
  G --> I["CanService_ProcessDecodedMessage"]
  I --> J["CanService_IncomingQueuePush"]

  J --> K["AppCore_TaskCan"]
  K --> L["CanModule_TryPopIncoming"]
  L --> M["AppCore_HandleCanIncoming"]
  M --> N["AppSlave1_HandleCanCommand"]

  N --> O["LedModule_SetPattern<br/>또는<br/>LedModule_StartGreenAckBlink"]
  N --> P["AppCore_SetModeText / SetButtonText / SetCanInputText"]

  M --> Q{"response 필요?"}
  Q -->|Yes| R["CanModule_QueueResponse"]
  R --> S["CanService_SendResponse"]
  S --> T["CanProto_EncodeMessage"]
  T --> U["CanHw_StartTx"]
  U --> V["IsoSdk_CanSend"]
  Q -->|No| W["종료"]
```

## S32K_LinCan_master

### 1. 초기화 흐름

```mermaid
flowchart TD
  A["main"] --> B["Runtime_Init"]

  B --> C["RuntimeIo_BoardInit"]
  C --> D["BoardHw_Init"]
  C --> E["BoardHw_EnableLinTransceiver"]
  D --> F["IsoSdk_BoardInit"]
  E --> G["IsoSdk_BoardEnableLinTransceiver"]

  B --> H["RuntimeTick_Init"]
  H --> I["TickHw_Init"]
  I --> J["IsoSdk_TickInit"]

  B --> K["AppCore_Init"]
  K --> L["RuntimeIo_GetLocalNodeId"]
  K --> M["AppCore_InitDefaultTexts"]
  K --> N["AppMaster_Init"]

  N --> O["AppConsole_Init"]
  O --> P["InfraQueue_Init"]
  O --> Q["UartService_Init"]
  Q --> R["UartHw_InitDefault"]
  R --> S["IsoSdk_UartInit"]
  R --> T["IsoSdk_UartStartReceiveByte"]

  N --> U["CanModule_Init"]
  U --> V["CanService_Init"]
  V --> W["CanProto_Init"]
  V --> X["CanTransport_Init"]
  X --> Y["CanHw_InitDefault"]

  N --> Z["RuntimeIo_GetMasterLinConfig"]
  Z --> ZA["LinHw_Configure"]
  N --> ZB["LinModule_Init"]
  ZB --> ZC["LinHw_Init"]
  ZC --> ZD["IsoSdk_LinInit"]
  N --> ZE["RuntimeIo_AttachLinModule"]
  ZE --> ZF["LinHw_AttachModule"]

  B --> ZG["RuntimeTick_ClearHooks"]
  B --> ZH["RuntimeTick_RegisterHook(AppCore_OnTickIsr)"]
  B --> ZI["Runtime_BuildTaskTable"]
  B --> ZJ["RuntimeTask_ResetTable"]
```

### 2. Super-loop와 task 분기

```mermaid
flowchart TD
  A["Runtime_Run"] --> B["RuntimeTask_RunDue"]

  B --> C["Runtime_TaskUart"]
  C --> D["AppCore_TaskUart"]
  D --> E["AppConsole_Task"]

  B --> F["Runtime_TaskLinFast"]
  F --> G["AppCore_TaskLinFast"]
  G --> H["LinModule_TaskFast"]
  H --> I["LinModule_ConsumeFreshStatus"]
  I --> J["AppMaster_HandleFreshLinStatus"]

  B --> K["Runtime_TaskCan"]
  K --> L["AppCore_TaskCan"]
  L --> M["CanModule_Task"]

  B --> N["Runtime_TaskLinPoll"]
  N --> O["AppCore_TaskLinPoll"]
  O --> P["LinModule_TaskPoll"]
  O --> Q["AppMaster_AfterLinPoll"]

  B --> R["Runtime_TaskRender"]
  R --> S["AppCore_TaskRender"]
  S --> T["AppConsole_Render"]

  B --> U["Runtime_TaskHeartbeat"]
  U --> V["AppCore_TaskHeartbeat"]
```

### 3. UART 콘솔 명령이 CAN 송신으로 이어지는 흐름

```mermaid
flowchart TD
  A["AppCore_TaskUart"] --> B["AppConsole_Task"]
  B --> C["UartService_ProcessRx"]
  C --> D["UartService_ReadLine"]
  D --> E["AppConsole_HandleLine"]
  E --> F["AppConsole_QueueCanCommand"]
  F --> G["InfraQueue_Push"]

  G --> H["다음 tick의 AppCore_TaskCan"]
  H --> I["AppConsole_TryPopCanCommand"]
  I --> J{"명령 종류"}

  J -->|OPEN/CLOSE/OFF/TEST| K["CanModule_QueueCommand"]
  J -->|TEXT| L["CanModule_QueueText"]
  J -->|EVENT| M["CanModule_QueueEvent"]

  K --> N["CanModule_SubmitPending"]
  L --> N
  M --> N

  N --> O["CanService_SendCommand / SendText / SendEvent"]
  O --> P["CanService_SendMessage"]
  P --> Q["CanProto_EncodeMessage"]
  P --> R["CanTransport_SendFrame"]
  R --> S["CanHw_StartTx"]
  S --> T["IsoSdk_CanSend"]

  H --> U["CanModule_TryPopResult"]
  U --> V["AppCore_FormatCanResult"]
  V --> W["AppCore_SetResultText"]
  W --> X["AppConsole_SetResultText"]
```

### 4. LIN status polling과 emergency 판단 흐름

```mermaid
flowchart TD
  A["AppCore_TaskLinPoll"] --> B["LinModule_TaskPoll"]
  B --> C["LinModule_MasterStart(pid_status)"]
  C --> D["LinHw_MasterSendHeader"]
  D --> E["IsoSdk_LinMasterSendHeader"]

  F["RuntimeTick_IrqHandler"] --> G["등록된 hook 호출"]
  G --> H["AppCore_OnTickIsr"]
  H --> I["LinModule_OnBaseTick"]
  I --> J["LinHw_ServiceTick"]
  J --> K["IsoSdk_LinServiceTick"]

  L["IsoSdk LIN callback"] --> M["LinHw_OnIsoSdkEvent"]
  M --> N["LinModule_OnEvent"]
  N --> O["PID_OK / RX_DONE / TX_DONE flag 기록"]

  P["AppCore_TaskLinFast"] --> Q["LinModule_TaskFast"]
  Q --> R["LinHw_StartReceive"]
  R --> S["IsoSdk_LinStartReceive"]
  Q --> T["LinModule_ParseStatusRx"]
  T --> U["LinModule_ConsumeFreshStatus"]
  U --> V["AppMaster_HandleFreshLinStatus"]

  V --> W{"zone == EMERGENCY<br/>or latch == 1 ?"}
  W -->|Yes| X["master_emergency_active = 1"]
  X --> Y["AppCore_QueueCanCommandCode<br/>(slave1, CAN_CMD_EMERGENCY)"]
  Y --> Z["CanModule_QueueCommand"]

  W -->|No| ZA["normal 상태 유지"]
  V --> ZB["ADC/LIN UI 문자열 갱신"]
```

### 5. slave1 OK 요청 -> slave2 OK token -> slave1 승인 흐름

```mermaid
flowchart TD
  A["AppCore_TaskCan"] --> B["CanModule_TryPopIncoming"]
  B --> C["AppCore_HandleCanIncoming"]
  C --> D["AppMaster_HandleCanCommand"]

  D --> E{"source == slave1<br/>payload[0] == CAN_CMD_OK ?"}
  E -->|No| F["기타 command 처리"]
  E -->|Yes| G["AppMaster_RequestOk"]

  G --> H["LinModule_GetLatestStatus"]
  H --> I{"zone == EMERGENCY ?"}
  I -->|Yes| J["deny / wait"]
  I -->|No| K{"emergency_latched == 1 ?"}
  K -->|No| L["already clear"]
  K -->|Yes| M["master_slave1_ok_pending = 1"]
  M --> N["LinModule_RequestOk"]

  N --> O["다음 AppCore_TaskLinPoll"]
  O --> P["LinModule_TaskPoll"]
  P --> Q["LinModule_MasterStart(pid_ok)"]
  Q --> R["LinHw_MasterSendHeader"]
  R --> S["IsoSdk_LinMasterSendHeader"]

  T["이후 fresh LIN status 도착"] --> U["AppMaster_HandleFreshLinStatus"]
  U --> V{"pending && latch == 0 && zone != EMERGENCY ?"}
  V -->|Yes| W["AppCore_QueueCanCommandCode<br/>(slave1, CAN_CMD_OK)"]
  W --> X["CanModule_QueueCommand"]
  X --> Y["slave1 승인 완료"]
  V -->|No| Z["AppMaster_AfterLinPoll -> LinModule_RequestOk 재시도"]
```

## S32K_Lin_slave

### 1. 초기화 흐름

```mermaid
flowchart TD
  A["main"] --> B["Runtime_Init"]

  B --> C["RuntimeIo_BoardInit"]
  C --> D["BoardHw_Init"]
  C --> E["BoardHw_EnableLinTransceiver"]
  D --> F["IsoSdk_BoardInit"]
  E --> G["IsoSdk_BoardEnableLinTransceiver"]

  B --> H["RuntimeTick_Init"]
  H --> I["TickHw_Init"]
  I --> J["IsoSdk_TickInit"]

  B --> K["AppCore_Init"]
  K --> L["RuntimeIo_GetLocalNodeId"]
  K --> M["AppCore_InitDefaultTexts"]
  K --> N["AppSlave2_Init"]

  N --> O["RuntimeIo_GetSlave2LedConfig"]
  O --> P["BoardHw_GetRgbLedConfig"]
  N --> Q["LedModule_Init"]

  N --> R["RuntimeIo_GetSlave2AdcConfig"]
  R --> S["AdcHw_IsSupported"]
  N --> T["AdcModule_Init"]
  T --> U["AdcHw_Init"]
  U --> V["IsoSdk_AdcInit"]

  N --> W["RuntimeIo_GetSlaveLinConfig"]
  W --> X["LinHw_Configure"]
  N --> Y["LinModule_Init"]
  Y --> Z["LinHw_Init"]
  Z --> ZA["IsoSdk_LinInit"]
  N --> ZB["RuntimeIo_AttachLinModule"]
  ZB --> ZC["LinHw_AttachModule"]

  B --> ZD["RuntimeTick_ClearHooks"]
  B --> ZE["RuntimeTick_RegisterHook(AppCore_OnTickIsr)"]
  B --> ZF["Runtime_BuildTaskTable"]
  B --> ZG["RuntimeTask_ResetTable"]
```

### 2. Super-loop와 task 분기

```mermaid
flowchart TD
  A["Runtime_Run"] --> B["RuntimeTask_RunDue"]

  B --> C["Runtime_TaskLinFast"]
  C --> D["AppCore_TaskLinFast"]
  D --> E["AppSlave2_HandleLinOkToken"]

  B --> F["Runtime_TaskAdc"]
  F --> G["AppCore_TaskAdc"]
  G --> H["AppSlave2_TaskAdc"]

  B --> I["Runtime_TaskLed"]
  I --> J["AppCore_TaskLed"]
  J --> K["LedModule_Task"]

  B --> L["Runtime_TaskHeartbeat"]
  L --> M["AppCore_TaskHeartbeat"]
```

### 3. ADC 샘플링 -> zone 분류 -> LIN 상태 게시 흐름

```mermaid
flowchart TD
  A["AppCore_TaskAdc"] --> B["AppSlave2_TaskAdc"]
  B --> C["AdcModule_Task"]
  C --> D["AdcHw_Sample"]
  D --> E["IsoSdk_AdcSample"]

  C --> F["AdcModule_ClassifyZone"]
  F --> G["snapshot.zone / latch 갱신"]

  B --> H["AdcModule_GetSnapshot"]
  H --> I["LinStatusFrame 구성"]
  I --> J["LinModule_SetSlaveStatus"]

  B --> K{"snapshot.emergency_latched ?"}
  K -->|Yes| L["LedModule_SetPattern(RED_BLINK)"]
  K -->|No| M{"zone == SAFE ?"}
  M -->|Yes| N["LedModule_SetPattern(GREEN_SOLID)"]
  M -->|No| O{"zone == WARNING ?"}
  O -->|Yes| P["LedModule_SetPattern(YELLOW_SOLID)"]
  O -->|No| Q["LedModule_SetPattern(RED_SOLID)"]
```

### 4. master의 status poll에 slave2가 응답하는 흐름

```mermaid
flowchart TD
  A["RuntimeTick_IrqHandler"] --> B["hook 호출"]
  B --> C["AppCore_OnTickIsr"]
  C --> D["LinModule_OnBaseTick"]
  D --> E["LinHw_ServiceTick"]
  E --> F["IsoSdk_LinServiceTick"]

  G["IsoSdk LIN callback"] --> H["LinHw_OnIsoSdkEvent"]
  H --> I["LinModule_OnEvent"]
  I --> J{"current_pid == pid_status ?"}

  J -->|Yes| K["LinModule_PrepareStatusTx"]
  K --> L["LinHw_StartSend"]
  L --> M["IsoSdk_LinStartSend"]

  J -->|No| N["다른 PID 처리"]
```

### 5. master의 OK token을 받아 latch 해제하는 흐름

```mermaid
flowchart TD
  A["IsoSdk LIN callback"] --> B["LinHw_OnIsoSdkEvent"]
  B --> C["LinModule_OnEvent"]
  C --> D{"current_pid == pid_ok ?"}

  D -->|No| E["다른 LIN 이벤트 처리"]
  D -->|Yes| F["LinHw_StartReceive"]
  F --> G["IsoSdk_LinStartReceive"]
  G --> H["RX_DONE"]
  H --> I["ok_token_pending = 1"]

  I --> J["다음 AppCore_TaskLinFast"]
  J --> K["AppSlave2_HandleLinOkToken"]
  K --> L["LinModule_ConsumeSlaveOkToken"]
  L --> M["AdcModule_ClearEmergencyLatch"]
  M --> N{"zone == EMERGENCY ?"}
  N -->|Yes| O["BUSY - latch 유지"]
  N -->|No| P["latch clear 성공"]
```

## 같이 보면 좋은 파일

- 다이어그램 요약: [`s32k_call_flow_diagram.md`](./s32k_call_flow_diagram.md)
- 상세 caller/callee 리포트: [`s32k_call_flow_report.md`](./s32k_call_flow_report.md)

