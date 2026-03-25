# UART 단계 진행 요약

## 이번 단계에서 추가된 것

- `hal_s32k_uart.*`
  - master/button binding 개념 추가
  - `Init / StartRx / PollRxByte / IsTxBusy / Tx` wrapper 분리
- `drv_uart.*`
  - RX ring buffer
  - TX chunk queue
  - 비동기 TX 진행 상태(`txInFlight`) 관리
  - RX overflow / TX drop / error counter 추가
- `svc_uart_console.*`
  - 줄 단위 명령 파서
  - `help`, `status`, `counters`, `ack`, `poll` 지원
  - prompt/banner 처리
- `app_master_node.*`
  - console snapshot 생성
  - console 명령을 gateway 정책으로 연결
  - UART는 stream, console은 service, policy는 gateway/app로 분리

## 현재 의도한 호출 흐름

1. `DrvUart_Process()`
   - HAL에서 받은 RX byte를 ring에 적재
   - queued TX chunk를 비동기 전송
2. `SvcUartConsole_Process()`
   - ring에서 byte를 꺼내 line assemble
   - 명령 파싱
   - text 응답 queue 적재
3. `AppMasterNode_Process()`
   - `ack` 명령이면 `SvcGateway_RequestLinAck()`
   - `poll` 명령이면 `SvcGateway_RequestImmediatePoll()`

## 아직 남은 것

- 실제 generated UART 심볼 연결
- RX/TX 완료 방식이 프로젝트 SDK 설정과 정확히 맞는지 확인
- command set 확장(`can status`, `lin status`, `diag` 등)
- TX queue가 포화될 때 로그/드롭 정책 세분화
