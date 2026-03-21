# LIN 2차 진행 상태

이번 단계에서 추가한 핵심은 아래와 같다.

## 1. 마스터 gateway 정책 보강

- sensor poll 주기와 status stale 기준을 분리했다.
- stale status가 되면 fresh status로 보지 않도록 했다.
- CAN 버튼 ACK 요청이 들어와도 status가 stale이면 즉시 ACK를 보내지 않는다.
- status가 fresh하고 `emergencyLatched=1`, `zone!=EMERGENCY` 조건일 때만 LIN OK command를 보낸다.

## 2. ACK 재시도 구조

- ACK 1회 송신 후 바로 pending을 지우지 않는다.
- 다음 sensor status에서 `emergencyLatched=0`이 확인되면 ACK 완료로 본다.
- 여전히 latch가 유지되면 retry period 기준으로 재전송할 수 있다.
- 최대 retry 횟수를 넘기면 pending을 정리하고 give-up counter를 올린다.

## 3. 마스터 App 보강

- `AppMasterNode_Process()`에서 gateway를 먼저 돌린다.
- LIN idle 상태에서 ACK 우선, 그다음 sensor poll 순서로 처리한다.
- poll 요청이 실제로 나간 시점을 `MarkSensorPollIssued()`로 기록해서 header spam을 막는다.
- LIN callback fault/timeout은 `linFaultCount`에 누적한다.

## 4. 추가로 남은 것

- 실제 프로젝트 tick을 `main_master_node.c`에 연결
- UART console에서 gateway/stat counter 출력
- 필요하면 LIN command 종류 확장
