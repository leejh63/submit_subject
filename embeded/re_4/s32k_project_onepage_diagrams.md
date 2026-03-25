# S32K 프로젝트별 한장 다이어그램

이 문서는 프로젝트마다 한 장씩, 아래 순서로 흐름을 묶어서 보여준다.

- 프로젝트
- 시작
- 초기화
- 태스크
- 태스크 함수
- 각 함수들이 부르는 함수들

세부 caller/callee 표는 [`s32k_call_flow_report.md`](./s32k_call_flow_report.md)를 보면 된다.

---

## S32K_Can_slave

```mermaid
flowchart TD
  subgraph P1["프로젝트: S32K_Can_slave"]
    direction TB

    START1["시작<br/>main"]

    subgraph INIT1["초기화"]
      direction TB
      RI1["Runtime_Init"]
      RB1["RuntimeIo_BoardInit<br/>-> BoardHw_Init<br/>-> IsoSdk_BoardInit"]
      RT1["RuntimeTick_Init<br/>-> TickHw_Init<br/>-> IsoSdk_TickInit"]
      ACI1["AppCore_Init"]
      TXT1["AppCore_InitDefaultTexts"]
      ASI1["AppSlave1_Init"]
      CANI1["AppCore_InitConsoleCan<br/>-> CanModule_Init<br/>-> CanService_Init<br/>-> CanProto_Init / CanTransport_Init<br/>-> CanHw_InitDefault<br/>-> IsoSdk_Can*"]
      LEDI1["RuntimeIo_GetSlave1LedConfig<br/>-> BoardHw_GetRgbLedConfig<br/>-> LedModule_Init<br/>-> IsoSdk_GpioSetPinsDirectionMask"]
      TBL1["Runtime_BuildTaskTable<br/>-> RuntimeTask_ResetTable"]
    end

    subgraph TASK1["태스크"]
      direction TB
      RUN1["Runtime_Run<br/>-> RuntimeTask_RunDue"]
      BTN1["button task"]
      CAN1["can task"]
      LED1["led task"]
      HB1["heartbeat task"]
    end

    subgraph TFN1["태스크 함수"]
      direction TB
      TBTN1["AppCore_TaskButton<br/>-> AppSlave1_TaskButton"]
      TCAN1["AppCore_TaskCan<br/>-> CanModule_Task"]
      TLED1["AppCore_TaskLed<br/>-> AppSlave1_TaskLed"]
      THB1["AppCore_TaskHeartbeat"]
    end

    subgraph CALL1["각 함수들이 부르는 함수들"]
      direction TB
      CBTN1["버튼 입력 경로<br/>RuntimeIo_ReadSlave1ButtonPressed<br/>-> BoardHw_ReadSlave1ButtonPressed<br/>-> IsoSdk_BoardReadSlave1ButtonPressed<br/><br/>조건 충족 시<br/>AppCore_QueueCanCommandCode<br/>-> CanModule_QueueCommand"]
      CCAN1["CAN 전송/수신 경로<br/>CanModule_SubmitPending<br/>-> CanService_SendCommand / SendResponse<br/>-> CanProto_EncodeMessage<br/>-> CanTransport_SendFrame<br/>-> CanHw_StartTx<br/>-> IsoSdk_CanSend<br/><br/>CanService_ProcessRx<br/>-> CanProto_DecodeFrame<br/>-> AppCore_HandleCanIncoming<br/>-> AppSlave1_HandleCanCommand"]
      CLED1["LED 처리 경로<br/>LedModule_Task<br/>-> LedModule_GetPattern<br/>-> ACK blink 종료 시 normal 복귀"]
      CHB1["heartbeat_count 증가"]
    end
  end

  START1 --> RI1
  RI1 --> RB1
  RI1 --> RT1
  RI1 --> ACI1
  RI1 --> TBL1
  ACI1 --> TXT1
  ACI1 --> ASI1
  ASI1 --> CANI1
  ASI1 --> LEDI1

  START1 --> RUN1
  RUN1 --> BTN1 --> TBTN1 --> CBTN1
  RUN1 --> CAN1 --> TCAN1 --> CCAN1
  RUN1 --> LED1 --> TLED1 --> CLED1
  RUN1 --> HB1 --> THB1 --> CHB1
```

---

## S32K_LinCan_master

