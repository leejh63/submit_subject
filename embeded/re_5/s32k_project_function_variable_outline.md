# S32K 세 프로젝트 함수/변수 정리

정리 대상:
- `S32K_Can_slave`
- `S32K_Lin_slave`
- `S32K_LinCan_master`

읽는 법:
- `>`: 직접 호출 흐름
- `-`: 함수 안에서 바뀌는 상태, 핵심 메모, 중요한 변수

주의:
- `re_5` 기준 CAN RX는 `re_4` 문서처럼 task polling으로 완료를 읽는 방식이 아니라, `FLEXCAN` event callback 기반이다.
- 즉 `CanHw_Task()`는 RX 완료 프레임을 직접 읽어 queue에 넣지 않고, interrupt callback이 먼저 `hw->rx_queue`에 적재한다.
- TX 완료 확인과 RX error recovery는 여전히 task 쪽에서 상태를 본다.

## 1. S32K_Can_slave

역할:
- node id = `2`
- slave1 현장 반응 노드
- 버튼 입력과 CAN 명령으로 LED 상태를 제어

task 주기:
- `button = 10ms`
- `can = 10ms`
- `led = 100ms`
- `heartbeat = 1000ms`

```text
ISR
> RuntimeTick_IrqHandler
    - 500us base tick 누적
    - g_runtime_tick_base_count++
    - g_runtime_tick_us_accumulator += 500
    - 1000us 이상이면
        - g_runtime_tick_ms++
        - g_runtime_tick_us_accumulator -= 1000
    - 등록 hook 순회
        - can_slave는 RuntimeTick_RegisterHook를 쓰지 않으므로 실제 hook 호출은 없음
    > TickHw_ClearCompareFlag
        > IsoSdk_TickClearCompareFlag
            > LPTMR_DRV_ClearCompareFlag

CAN callback (FlexCAN interrupt)
> IsoSdk_CanSdkEventCallback
    > IsoSdk_CanDispatchEvent
        > CanHw_OnIsoSdkEvent
            - event == RX_DONE && mb_index == rx_mb_index 이면
                > CanHw_OnRxComplete
                    > IsoSdk_CanReadRxFrame
                    > CanHw_RxQueuePush
                    > CanHw_StartReceive
                        > IsoSdk_CanStartReceive
                            > FLEXCAN_DRV_Receive

> IsoSdk_CanSdkErrorCallback
    > IsoSdk_CanDispatchEvent
        > CanHw_OnIsoSdkEvent
            - last_error = CAN_HW_ERROR_RX_STATUS_FAIL

main
> Runtime_Init
    > RuntimeIo_BoardInit
        > BoardHw_Init
            > IsoSdk_BoardInit
                > CLOCK_SYS_Init
                > CLOCK_SYS_UpdateConfiguration
                > PINS_DRV_Init

    > RuntimeTick_Init
        - init val
            - g_runtime_tick_ms
            - g_runtime_tick_base_count
            - g_runtime_tick_us_accumulator
            - g_runtime_tick_hooks[]
        > TickHw_Init
            > IsoSdk_TickInit
                > LPTMR_DRV_Init
                > INT_SYS_InstallHandler
                    - RuntimeTick_IrqHandler
                > INT_SYS_EnableIRQ
                > LPTMR_DRV_StartCounter

    > AppCore_Init
        > RuntimeIo_GetLocalNodeId
            - APP_NODE_ID_SLAVE1(2) 반환

        > AppCore_InitDefaultTexts
            > AppCore_SetModeText
            > AppCore_SetButtonText
            > AppCore_SetAdcText
            > AppCore_SetCanInputText

        > AppSlave1_Init
            > AppCore_InitConsoleCan
                - 이름은 legacy
                - master에서는 console + CAN init
                - can_slave에서는 실제로 CAN만 init
                > CanModule_Init
                    > InfraQueue_Init
                    > CanService_Init
                        > CanProto_Init
                        > CanTransport_Init
                            > CanHw_InitDefault
                                > IsoSdk_CanIsSupported
                                    - 1 return / CAN 사용 가능 여부
                                > IsoSdk_CanGetDefaultInstance
                                    - ISOSDK_SDK_CAN_INSTANCE 반환
                                > IsoSdk_CanInitController
                                    > FLEXCAN_DRV_Init
                                > IsoSdk_CanInstallEventCallback
                                    > FLEXCAN_DRV_InstallEventCallback
                                    > FLEXCAN_DRV_InstallErrorCallback
                                > IsoSdk_CanInitTxMailbox
                                    > IsoSdk_CanInitDataInfo
                                        - flexcan_data_info_t 채움
                                    > FLEXCAN_DRV_ConfigTxMb
                                > IsoSdk_CanConfigRxAcceptAll
                                    > FLEXCAN_DRV_SetRxMaskType
                                    > FLEXCAN_DRV_SetRxIndividualMask
                                > IsoSdk_CanInitRxMailbox
                                    > IsoSdk_CanInitDataInfo
                                        - flexcan_data_info_t 채움
                                    > FLEXCAN_DRV_ConfigRxMb
                                > CanHw_StartReceive
                                    > IsoSdk_CanStartReceive
                                        > FLEXCAN_DRV_Receive

            > RuntimeIo_GetSlave1LedConfig
                > BoardHw_GetRgbLedConfig
                    - config->gpio_port
                        > IsoSdk_BoardGetRgbLedPort
                    - config->red_pin
                        > IsoSdk_BoardGetRgbLedRedPin
                    - config->green_pin
                        > IsoSdk_BoardGetRgbLedGreenPin
                    - config->active_on_level
                        > IsoSdk_BoardGetRgbLedActiveOnLevel

            > LedModule_Init
                > IsoSdk_GpioSetPinsDirectionMask
                    > PINS_DRV_SetPinsDirection
                > LedModule_ApplyPattern
                    > LedModule_ApplyOutputs
                        > LedModule_WritePin
                            > IsoSdk_GpioWritePin
                                > PINS_DRV_WritePin

    > Runtime_BuildTaskTable
        - button -> Runtime_TaskButton
        - can -> Runtime_TaskCan
        - led -> Runtime_TaskLed
        - heartbeat -> Runtime_TaskHeartbeat

    > RuntimeTick_GetMs
        - g_runtime_tick_ms 반환

    > RuntimeTask_ResetTable
        - 모든 RuntimeTaskEntry.last_run_ms = start_ms

> Runtime_Run
    - initialized == 0 또는 init_status != OK 이면 Runtime_FaultLoop
    - for (;;)
        > RuntimeTask_RunDue
            - due 판단 기준
                > Infra_TimeIsDue(now_ms, last_run_ms, period_ms)

            > button due
                > Runtime_TaskButton
                    > AppCore_TaskButton
                        > AppSlave1_TaskButton
                            > RuntimeIo_ReadSlave1ButtonPressed
                                > BoardHw_ReadSlave1ButtonPressed
                                    > IsoSdk_BoardReadSlave1ButtonPressed
                                        > PINS_DRV_ReadPins
                            - debounce state 사용
                                - slave1_last_sample_pressed
                                - slave1_same_sample_count
                                - slave1_stable_pressed
                            - stable pressed 이고 mode == EMERGENCY 이면
                                > AppCore_SetButtonText
                                > AppCore_QueueCanCommandCode
                                    > CanModule_QueueCommand

            > can due
                > Runtime_TaskCan
                    > AppCore_TaskCan
                        > CanModule_Task
                            > CanService_Task
                                > CanTransport_Task
                                    > CanHw_Task
                                        - TX 완료/에러 상태 확인
                                            > IsoSdk_CanGetTransferState
                                        - RX 완료 frame 적재는 interrupt callback이 이미 수행
                                        - RX transfer error 복구만 확인
                                            > IsoSdk_CanGetTransferState
                                            > CanHw_StartReceive
                                    > CanTransport_DrainHwRx
                                        - interrupt callback이 넣어둔 hw->rx_queue를 transport queue로 이동
                                    > CanTransport_ProcessTx
                                > CanService_ProcessRx
                                > CanService_ProcessTimeouts
                            > CanModule_SubmitPending
                        > CanModule_TryPopResult
                        > CanModule_TryPopIncoming
                            > AppCore_HandleCanIncoming
                                > AppSlave1_HandleCanCommand
                                    - CAN_CMD_EMERGENCY
                                        - slave1_mode = EMERGENCY
                                        - LED_PATTERN_RED_SOLID
                                        - mode_text = "emergency"
                                    - CAN_CMD_OK
                                        - slave1_mode = ACK_BLINK
                                        - LedModule_StartGreenAckBlink
                                        - mode_text = "ack blink"
                                    - CAN_CMD_OFF
                                        - slave1_mode = NORMAL
                                        - LED_PATTERN_OFF
                                    - CAN_CMD_OPEN / CLOSE / TEST
                                        - can_input_text만 갱신
                                - need_response 이면
                                    > CanModule_QueueResponse

            > led due
                > Runtime_TaskLed
                    > AppCore_TaskLed
                        > AppSlave1_TaskLed
                            > LedModule_Task
                            - ACK blink 종료 후 pattern == OFF 이면
                                - slave1_mode = NORMAL
                                - mode_text = "normal"
                                - button_text = "waiting"

            > heartbeat due
                > Runtime_TaskHeartbeat
                    > AppCore_TaskHeartbeat
                        - heartbeat_count++
```

