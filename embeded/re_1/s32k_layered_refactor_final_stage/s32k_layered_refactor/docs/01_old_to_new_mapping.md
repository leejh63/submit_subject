# 기존 main.c -> 새 구조 매핑

## 기존 함수

- `Slave2_AdcInit()` -> `HalS32kAdc_InitDefault()` -> `DrvAdc_Start()`
- `Slave2_AdcReadBlocking()` -> `HalS32kAdc_ReadBlocking()` -> `DrvAdc_Read()`
- `Slave2_LedInit()` -> `DrvLed_Start()`
- `Slave2_ClassifyZone()` -> `SvcZone_Classify()`
- `Slave2_SetLedSafe/Warning/Danger/EmergencyBlink()` -> `SvcLedPattern_*()` 내부
- `Slave2_UpdateLedFromAdc()` -> `AppLinSensorSlave_Process()` + `SvcLedPattern_ApplyZone()`
- `Slave2_RunEmergencyBlinkTask()` -> `SvcLedPattern_Process()`
- `Slave2_PrepareAdcFrame()` -> `SvcLinSensorProto_BuildStatusFrame()`
- `Slave2_GoIdle()` -> `DrvLinSlave_GoIdle()`
- `Slave2_LinCallback()` -> `AppLinSensorSlave_OnLinEvent()`
- `main()` -> `main/main_lin_sensor_slave.c`

## 핵심 변화

1. SDK 함수 직접 호출을 app에서 제거
2. LED pin/polarity를 driver/service 아래로 내림
3. ADC raw -> zone -> LED 의미 처리 분리
4. LIN payload packing을 service로 분리
5. main은 조립과 주기 task 호출만 담당
