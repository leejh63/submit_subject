# S32K 태스크 기준 레이어 호출 맵

이 문서는 아래 형태로 정리했다.

`태스크 -> 모듈 -> 그 모듈이 부르는 함수들을 레이어별로 구분`

읽는 법:

- 가장 왼쪽은 `Runtime Layer`의 태스크 진입점이다.
- 가운데는 실제 정책이 들어 있는 `App Layer` 모듈이다.
- 오른쪽으로 갈수록 `Service -> Driver -> Platform` 순으로 내려간다.
- 콜백이나 hook처럼 다시 위로 올라오는 경로는 점선으로 표시했다.

세부 함수 전수표는 [`s32k_call_flow_report.md`](./s32k_call_flow_report.md)를 보면 된다.

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
  SVC1["Service Layer<br/>services/can_module.c<br/>CanModule_Task<br/>CanModule_SubmitPending<br/>CanModule_TryPopIncoming / TryPopResult"]
  SVC2["Service Layer<br/>services/can_service.c<br/>CanService_Task / Send* / ProcessRx"]
  SVC3["Service Layer<br/>services/can_proto.c<br/>CanProto_EncodeMessage / DecodeFrame"]
  DRV["Driver Layer<br/>drivers/can_hw.c<br/>CanHw_Task / StartTx / TryPopRx"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_can.c<br/>IsoSdk_CanSend / ReadRxFrame / GetTransferState"]
  APP2["App Module<br/>app/app_slave1.c<br/>AppSlave1_HandleCanCommand"]

  RT --> APP --> SVC1 --> SVC2 --> SVC3
  SVC2 --> DRV --> PLT
  APP --> APP2
  APP2 --> SVC1
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
  APP2["App Modules<br/>app/app_console.c<br/>AppConsole_TryPopCanCommand"]
  APP3["App Modules<br/>app/app_master.c<br/>AppMaster_HandleCanCommand"]
  SVC1["Service Layer<br/>services/can_module.c<br/>CanModule_Task / Queue* / TryPop*"]
  SVC2["Service Layer<br/>services/can_service.c<br/>CanService_Task / Send* / ProcessRx"]
  SVC3["Service Layer<br/>services/can_proto.c<br/>CanProto_EncodeMessage / DecodeFrame"]
  DRV["Driver Layer<br/>drivers/can_hw.c<br/>CanHw_Task / StartTx / TryPopRx"]
  PLT["Platform Layer<br/>platform/s32k_sdk/isosdk_can.c<br/>IsoSdk_CanSend / ReadRxFrame / GetTransferState"]

  RT --> APP1
  APP1 --> APP2
  APP1 --> APP3
  APP1 --> SVC1 --> SVC2 --> SVC3
  SVC2 --> DRV --> PLT
```

### lin_fast task

```mermaid
flowchart LR
  RT["Runtime Layer<br/>Runtime_TaskLinFast<br/>-> AppCore_TaskLinFast"]
  APP1["App Modules<br/>app/app_core.c<br/>AppCore_TaskLinFast"]
  APP2["App Modules<br/>app/app_master.c<br/>AppMaster_HandleFreshLinStatus"]
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
  APP1["App Modules<br/>app/app_core.c<br/>AppCore_TaskLinPoll"]
  APP2["App Modules<br/>app/app_console.c<br/>AppConsole_ConsumeLocalOk"]
  APP3["App Modules<br/>app/app_master.c<br/>AppMaster_RequestOk / AppMaster_AfterLinPoll"]
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
  APP1["App Modules<br/>app/app_core.c<br/>AppCore_TaskRender"]
  APP2["App Modules<br/>app/app_console.c<br/>AppConsole_SetTaskText / SetSourceText / SetValueText / Render"]
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

  CORE -. hook .-> APP --> SVC --> DRV --> PLT
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

  CORE -. hook .-> APP --> SVC --> DRV --> PLT
  PLT -. callback .-> DRV -. notify .-> SVC
```