주요 변수:
- `g_runtime`
  - runtime 전체 상태
  - `initialized`, `init_status`, `app`, `tasks[4]`
- `g_runtime_tick_ms`
  - scheduler 기준 ms 시간
- `g_runtime_tick_base_count`
  - 500us interrupt 누적 횟수
- `g_runtime_tick_us_accumulator`
  - 500us -> 1ms 변환 누적기
- `g_runtime_tick_hooks[]`
  - ISR hook 슬롯 4개
  - can_slave에서는 초기화만 하고 미사용
- `AppCore`
  - `can_enabled`, `led1_enabled`
  - `slave1_mode`
  - `slave1_last_sample_pressed`, `slave1_stable_pressed`, `slave1_same_sample_count`
  - `heartbeat_count`
  - `can_module`, `slave1_led`
  - `mode_text`, `button_text`, `adc_text`, `can_input_text`
- `CanModule`
  - `request_queue`, `request_storage[]`
  - `service`
  - `max_submit_per_tick`
- `s_can_hw_instance`
  - CAN IRQ callback이 fallback으로 참조하는 현재 `CanHw` 포인터
- `s_iso_sdk_can_event_cb`, `s_iso_sdk_can_event_context`
  - FlexCAN driver callback을 IsoSdk 쪽에 연결해 두는 전역 상태
