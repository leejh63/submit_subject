# 05. 단계 4 프롬프트 — 모듈 계약 / 네이밍 / 인터페이스 확정

이 단계는 구조를 실제 모듈 단위 계약으로 굳히는 단계다.

```text
마스터 시스템 프롬프트와 이전 단계 결과를 전제로 한다.

지금은 아래만 수행하라.
- 주요 모듈 목록 정의
- 모듈별 Config / Runtime / API / Ownership / Timing / Error contract 확정
- 네이밍 규칙 확정
- public/private 인터페이스 확정

반드시 아래 형식으로 답하라.

1) 모듈 목록
   - module name
   - 책임
   - 소속 레이어

2) 각 모듈의 계약
   아래 템플릿을 모듈별로 채워라.
   - Purpose
   - Public types
   - Private state
   - Init inputs
   - Runtime API
   - Task/Poll API
   - ISR/DMA hook 여부
   - Ownership
   - Blocking 여부
   - Error reporting 방식
   - Stats 필요 항목
   - Reset/Recovery 동작

3) 공통 status/error 규칙
   - 공통 status enum
   - 모듈별 last error code 범위
   - recoverable / non-recoverable 구분

4) 네이밍 규칙
   - 타입명
   - 함수명
   - enum/constant
   - file name
   - private helper 함수명
   - callback 이름

5) 금지 이름
   - tmp
   - common
   - util
   - misc
   - manager
   - helper
   처럼 의미 빈약한 이름을 어디까지 금지할지 적어라.

6) header 설계 원칙
   - public header 최소화
   - forward declaration 가능 여부
   - opaque type 사용 여부
   - inline 허용 범위

7) 각 모듈의 GetState / GetStats / GetLastError 필요성
8) 각 모듈의 테스트 포인트
9) 다음 단계 구현 우선순위

중요:
- 실제 코드보다 계약과 이름이 먼저다.
- 인터페이스를 최소화하되 필요한 진단/테스트 포인트는 남겨라.
```
