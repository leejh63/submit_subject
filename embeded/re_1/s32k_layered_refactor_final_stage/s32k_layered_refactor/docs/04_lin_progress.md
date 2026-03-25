# LIN 진행 상태

이번 단계에서 추가된 핵심

1. `hal_s32k_lin`이 slave/master binding을 구분하도록 변경
2. `drv_lin_master` 추가
3. `svc_lin_sensor_proto`에 status parse / ok command build 추가
4. `svc_gateway` 추가
5. `app_master_node`가 CAN button event -> LIN ACK command / periodic LIN status poll 흐름을 가지도록 확장

주의

- `LIN_DRV_MasterSendHeader()` 사용을 전제로 작성했다.
- master project의 실제 generated 이름은 `platform/s32k_bindings.h`에 연결해야 한다.
- `main_master_node.c`의 `nowMs`는 아직 board tick과 연결되지 않았으므로 실제 프로젝트에서 tick source를 넣어야 한다.
- callback 내부에서는 header 이후 `ReceiveFrameData` / `SendFrameData`만 빠르게 수행하고, 정책 판단은 `AppMasterNode_Process()`와 `SvcGateway`에 남겨 두는 구조다.