- `LedModule`
  - `pattern`
  - `output_phase_on`
  - `finite_blink_enabled`
  - `blink_toggles_remaining`

## 2. S32K_Lin_slave

역할:
- node id = `3`
- slave2 센서 노드
- ADC 값을 zone으로 해석하고 LIN status frame으로 게시

task 주기:
- `lin_fast = 1ms`
- `adc = 20ms`
- `led = 100ms`
- `heartbeat = 1000ms`

```text
ISR
> RuntimeTick_IrqHandler
    - 500us base tick 누적
    - g_runtime_tick_base_count++
    - g_runtime_tick_us_accumulator += 500
    - 1000us 이상이면 g_runtime_tick_ms++
    - 등록 hook 호출
        > AppCore_OnTickIsr
            > LinModule_OnBaseTick
                > LinHw_ServiceTick
                    > IsoSdk_LinServiceTick
                        > LIN_DRV_TimeoutService
    > TickHw_ClearCompareFlag
        > IsoSdk_TickClearCompareFlag
            > LPTMR_DRV_ClearCompareFlag

LIN callback
> IsoSdk_LinSdkCallback
    > IsoSdk_LinDispatchEvent
        > LinHw_OnIsoSdkEvent
            > LinModule_OnEvent
                - slave 모드에서는 PID에 따라 RX/TX 시작
                - pid_status 이면 slave_status_cache 전송
                - pid_ok 이면 ok token 수신 대기

main
> Runtime_Init
    > RuntimeIo_BoardInit
        > BoardHw_Init
            > IsoSdk_BoardInit
                > CLOCK_SYS_Init
                > CLOCK_SYS_UpdateConfiguration
                > PINS_DRV_Init
        > BoardHw_EnableLinTransceiver
            > IsoSdk_BoardEnableLinTransceiver
                > PINS_DRV_SetPinsDirection
                > PINS_DRV_SetPins

    > RuntimeTick_Init
        - init val
            - g_runtime_tick_ms
            - g_runtime_tick_base_count
            - g_runtime_tick_us_accumulator
            - g_runtime_tick_hooks[]
        > TickHw_Init
            > IsoSdk_TickInit
                > LPTMR_DRV_Init
                > INT_SYS_InstallHandler
                    - RuntimeTick_IrqHandler
                > INT_SYS_EnableIRQ
                > LPTMR_DRV_StartCounter

    > AppCore_Init
        > RuntimeIo_GetLocalNodeId
            - APP_NODE_ID_SLAVE2(3) 반환

        > AppCore_InitDefaultTexts
            > AppCore_SetAdcText
            > AppCore_SetLinInputText
            > AppCore_SetLinLinkText

        > AppSlave2_Init
            > RuntimeIo_GetSlave2LedConfig
                > BoardHw_GetRgbLedConfig
            > LedModule_Init

            > RuntimeIo_GetSlave2AdcConfig
                - init_fn = AdcHw_Init
                - sample_fn = AdcHw_Sample
                - threshold 설정
                    - range_max = 4096
                    - safe_max = 1365
                    - warning_max = 2730
                    - emergency_min = 3413
            > AdcModule_Init
                > AdcHw_Init
                    > IsoSdk_AdcInit
                        > ADC_DRV_ConfigConverter
                        > ADC_DRV_AutoCalibration

            > RuntimeIo_GetSlaveLinConfig
                - role = LIN_ROLE_SLAVE
                - pid_status = 0x24
                - pid_ok = 0x25
                - ok_token = 0xA5
                > LinHw_Configure
            > LinModule_Init
                > LinHw_Init
                    > IsoSdk_LinInit
                        > LIN_DRV_Init
                        > LIN_DRV_InstallCallback
            > RuntimeIo_AttachLinModule
                > LinHw_AttachModule

    > RuntimeTick_ClearHooks
    > RuntimeTick_RegisterHook
        - AppCore_OnTickIsr 등록

    > Runtime_BuildTaskTable
        - lin_fast -> Runtime_TaskLinFast
        - adc -> Runtime_TaskAdc
        - led -> Runtime_TaskLed
        - heartbeat -> Runtime_TaskHeartbeat

    > RuntimeTick_GetMs
    > RuntimeTask_ResetTable
        - 모든 RuntimeTaskEntry.last_run_ms = start_ms

> Runtime_Run
    - init 실패면 Runtime_FaultLoop
    - for (;;)
        > RuntimeTask_RunDue
            > lin_fast due
                > Runtime_TaskLinFast
                    > AppCore_TaskLinFast
                        > AppSlave2_HandleLinOkToken
                            > LinModule_ConsumeSlaveOkToken
                            > AdcModule_ClearEmergencyLatch
                                - 최신 zone != EMERGENCY 일 때만 latch clear
                            > AppCore_SetLinInputText

            > adc due
                > Runtime_TaskAdc
                    > AppCore_TaskAdc
                        > AppSlave2_TaskAdc
                            > AdcModule_Task
                                > AdcHw_Sample
                                    > IsoSdk_AdcSample
                                        > ADC_DRV_ConfigChan
                                        > ADC_DRV_WaitConvDone
                                        > ADC_DRV_GetChanResult
                                > AdcModule_ClassifyZone
                                    - SAFE / WARNING / DANGER / EMERGENCY 분류
                                - zone == EMERGENCY 이면
                                    - snapshot.emergency_latched = 1
                            > AdcModule_GetSnapshot
                            - adc_text 갱신
                            > LinModule_SetSlaveStatus
                                - slave_status_cache 갱신
                            - LED 반영
                                - latch != 0 -> RED_BLINK
                                - SAFE -> GREEN_SOLID
                                - WARNING -> YELLOW_SOLID
                                - 그 외 -> RED_SOLID

            > led due
                > Runtime_TaskLed
                    > AppCore_TaskLed
                        > LedModule_Task

            > heartbeat due
                > Runtime_TaskHeartbeat
                    > AppCore_TaskHeartbeat
                        - heartbeat_count++
```

