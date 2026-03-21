# 08 final stabilization

이번 단계에서 마지막으로 고정한 핵심은 아래 4개다.

1. **LIN callback ↔ main shared pending 계약**
   - `DrvLinMaster`, `DrvLinSlave` 의 pending flag 를 `volatile` 로 바꾸고,
   - `Take*()` / `TryGetStatusFrame()` 의 read-clear 구간을 `EMB_ENTER_CRITICAL` / `EMB_EXIT_CRITICAL` 로 감쌌다.
   - 기본 매크로는 no-op 이며, 실제 프로젝트에서는 플랫폼에 맞는 IRQ guard 로 override 한다.

2. **마스터 App 처리 순서 정리**
   - 기존에는 `SvcGateway_Process()` 가 CAN/UART 이벤트 반영보다 먼저 돌았다.
   - 지금은 `driver process -> LIN deferred -> CAN drain -> UART console -> gateway process -> LIN issue` 순서로 바꿨다.
   - 이 순서가 가장 최근 입력을 기준으로 ACK/POLL 결정을 내린다.

3. **board init / tick init 명시화**
   - `S32kBoard_InitMasterNode()` / `S32kBoard_InitCanButtonSlave()` 를 추가했다.
   - main 에 남아 있던 `TODO` 를 실제 초기화 함수 호출로 바꿨다.
   - master/button 프로젝트마다 clock/pin/lptmr generated bind 를 선택적으로 붙일 수 있게 `s32k_bindings.h` 예시를 늘렸다.

4. **tick 모듈 일반화**
   - 기존 `S32kTick` 은 sensor slave LIN timeout service 에 사실상 고정돼 있었다.
   - 지금은 `S32K_TICK_TIMEOUT_SERVICE_INSTANCE`, `S32K_TICK_LPTMR_INSTANCE` 매크로를 통해 노드별로 바꿔 끼울 수 있게 정리했다.

## 프로젝트에 바로 반영할 때 마지막으로 확인할 것

- `EMB_ENTER_CRITICAL` / `EMB_EXIT_CRITICAL` 를 실제 플랫폼 IRQ guard 로 바꿀지 여부
- master/button 각 프로젝트의 `CLOCK/PIN/LPTMR` generated symbol 연결
- master 와 button 노드에서 정말 tick 이 필요한지, 필요하면 어느 timer 를 쓸지
- UART/CAN 큐 depth 를 실제 부하에 맞춰 늘릴지 여부