```mermaid
flowchart TD
  subgraph P2["프로젝트: S32K_LinCan_master"]
    direction TB

    START2["시작<br/>main"]

    subgraph INIT2["초기화"]
      direction TB
      RI2["Runtime_Init"]
      RB2["RuntimeIo_BoardInit<br/>-> BoardHw_Init<br/>-> BoardHw_EnableLinTransceiver<br/>-> IsoSdk_Board*"]
      RT2["RuntimeTick_Init<br/>-> TickHw_Init<br/>-> IsoSdk_TickInit"]
      ACI2["AppCore_Init"]
      TXT2["AppCore_InitDefaultTexts"]
      AMI2["AppMaster_Init"]
      UARTI2["AppConsole_Init<br/>-> UartService_Init<br/>-> UartHw_InitDefault<br/>-> IsoSdk_UartInit / StartReceiveByte"]
      CANI2["CanModule_Init<br/>-> CanService_Init<br/>-> CanProto_Init / CanTransport_Init<br/>-> CanHw_InitDefault<br/>-> IsoSdk_Can*"]
      LINI2["RuntimeIo_GetMasterLinConfig<br/>-> LinHw_Configure<br/>-> LinModule_Init<br/>-> LinHw_Init<br/>-> IsoSdk_LinInit<br/>-> RuntimeIo_AttachLinModule<br/>-> LinHw_AttachModule"]
      HOOK2["RuntimeTick_ClearHooks<br/>-> RuntimeTick_RegisterHook(AppCore_OnTickIsr)"]
      TBL2["Runtime_BuildTaskTable<br/>-> RuntimeTask_ResetTable"]
    end

    subgraph TASK2["태스크"]
      direction TB
      RUN2["Runtime_Run<br/>-> RuntimeTask_RunDue"]
      UART2["uart task"]
      LINF2["lin_fast task"]
      CAN2["can task"]
      LINP2["lin_poll task"]
      REND2["render task"]
      HB2["heartbeat task"]
    end

    subgraph TFN2["태스크 함수"]
      direction TB
      TUART2["AppCore_TaskUart<br/>-> AppConsole_Task"]
      TLINF2["AppCore_TaskLinFast<br/>-> LinModule_TaskFast"]
      TCAN2["AppCore_TaskCan<br/>-> CanModule_Task"]
      TLINP2["AppCore_TaskLinPoll<br/>-> LinModule_TaskPoll<br/>+ AppMaster_AfterLinPoll"]
      TREND2["AppCore_TaskRender<br/>-> AppConsole_Render"]
      THB2["AppCore_TaskHeartbeat"]
    end

    subgraph CALL2["각 함수들이 부르는 함수들"]
      direction TB
      CUART2["UART 콘솔 경로<br/>UartService_ProcessRx / ReadLine<br/>-> AppConsole_HandleLine<br/>-> AppConsole_QueueCanCommand<br/>-> InfraQueue_Push"]
      CLINF2["LIN fast 경로<br/>LinModule_ConsumeFreshStatus<br/>-> AppMaster_HandleFreshLinStatus<br/>-> emergency 판단<br/>-> AppCore_QueueCanCommandCode(slave1, CAN_CMD_EMERGENCY 또는 CAN_CMD_OK)"]
      CCAN2["CAN 경로<br/>AppConsole_TryPopCanCommand<br/>-> CanModule_QueueCommand / QueueText / QueueEvent<br/>-> CanModule_SubmitPending<br/>-> CanService_Send*<br/>-> CanProto_EncodeMessage<br/>-> CanHw_StartTx<br/>-> IsoSdk_CanSend<br/><br/>CanModule_TryPopIncoming<br/>-> AppCore_HandleCanIncoming<br/>-> AppMaster_HandleCanCommand"]
      CLINP2["LIN poll 경로<br/>AppConsole_ConsumeLocalOk<br/>-> AppMaster_RequestOk<br/>-> LinModule_RequestOk<br/>-> LinModule_MasterStart(pid_status 또는 pid_ok)<br/>-> LinHw_MasterSendHeader<br/>-> IsoSdk_LinMasterSendHeader"]
      CREND2["렌더 경로<br/>AppConsole_SetTaskText / SetSourceText / SetValueText<br/>-> AppConsole_Render"]
      CHB2["heartbeat_count 증가"]
    end
  end

  START2 --> RI2
  RI2 --> RB2
  RI2 --> RT2
  RI2 --> ACI2
  RI2 --> HOOK2
  RI2 --> TBL2
  ACI2 --> TXT2
  ACI2 --> AMI2
  AMI2 --> UARTI2
  AMI2 --> CANI2
  AMI2 --> LINI2

  START2 --> RUN2
  RUN2 --> UART2 --> TUART2 --> CUART2
  RUN2 --> LINF2 --> TLINF2 --> CLINF2
  RUN2 --> CAN2 --> TCAN2 --> CCAN2
  RUN2 --> LINP2 --> TLINP2 --> CLINP2
  RUN2 --> REND2 --> TREND2 --> CREND2
  RUN2 --> HB2 --> THB2 --> CHB2
```