주요 변수:
- `g_runtime`
  - runtime 전체 상태
  - `initialized`, `init_status`, `app`, `tasks[4]`
- `g_runtime_tick_ms`
  - scheduler 기준 ms 시간
- `g_runtime_tick_base_count`
  - 500us interrupt 누적 횟수
- `g_runtime_tick_us_accumulator`
  - 500us -> 1ms 환산 누적기
- `g_runtime_tick_hooks[]`
  - `AppCore_OnTickIsr` 등록
- `g_runtime_io_lin_module`
  - runtime_io가 붙잡고 있는 LIN module 포인터
- `AppCore`
  - `lin_enabled`, `adc_enabled`, `led2_enabled`
  - `heartbeat_count`
  - `lin_module`, `adc_module`, `slave2_led`
  - `adc_text`, `lin_input_text`, `lin_link_text`
- `AdcModule`
  - `config`
  - `snapshot.raw_value`
  - `snapshot.zone`
  - `snapshot.emergency_latched`
  - `last_sample_ms`
- `LinModule`
  - `state`
  - `flags`
  - `current_pid`
  - `ok_token_pending`
  - `slave_status_cache`
  - `latest_status`
- `s_adc_hw_context`
  - ADC SDK context
- `s_lin_hw`
  - LIN driver가 쓰는 현재 module, sdk_context, role, timeout_ticks

