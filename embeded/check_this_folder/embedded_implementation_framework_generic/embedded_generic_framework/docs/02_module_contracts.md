# 02. Module Contracts

## 모든 모듈이 공통으로 가져야 할 것

### A. Config
변하지 않는 설정 묶음이다.

예:
- instance id
- channel id
- baudrate / bitrate
- sampling time
- period / duty range
- callback hooks
- timeout values
- capacity

### B. Runtime
실행 중 변하는 상태다.

예:
- state
- counters
- last error
- flags
- timestamps
- buffers / queues / pools
- pending requests

### C. Public API
권장 최소 집합:

- `*_Init()`
- `*_Deinit()` 또는 `*_Reset()`
- `*_Start()`
- `*_Stop()`
- `*_Process()`
- `*_GetState()`
- `*_GetStats()`

### D. ISR Entry / Callback Entry
필요한 경우만 둔다.

- `*_OnRxIrq()`
- `*_OnTxIrq()`
- `*_OnDmaComplete()`
- `*_OnTimeoutTick()`

## 주변장치별 추가로 필요한 것

### GPIO
- 방향 설정
- 초기 출력값
- active-high/low 정책
- debounce 필요 여부

### TIMER / TIMEBASE
- 단조 증가 tick 보장
- wrap-around 비교 함수
- ISR 기반이면 jitter 한계 문서화

### UART
- RX 경계 정책(line, packet, raw stream)
- framing error 처리
- RX overrun 카운터
- TX queue 유무
- DMA 사용 시 half/full/idle 처리 정책

### SPI
- chip-select ownership
- full-duplex / half-duplex
- transfer timeout
- DMA 연동 여부
- transaction boundary 보호

### I2C
- bus busy recovery
- nack/timeout/retry 정책
- repeated start 필요 여부
- clock stretching 허용 여부

### CAN
- tx mailbox ownership
- filter configuration ownership
- bus-off recovery 정책
- ID scheme / DLC validation
- request-response correlation

### LIN
- master/slave role 분리
- schedule table ownership
- header/response 책임 분리
- checksum/classic/enhanced 구분
- sleep/wakeup 정책

### ADC
- single / continuous / scan / injected 구분
- trigger source(timer/software/external)
- calibration/settling 규칙
- raw -> engineering value 변환 정책
- filter / averaging / plausibility check

### PWM / FTM
- period range
- duty clamp
- glitch-free update 시점
- safe default duty
- complementary/deadtime 필요 여부

### DMA
- source/sink ownership
- descriptor lifetime
- cache coherence 여부
- half/full complete semantics
- cancel / abort 경로

### FLASH / NVM
- erase/write alignment
- wear leveling 여부
- CRC / version / magic
- power-fail 대응
- double buffer/commit marker