---

## S32K_Lin_slave

```mermaid
flowchart TD
  subgraph P3["프로젝트: S32K_Lin_slave"]
    direction TB

    START3["시작<br/>main"]

    subgraph INIT3["초기화"]
      direction TB
      RI3["Runtime_Init"]
      RB3["RuntimeIo_BoardInit<br/>-> BoardHw_Init<br/>-> BoardHw_EnableLinTransceiver<br/>-> IsoSdk_Board*"]
      RT3["RuntimeTick_Init<br/>-> TickHw_Init<br/>-> IsoSdk_TickInit"]
      ACI3["AppCore_Init"]
      TXT3["AppCore_InitDefaultTexts"]
      ASI3["AppSlave2_Init"]
      LEDI3["RuntimeIo_GetSlave2LedConfig<br/>-> BoardHw_GetRgbLedConfig<br/>-> LedModule_Init"]
      ADCI3["RuntimeIo_GetSlave2AdcConfig<br/>-> AdcModule_Init<br/>-> AdcHw_Init<br/>-> IsoSdk_AdcInit"]
      LINI3["RuntimeIo_GetSlaveLinConfig<br/>-> LinHw_Configure<br/>-> LinModule_Init<br/>-> LinHw_Init<br/>-> IsoSdk_LinInit<br/>-> RuntimeIo_AttachLinModule<br/>-> LinHw_AttachModule"]
      HOOK3["RuntimeTick_ClearHooks<br/>-> RuntimeTick_RegisterHook(AppCore_OnTickIsr)"]
      TBL3["Runtime_BuildTaskTable<br/>-> RuntimeTask_ResetTable"]
    end

    subgraph TASK3["태스크"]
      direction TB
      RUN3["Runtime_Run<br/>-> RuntimeTask_RunDue"]
      LINF3["lin_fast task"]
      ADC3["adc task"]
      LED3["led task"]
      HB3["heartbeat task"]
    end

    subgraph TFN3["태스크 함수"]
      direction TB
      TLINF3["AppCore_TaskLinFast<br/>-> AppSlave2_HandleLinOkToken"]
      TADC3["AppCore_TaskAdc<br/>-> AppSlave2_TaskAdc"]
      TLED3["AppCore_TaskLed<br/>-> LedModule_Task"]
      THB3["AppCore_TaskHeartbeat"]
    end

    subgraph CALL3["각 함수들이 부르는 함수들"]
      direction TB
      CLINF3["OK token 처리 경로<br/>LinModule_ConsumeSlaveOkToken<br/>-> AdcModule_ClearEmergencyLatch"]
      CADC3["ADC + LIN 게시 경로<br/>AdcModule_Task<br/>-> AdcHw_Sample<br/>-> IsoSdk_AdcSample<br/>-> AdcModule_ClassifyZone<br/>-> AdcModule_GetSnapshot<br/>-> LinModule_SetSlaveStatus<br/>-> LedModule_SetPattern"]
      CLED3["LED 처리 경로<br/>LedModule_Task"]
      CHB3["heartbeat_count 증가"]
    end
  end

  START3 --> RI3
  RI3 --> RB3
  RI3 --> RT3
  RI3 --> ACI3
  RI3 --> HOOK3
  RI3 --> TBL3
  ACI3 --> TXT3
  ACI3 --> ASI3
  ASI3 --> LEDI3
  ASI3 --> ADCI3
  ASI3 --> LINI3

  START3 --> RUN3
  RUN3 --> LINF3 --> TLINF3 --> CLINF3
  RUN3 --> ADC3 --> TADC3 --> CADC3
  RUN3 --> LED3 --> TLED3 --> CLED3
  RUN3 --> HB3 --> THB3 --> CHB3
```