## 3. S32K_LinCan_master

역할:
- node id = `1`
- master coordinator
- UART console 입력, CAN 제어, LIN polling, 승인 정책 담당

task 주기:
- `uart = 1ms`
- `lin_fast = 1ms`
- `can = 10ms`
- `lin_poll = 20ms`
- `render = 100ms`
- `heartbeat = 1000ms`

```text
ISR
> RuntimeTick_IrqHandler
    - 500us base tick 누적
    - g_runtime_tick_base_count++
    - g_runtime_tick_us_accumulator += 500
    - 1000us 이상이면 g_runtime_tick_ms++
    - 등록 hook 호출
        > AppCore_OnTickIsr
            > LinModule_OnBaseTick
                > LinHw_ServiceTick
                    > IsoSdk_LinServiceTick
                        > LIN_DRV_TimeoutService
    > TickHw_ClearCompareFlag
        > IsoSdk_TickClearCompareFlag
            > LPTMR_DRV_ClearCompareFlag

UART callback
> IsoSdk_UartRxCallback
    > UartHw_OnIsoSdkEvent
        - RX_FULL 이면 rx_byte를 rx_pending ring에 push
        - 이어서 다음 1byte receive 재시작

CAN callback (FlexCAN interrupt)
> IsoSdk_CanSdkEventCallback
    > IsoSdk_CanDispatchEvent
        > CanHw_OnIsoSdkEvent
            - event == RX_DONE && mb_index == rx_mb_index 이면
                > CanHw_OnRxComplete
                    > IsoSdk_CanReadRxFrame
                    > CanHw_RxQueuePush
                    > CanHw_StartReceive
                        > IsoSdk_CanStartReceive
                            > FLEXCAN_DRV_Receive

> IsoSdk_CanSdkErrorCallback
    > IsoSdk_CanDispatchEvent
        > CanHw_OnIsoSdkEvent
            - last_error = CAN_HW_ERROR_RX_STATUS_FAIL

LIN callback
> IsoSdk_LinSdkCallback
    > IsoSdk_LinDispatchEvent
        > LinHw_OnIsoSdkEvent
            > LinModule_OnEvent
                - master 모드에서는 flags만 세팅
                    - LIN_FLAG_PID_OK
                    - LIN_FLAG_RX_DONE
                    - LIN_FLAG_TX_DONE
                    - LIN_FLAG_ERROR

main
> Runtime_Init
    > RuntimeIo_BoardInit
        > BoardHw_Init
            > IsoSdk_BoardInit
                > CLOCK_SYS_Init
                > CLOCK_SYS_UpdateConfiguration
                > PINS_DRV_Init
        > BoardHw_EnableLinTransceiver
            > IsoSdk_BoardEnableLinTransceiver
                > PINS_DRV_SetPinsDirection
                > PINS_DRV_SetPins

    > RuntimeTick_Init
        - init val
            - g_runtime_tick_ms
            - g_runtime_tick_base_count
            - g_runtime_tick_us_accumulator
            - g_runtime_tick_hooks[]
        > TickHw_Init
            > IsoSdk_TickInit
                > LPTMR_DRV_Init
                > INT_SYS_InstallHandler
                    - RuntimeTick_IrqHandler
                > INT_SYS_EnableIRQ
                > LPTMR_DRV_StartCounter

    > AppCore_Init
        > RuntimeIo_GetLocalNodeId
            - APP_NODE_ID_MASTER(1) 반환

        > AppCore_InitDefaultTexts
            > AppCore_SetModeText
            > AppCore_SetButtonText
            > AppCore_SetAdcText
            > AppCore_SetCanInputText
            > AppCore_SetLinInputText
            > AppCore_SetLinLinkText

        > AppMaster_Init
            > AppCore_InitConsoleCan
                > AppConsole_Init
                    > InfraQueue_Init
                    > UartService_Init
                        > UartHw_InitDefault
                            > IsoSdk_UartIsSupported
                            > IsoSdk_UartGetDefaultInstance
                            > IsoSdk_UartInit
                                > LPUART_DRV_Init
                                > LPUART_DRV_InstallRxCallback
                            > UartHw_StartReceiveByte
                                > IsoSdk_UartStartReceiveByte
                                    > LPUART_DRV_ReceiveData
                    - 초기 console view 문자열 세팅
                > CanModule_Init
                    > InfraQueue_Init
                    > CanService_Init
                        > CanProto_Init
                        > CanTransport_Init
                            > CanHw_InitDefault
                                > IsoSdk_CanIsSupported
                                > IsoSdk_CanGetDefaultInstance
                                > IsoSdk_CanInitController
                                    > FLEXCAN_DRV_Init
                                > IsoSdk_CanInstallEventCallback
                                    > FLEXCAN_DRV_InstallEventCallback
                                    > FLEXCAN_DRV_InstallErrorCallback
                                > IsoSdk_CanInitTxMailbox
                                    > FLEXCAN_DRV_ConfigTxMb
                                > IsoSdk_CanConfigRxAcceptAll
                                    > FLEXCAN_DRV_SetRxMaskType
                                    > FLEXCAN_DRV_SetRxIndividualMask
                                > IsoSdk_CanInitRxMailbox
                                    > FLEXCAN_DRV_ConfigRxMb
                                > CanHw_StartReceive
                                    > IsoSdk_CanStartReceive
                                        > FLEXCAN_DRV_Receive

            > RuntimeIo_GetMasterLinConfig
                - role = LIN_ROLE_MASTER
                - pid_status = 0x24
                - pid_ok = 0x25
                - ok_token = 0xA5
                > LinHw_Configure
            > LinModule_Init
                > LinHw_Init
                    > IsoSdk_LinInit
                        > LIN_DRV_Init
                        > LIN_DRV_InstallCallback
            > RuntimeIo_AttachLinModule
                > LinHw_AttachModule

    > RuntimeTick_ClearHooks
    > RuntimeTick_RegisterHook
        - AppCore_OnTickIsr 등록

    > Runtime_BuildTaskTable
        - uart -> Runtime_TaskUart
        - lin_fast -> Runtime_TaskLinFast
        - can -> Runtime_TaskCan
        - lin_poll -> Runtime_TaskLinPoll
        - render -> Runtime_TaskRender
        - heartbeat -> Runtime_TaskHeartbeat

    > RuntimeTick_GetMs
    > RuntimeTask_ResetTable
        - 모든 RuntimeTaskEntry.last_run_ms = start_ms

> Runtime_Run
    - init 실패면 Runtime_FaultLoop
    - for (;;)
        > RuntimeTask_RunDue
            > uart due
                > Runtime_TaskUart
                    > AppCore_TaskUart
                        - uart_task_count++
                        > AppConsole_Task
                            > UartService_ProcessRx
                            - line 완성 시
                                > AppConsole_HandleLine
                                    - help / hello / ping / status / ok 는 로컬 처리
                                    - open/close/off/test/text/event 는
                                        > AppConsole_QueueCanCommand
                            > UartService_ProcessTx

            > lin_fast due
                > Runtime_TaskLinFast
                    > AppCore_TaskLinFast
                        > LinModule_TaskFast
                            - flags 기반으로 RX/TX 진행
                            - status 수신이면 latest_status 갱신
                        > LinModule_ConsumeFreshStatus
                        > AppMaster_HandleFreshLinStatus
                            - adc_text, lin_input_text, lin_link_text 갱신
                            - emergency_active 계산
                                - zone == EMERGENCY 또는 latch != 0
                            - emergency 진입 시
                                > AppCore_QueueCanCommandCode
                                    > CanModule_QueueCommand
                                        - target = slave1
                                        - command = CAN_CMD_EMERGENCY
                            - ok pending 이고
                              zone != EMERGENCY 이고
                              latch == 0 이면
                                > AppCore_QueueCanCommandCode
                                    > CanModule_QueueCommand
                                        - target = slave1
                                        - command = CAN_CMD_OK

            > can due
                > Runtime_TaskCan
                    > AppCore_TaskCan
                        - console queue 비우기
                            > AppConsole_TryPopCanCommand
                            - command.type에 따라
                                > CanModule_QueueCommand
                                > CanModule_QueueText
                                > CanModule_QueueEvent
                        > CanModule_Task
                            > CanService_Task
                                > CanTransport_Task
                                    > CanHw_Task
                                        - TX 완료/에러 상태 확인
                                        - RX 완료 frame 적재는 interrupt callback이 이미 수행
                                        - RX transfer error 복구만 확인
                                    > CanTransport_DrainHwRx
                                        - interrupt callback이 넣어둔 hw->rx_queue를 transport queue로 이동
                                    > CanTransport_ProcessTx
                                > CanService_ProcessRx
                                > CanService_ProcessTimeouts
                            > CanModule_SubmitPending
                        > CanModule_TryPopResult
                            > AppCore_FormatCanResult
                            > AppCore_SetResultText
                        > CanModule_TryPopIncoming
                            > AppCore_HandleCanIncoming
                                - EVENT / TEXT 는 화면 메시지로 반영
                                - COMMAND 이면
                                    > AppMaster_HandleCanCommand
                                        - source == slave1 && cmd == CAN_CMD_OK 이면
                                            > AppMaster_RequestOk
                                                > LinModule_GetLatestStatus
                                                - latest status 없으면 waiting
                                                - zone == EMERGENCY 이면 denied
                                                - latch == 0 이면 already clear
                                                - 승인 가능하면
                                                    - master_slave1_ok_pending = 1
                                                    > LinModule_RequestOk

            > lin_poll due
                > Runtime_TaskLinPoll
                    - console local ok 확인
                        > AppConsole_ConsumeLocalOk
                        - 1 이면
                            > AppMaster_RequestOk
                    > LinModule_TaskPoll
                        - ok_tx_pending 있으면
                            > LinModule_MasterStart(pid_ok)
                                > LinHw_MasterSendHeader
                                    > IsoSdk_LinMasterSendHeader
                                        > LIN_DRV_MasterSendHeader
                        - 아니면
                            > LinModule_MasterStart(pid_status)
                                > LinHw_MasterSendHeader
                                    > IsoSdk_LinMasterSendHeader
                                        > LIN_DRV_MasterSendHeader
                    > AppMaster_AfterLinPoll
                        - ok_pending 이고 latch 살아 있으면
                            > LinModule_RequestOk 재시도

            > render due
                > Runtime_TaskRender
                    > AppCore_TaskRender
                        - task_text / source_text / value_text 조립
                        > AppConsole_SetTaskText
                        > AppConsole_SetSourceText
                        > AppConsole_SetValueText
                        > AppConsole_Render
                            > AppConsole_UpdateInputView
                                > UartService_GetCurrentInputText
                            > AppConsole_RenderLayout
                                > UartService_RequestTx
                            > AppConsole_RenderDirtyLines
                                > UartService_RequestTx

            > heartbeat due
                > Runtime_TaskHeartbeat
                    > AppCore_TaskHeartbeat
                        - heartbeat_count++
```

