# 07. 단계 6 프롬프트 — 최종 리뷰 / 리팩터링 / 시니어 품질 게이트

이 단계는 결과물이 정말로 좋은 구조인지 냉정하게 검증하는 단계다.

```text
마스터 시스템 프롬프트와 이전 단계 결과 또는 현재 코드를 전제로 한다.

지금은 아래만 수행하라.
- 현재 설계/코드의 품질을 냉정하게 평가
- 시니어 코드 품질 기준에서 부족한 점 식별
- 리팩터링 우선순위 도출
- 절대 놓치면 안 되는 리스크 정리

반드시 아래 형식으로 답하라.

1) 전체 등급
   - architecture
   - sdk isolation
   - module contracts
   - naming
   - testability
   - diagnostics
   - time/state model
   - maintainability

2) 좋은 점
3) 문제점
   아래 축별로 평가하라.
   - 레이어 위반
   - vendor/generated symbol 누수
   - hidden global state
   - ownership 불명확
   - timeout/retry/cancel 불명확
   - queue overflow/drop 정책 부족
   - 테스트 불가능한 결합
   - 네이밍 불명확
   - 과설계 또는 과소설계

4) 가장 먼저 고쳐야 할 것 Top N
   각 항목마다
   - 문제
   - 왜 위험한지
   - 수정 방향
   - 수정 범위
   - 예상 부작용
   를 적어라.

5) 리팩터링 순서
   - no-risk cleanup
   - boundary cleanup
   - sdk isolation cleanup
   - contract cleanup
   - test extraction
   - state/timeout cleanup

6) 시니어 품질 게이트 질문
   모두 yes/no로 답하고, no면 이유를 써라.
   - public header에서 vendor type이 사라졌는가?
   - generated symbol이 binding 밖으로 나오지 않는가?
   - App이 hardware 세부를 모르고도 동작하는가?
   - host-side test 가능한 로직이 충분히 분리되었는가?
   - timeout/retry/cancel contract가 문서화되었는가?
   - queue/buffer/pool의 capacity와 overflow 정책이 명시되었는가?
   - GetState/GetStats/GetLastError가 필요한 곳에 존재하는가?
   - 보드 변경 시 수정 위치가 국소화되어 있는가?
   - SDK 교체 시 binding/adapter 경계 안에서 끝날 가능성이 높은가?
   - 새 peripheral 추가 시 레이어를 무너뜨리지 않고 확장 가능한가?

7) 최종 판정
   아래 중 하나로 분류하라.
   - 바로 사용 가능
   - 제한적 사용 가능
   - 구조 보강 후 사용
   - 대규모 재설계 필요

중요:
- 듣기 좋은 말보다 정확한 평가를 우선하라.
- 코드 품질을 점수처럼 애매하게 말하지 말고, 왜 그런지 근거 중심으로 설명하라.
```
