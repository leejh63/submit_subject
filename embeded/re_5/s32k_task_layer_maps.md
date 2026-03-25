# S32K 태스크 기준 레이어 호출 맵

이 문서는 `re_5` 기준으로 아래 형태를 정리한다.

`태스크 -> 앱 정책 -> 서비스 -> 드라이버 -> 플랫폼`

읽는 법:

- 가장 왼쪽은 `Runtime Layer`의 태스크 진입점이다.
- 가운데는 실제 정책이 들어 있는 `App Layer`와 `Service Layer`다.
- 오른쪽으로 갈수록 `Driver -> Platform` 순으로 내려간다.
- `re_5`에서는 CAN RX가 polling 완료 확인이 아니라 `FlexCAN callback` 기반이므로, CAN 쪽에는 점선 callback 경로를 같이 표시했다.

함수 단위 상세 흐름은 [s32k_project_function_variable_outline.md](./s32k_project_function_variable_outline.md)를 보면 된다.

---

## S32K_Can_slave

### button task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskButton<br/>-> AppCore_TaskButton"]
  APP["App Module<br/>app/app_slave1.c<br/>AppSlave1_TaskButton<br/>AppCore_QueueCanCommandCode"]
  RIO["Runtime Binding<br/>runtime/runtime_io.c<br/>RuntimeIo_ReadSlave1ButtonPressed"]
  DRV["Driver Layer<br/>drivers/board_hw.c<br/>BoardHw_ReadSlave1ButtonPressed"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_board.c<br/>IsoSdk_BoardReadSlave1ButtonPressed"]
  SVC["Service Layer<br/>services/can_module.c<br/>CanModule_QueueCommand"]

  RT --> APP
  APP --> RIO --> DRV --> PLT
  APP --> SVC
```

### can task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskCan<br/>-> AppCore_TaskCan"]
  APP["App Modules<br/>app/app_core.c<br/>AppCore_HandleCanIncoming"]
  APP2["App Module<br/>app/app_slave1.c<br/>AppSlave1_HandleCanCommand"]
  SVC1["Service Layer<br/>services/can_module.c<br/>CanModule_Task<br/>CanModule_SubmitPending<br/>CanModule_TryPopIncoming / TryPopResult"]
  SVC2["Service Layer<br/>services/can_service.c<br/>CanService_Task / ProcessRx / Send*"]
  SVC3["Service Layer<br/>services/can_proto.c<br/>CanProto_EncodeMessage / DecodeFrame"]
  DRV["Driver Layer<br/>drivers/can_hw.c<br/>CanHw_Task / OnIsoSdkEvent / OnRxComplete / StartTx / TryPopRx"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_can.c<br/>IsoSdk_CanInstallEventCallback<br/>IsoSdk_CanSend / ReadRxFrame / GetTransferState"]

  RT --> APP --> SVC1 --> SVC2 --> SVC3
  SVC2 --> DRV --> PLT
  APP --> APP2
  APP2 --> SVC1
  PLT -. FlexCAN callback .-> DRV
```

### led task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskLed<br/>-> AppCore_TaskLed"]
  APP["App Module<br/>app/app_slave1.c<br/>AppSlave1_TaskLed"]
  DRV["Driver Layer<br/>drivers/led_module.c<br/>LedModule_Task / GetPattern"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_board.c<br/>IsoSdk_GpioWritePin"]

  RT --> APP --> DRV --> PLT
```

### heartbeat task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskHeartbeat"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_TaskHeartbeat"]

  RT --> APP
```

### can callback / IRQ 보조 흐름

```mermaid
flowchart LR
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_can.c<br/>IsoSdk_CanSdkEventCallback / DispatchEvent"]
  DRV["Driver Layer<br/>drivers/can_hw.c<br/>CanHw_OnIsoSdkEvent / CanHw_OnRxComplete"]
  SVC["Service Layer<br/>services/can_service.c<br/>CanTransport_DrainHwRx"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_HandleCanIncoming"]

  PLT -. RX done callback .-> DRV
  DRV -. enqueue rx frame .-> SVC
  SVC --> APP
```

---

## S32K_LinCan_master

### uart task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskUart<br/>-> AppCore_TaskUart"]
  APP["App Module<br/>app/app_console.c<br/>AppConsole_Task / HandleLine / QueueCanCommand"]
  SVC["Service Layer<br/>services/uart_service.c<br/>UartService_ProcessRx / ReadLine / RequestTx / ProcessTx"]
  DRV["Driver Layer<br/>drivers/uart_hw.c<br/>UartHw_InitDefault / StartTransmit / ContinueReceiveByte"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_uart.c<br/>IsoSdk_UartInit / StartTransmit / ContinueReceiveByte"]

  RT --> APP --> SVC --> DRV --> PLT
  PLT -. RX callback .-> DRV
  DRV -. RX byte/event .-> SVC
```

### can task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskCan<br/>-> AppCore_TaskCan"]
  APP1["App Modules<br/>app/app_core.c<br/>AppCore_TaskCan / AppCore_HandleCanIncoming"]
  APP2["App Module<br/>app/app_console.c<br/>AppConsole_TryPopCanCommand"]
  APP3["App Module<br/>app/app_master.c<br/>AppMaster_HandleCanCommand"]
  SVC1["Service Layer<br/>services/can_module.c<br/>CanModule_Task / Queue* / TryPop*"]
  SVC2["Service Layer<br/>services/can_service.c<br/>CanService_Task / ProcessRx / Send*"]
  SVC3["Service Layer<br/>services/can_proto.c<br/>CanProto_EncodeMessage / DecodeFrame"]
  DRV["Driver Layer<br/>drivers/can_hw.c<br/>CanHw_Task / OnIsoSdkEvent / OnRxComplete / StartTx / TryPopRx"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_can.c<br/>IsoSdk_CanInstallEventCallback<br/>IsoSdk_CanSend / ReadRxFrame / GetTransferState"]

  RT --> APP1
  APP1 --> APP2
  APP1 --> APP3
  APP1 --> SVC1 --> SVC2 --> SVC3
  SVC2 --> DRV --> PLT
  PLT -. FlexCAN callback .-> DRV
```

### lin_fast task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskLinFast<br/>-> AppCore_TaskLinFast"]
  APP1["App Module<br/>app/app_core.c<br/>AppCore_TaskLinFast"]
  APP2["App Module<br/>app/app_master.c<br/>AppMaster_HandleFreshLinStatus"]
  SVC["Service Layer<br/>services/lin_module.c<br/>LinModule_TaskFast / ConsumeFreshStatus / ParseStatusRx"]
  DRV["Driver Layer<br/>drivers/lin_hw.c<br/>LinHw_StartReceive / OnIsoSdkEvent"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_lin.c<br/>IsoSdk_LinStartReceive / LIN callback"]

  RT --> APP1 --> SVC --> DRV --> PLT
  SVC --> APP2
  PLT -. LIN callback .-> DRV
  DRV -. event notify .-> SVC
```

### lin_poll task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskLinPoll<br/>-> AppCore_TaskLinPoll"]
  APP1["App Module<br/>app/app_core.c<br/>AppCore_TaskLinPoll"]
  APP2["App Module<br/>app/app_console.c<br/>AppConsole_ConsumeLocalOk"]
  APP3["App Module<br/>app/app_master.c<br/>AppMaster_RequestOk / AppMaster_AfterLinPoll"]
  SVC["Service Layer<br/>services/lin_module.c<br/>LinModule_TaskPoll / RequestOk / MasterStart"]
  DRV["Driver Layer<br/>drivers/lin_hw.c<br/>LinHw_MasterSendHeader"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_lin.c<br/>IsoSdk_LinMasterSendHeader"]

  RT --> APP1 --> APP2
  APP1 --> APP3
  APP3 --> SVC --> DRV --> PLT
```

### render task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskRender<br/>-> AppCore_TaskRender"]
  APP1["App Module<br/>app/app_core.c<br/>AppCore_TaskRender"]
  APP2["App Module<br/>app/app_console.c<br/>AppConsole_SetTaskText / SetSourceText / SetValueText / Render"]
  SVC["Service Layer<br/>services/uart_service.c<br/>UartService_RequestTx / ProcessTx"]
  DRV["Driver Layer<br/>drivers/uart_hw.c<br/>UartHw_StartTransmit"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_uart.c<br/>IsoSdk_UartStartTransmit"]

  RT --> APP1 --> APP2 --> SVC --> DRV --> PLT
```

### heartbeat task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskHeartbeat"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_TaskHeartbeat"]

  RT --> APP
```

### tick hook / callback 보조 흐름

```mermaid
flowchart LR
  CORE["Core Layer<br/>core/runtime_tick.c<br/>RuntimeTick_IrqHandler"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_OnTickIsr"]
  SVC["Service Layer<br/>services/lin_module.c<br/>LinModule_OnBaseTick"]
  DRV["Driver Layer<br/>drivers/lin_hw.c<br/>LinHw_ServiceTick"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_lin.c<br/>IsoSdk_LinServiceTick"]

  CORE -. tick hook .-> APP --> SVC --> DRV --> PLT
```

### can callback / IRQ 보조 흐름

```mermaid
flowchart LR
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_can.c<br/>IsoSdk_CanSdkEventCallback / DispatchEvent"]
  DRV["Driver Layer<br/>drivers/can_hw.c<br/>CanHw_OnIsoSdkEvent / CanHw_OnRxComplete"]
  SVC["Service Layer<br/>services/can_service.c<br/>CanTransport_DrainHwRx / ProcessRx"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_HandleCanIncoming / AppMaster_HandleCanCommand"]

  PLT -. RX done callback .-> DRV
  DRV -. enqueue rx frame .-> SVC
  SVC --> APP
```

---

## S32K_Lin_slave

### lin_fast task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskLinFast<br/>-> AppCore_TaskLinFast"]
  APP["App Module<br/>app/app_slave2.c<br/>AppSlave2_HandleLinOkToken"]
  SVC1["Service Layer<br/>services/lin_module.c<br/>LinModule_ConsumeSlaveOkToken"]
  SVC2["Service Layer<br/>services/adc_module.c<br/>AdcModule_ClearEmergencyLatch"]
  DRV["Driver Layer<br/>drivers/lin_hw.c<br/>LinHw_OnIsoSdkEvent"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_lin.c<br/>LIN callback"]

  RT --> APP --> SVC1
  APP --> SVC2
  PLT -. LIN callback .-> DRV
  DRV -. event notify .-> SVC1
```

### adc task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskAdc<br/>-> AppCore_TaskAdc"]
  APP["App Module<br/>app/app_slave2.c<br/>AppSlave2_TaskAdc"]
  SVC1["Service Layer<br/>services/adc_module.c<br/>AdcModule_Task / GetSnapshot / ZoneText"]
  SVC2["Service Layer<br/>services/lin_module.c<br/>LinModule_SetSlaveStatus"]
  DRV1["Driver Layer<br/>drivers/adc_hw.c<br/>AdcHw_Sample"]
  DRV2["Driver Layer<br/>drivers/led_module.c<br/>LedModule_SetPattern"]
  PLT1["Platform Layer<br/>platform/s32k_sdk/isosdk_adc.c<br/>IsoSdk_AdcSample"]
  PLT2["Platform Layer<br/>platform/s32k_sdk/isosdk_board.c<br/>IsoSdk_GpioWritePin"]

  RT --> APP --> SVC1 --> DRV1 --> PLT1
  APP --> SVC2
  APP --> DRV2 --> PLT2
```

### led task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskLed<br/>-> AppCore_TaskLed"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_TaskLed"]
  DRV["Driver Layer<br/>drivers/led_module.c<br/>LedModule_Task"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_board.c<br/>IsoSdk_GpioWritePin"]

  RT --> APP --> DRV --> PLT
```

### heartbeat task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskHeartbeat"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_TaskHeartbeat"]

  RT --> APP
```

### tick hook / callback 보조 흐름

```mermaid
flowchart LR
  CORE["Core Layer<br/>core/runtime_tick.c<br/>RuntimeTick_IrqHandler"]
  APP["App Layer<br/>app/app_core.c<br/>AppCore_OnTickIsr"]
  SVC["Service Layer<br/>services/lin_module.c<br/>LinModule_OnBaseTick / OnEvent"]
  DRV["Driver Layer<br/>drivers/lin_hw.c<br/>LinHw_ServiceTick / OnIsoSdkEvent"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_lin.c<br/>IsoSdk_LinServiceTick / LIN callback"]

  CORE -. tick hook .-> APP --> SVC --> DRV --> PLT
  PLT -. callback .-> DRV -. notify .-> SVC
```

---

## 빠른 요약

- `S32K_Can_slave`
  - 버튼과 CAN 명령을 받아 LED와 CAN 응답으로 반응한다.
  - `re_5`에서는 CAN RX 완료가 callback으로 `drivers/can_hw.c`에 먼저 들어온다.

- `S32K_LinCan_master`
  - UART console, CAN, LIN을 모두 엮는 orchestration 중심 노드다.
  - CAN/LIN/UART 모두 callback 또는 hook 보조 흐름이 있어 레이어 역방향 경로가 가장 많다.

- `S32K_Lin_slave`
  - ADC 해석 결과를 LIN 상태로 게시하는 센서 노드다.
  - 레이어 구조는 가장 단순하지만 tick hook + LIN callback 흐름은 분명히 존재한다.