주요 변수:
- `g_runtime`
  - runtime 전체 상태
  - `initialized`, `init_status`, `app`, `tasks[6]`
- `g_runtime_tick_ms`
  - scheduler 기준 ms 시간
- `g_runtime_tick_base_count`
  - 500us interrupt 누적 횟수
- `g_runtime_tick_us_accumulator`
  - 500us -> 1ms 환산 누적기
- `g_runtime_tick_hooks[]`
  - `AppCore_OnTickIsr` 등록
- `g_runtime_io_lin_module`
  - runtime_io가 가진 master LIN module 포인터
- `AppCore`
  - `console_enabled`, `can_enabled`, `lin_enabled`
  - `master_emergency_active`
  - `master_slave1_ok_pending`
  - `can_last_activity`
  - `lin_last_reported_zone`, `lin_last_reported_lock`
  - `heartbeat_count`, `uart_task_count`, `can_task_count`
  - `console`, `can_module`, `lin_module`
  - `mode_text`, `button_text`, `adc_text`, `can_input_text`, `lin_input_text`, `lin_link_text`
- `AppConsole`
  - `state`
  - `local_ok_pending`
  - `uart`
  - `can_cmd_queue`, `can_cmd_storage[]`
  - `view`
- `AppConsoleView`
  - `input_text`, `task_text`, `source_text`, `result_text`, `value_text`
  - `full_refresh_required`
  - `input_dirty`, `task_dirty`, `source_dirty`, `result_dirty`, `value_dirty`
  - `layout_drawn`
