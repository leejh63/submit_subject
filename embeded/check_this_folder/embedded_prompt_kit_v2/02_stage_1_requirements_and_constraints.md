# 02. 단계 1 프롬프트 — 요구사항 / 제약 / 시간 / 상태 정리

이 단계의 목적은 아직 코드를 만들지 않고, 시스템 문제를 정확히 다시 정의하는 것이다.

```text
마스터 시스템 프롬프트를 전제로 한다.

지금은 코드를 쓰지 말고, 아래만 수행하라.

목표:
- 요구사항 재정의
- 시스템 경계 정의
- 시간 모델 정의
- 상태 모델 정의
- ISR/task/deferred work 경계 정의
- 실패 경로/복구 포인트 식별

반드시 아래 형식으로만 답하라.

1) 시스템 목표 한 문장 요약
2) 입력/출력/외부 의존
3) 기능 요구사항
4) 비기능 요구사항
   - latency
   - throughput
   - memory 제약
   - startup time
   - 안전성/복구성
5) 시간 모델
   - 주기 task
   - event-driven path
   - timeout 기준 시점
   - retry 가능 여부
6) 상태 모델
   - 주요 상태 enum 후보
   - 상태 전이 트리거
   - 금지해야 할 모순 상태
7) 동시성 모델
   - ISR에서 할 일
   - task에서 할 일
   - DMA 완료 후 deferred work
8) 실패 시나리오 목록
   - init 실패
   - timeout
   - queue full
   - stale data
   - invalid command/input
   - peripheral busy/error
9) 이 단계에서 아직 결정하지 말아야 할 것
10) 다음 단계에서 확정해야 할 항목

중요:
- 파일 구조 제안 금지
- 코드 제안 금지
- 현재 단계에서는 문제 모델링에 집중하라.
```
