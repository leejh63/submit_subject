# LIN 3차 진행 요약

이번 단계의 목표는 **LIN callback 경량화**였다.

## 바뀐 점

### 1. master callback 경량화
- `AppMasterNode_OnLinEvent()`는 이제 직접 parse/gateway를 하지 않는다.
- callback에서는 `DrvLinMaster_OnCallback()`만 호출한다.
- driver는 transport 레벨 처리만 한다.
  - `LIN_PID_OK`에서 즉시 `ReceiveFrameData` 또는 `SendFrameData`
  - `LIN_RX_COMPLETED`면 raw status frame ready flag만 세팅
  - `LIN_TX_COMPLETED`면 done flag만 세팅
  - timeout/error는 pending flag만 세팅
- 실제 status parse와 `SvcGateway_OnSensorStatus()`는 `AppMasterNode_Process()`에서 처리한다.

### 2. slave callback 경량화
- status payload를 callback에서 새로 build하지 않는다.
- App가 최신 `statusFrame[8]`를 미리 유지한다.
- callback에서는 이 prebuilt frame을 바로 송신만 한다.
- `OK_CMD` 수신 완료 시에도 callback에서 emergency clear를 하지 않고, rx pending flag만 세팅한다.
- 실제 `ClearEmergency()`는 `AppLinSensorSlave_Process()`에서 처리한다.

## 기대 효과
- LIN 응답 타이밍에 필요한 최소 작업만 callback에 남는다.
- policy / parse / gateway / LED 반영이 ISR 점유 시간을 늘리지 않는다.
- callback은 transport, task는 service/app 역할이 더 선명해진다.

## 남은 점
- slave callback도 event flag 접근은 ISR↔main 공유 상태이므로, 실제 프로젝트에서는 원자성/임계구역 정책을 정해야 한다.
- 필요하면 다음 단계에서 `pending bitfield + atomic accessor` 형태로 한 번 더 정리할 수 있다.