- `UartService`
  - `rx_pending`
  - `rx_line`
  - `tx.queue`
  - `tx.busy`
  - `error_flag`, `error_code`, `error_count`
- `CanModule`
  - `request_queue`, `service`, `last_activity`
- `s_can_hw_instance`
  - CAN IRQ callback이 fallback으로 참조하는 현재 `CanHw` 포인터
- `s_iso_sdk_can_event_cb`, `s_iso_sdk_can_event_context`
  - FlexCAN driver callback을 IsoSdk 쪽에 연결해 두는 전역 상태
- `LinModule`
  - `state`, `flags`, `current_pid`
  - `last_poll_ms`
  - `ok_tx_pending`
  - `latest_status`
- `s_lin_hw`
  - master LIN hardware binding 상태

## 빠른 비교

- `S32K_Can_slave`
  - 입력: 버튼, CAN
  - 출력: LED, CAN 응답
  - 핵심 상태: `slave1_mode`, debounce 변수, `can_module`

- `S32K_Lin_slave`
  - 입력: ADC, LIN ok token
  - 출력: LIN status, LED
  - 핵심 상태: `AdcSnapshot`, `slave_status_cache`, `ok_token_pending`

- `S32K_LinCan_master`
  - 입력: UART console, CAN from slave1, LIN status from slave2
  - 출력: CAN 명령, LIN poll/ok token, UART render
  - 핵심 상태: `master_emergency_active`, `master_slave1_ok_pending`, `AppConsole`, `LinModule.latest_status`
