# CAN 진행 상태

이번 단계에서 채운 것:

1. `hal/hal_s32k_can.*`
   - master / button 노드 binding 분리
   - `FLEXCAN_DRV_Init`, `ConfigRxMb`, `ConfigTxMb`, `Receive`, `Send`, `GetTransferStatus` 연결 자리 추가
   - RX mailbox 재-arm 구조 반영

2. `drivers/drv_can.*`
   - software TX/RX queue 추가
   - non-blocking process loop
   - `txInFlight`, `txStartCount`, `txOkCount`, `rxOkCount`, `dropCount`, `errorCount` 추가
   - `DrvCan_TryReceive()` 추가

3. `services/svc_can_button_proto.*`
   - button semantic event <-> CAN frame packing/unpacking 추가

4. `app/app_can_button_slave.*`
   - 버튼 이벤트 보고 API 추가
   - pending event를 process 주기에서 frame 으로 송신

5. `app/app_master_node.*`
   - CAN frame 수신 후 button semantic event로 해석
   - 마지막 버튼 event 상태 저장

남은 것:

- 실제 generated 이름을 `platform/s32k_bindings.h`에 반영
- 버튼 GPIO debounce service 추가
- master gateway service에서 CAN 버튼 event -> LIN command 연결
- UART console에서 CAN 상태/통계 조회 연결
