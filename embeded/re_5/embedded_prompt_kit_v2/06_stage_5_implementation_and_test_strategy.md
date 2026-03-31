# 06. 단계 5 프롬프트 — 구현 순서 / 테스트 전략 / 계측 / 진단

이 단계는 구현을 무작정 시작하지 않고, 가장 안전한 순서로 밟게 만드는 단계다.

```text
마스터 시스템 프롬프트와 이전 단계 결과를 전제로 한다.

지금은 아래만 수행하라.
- 구현 순서 제안
- host-side test 가능한 부분과 target-only 부분 분리
- 계측/진단/로그/통계 전략 정의
- bring-up 순서 정의

반드시 아래 형식으로 답하라.

1) 구현 우선순위
   아래 순서를 기본으로 검토하라.
   1. infra / time / queue / status
   2. binding / adapter skeleton
   3. driver minimal bring-up
   4. service state machine
   5. app policy
   6. diagnostics / stats / recovery
   7. integration test

2) 각 단계의 완료 조건
   - compile 조건
   - unit test 조건
   - hardware test 조건
   - regression 조건

3) host-side test 가능한 부분
   - parser
   - queue/buffer
   - timeout math
   - state machine
   - retry/recovery policy
   - pending table
   - validation logic

4) target-only test 필요한 부분
   - irq latency
   - dma complete path
   - peripheral driver interaction
   - pin/clock/init sequence
   - bus error / framing / arbitration / overrun

5) 진단 항목
   모듈별로 아래를 강제하라.
   - current state
   - last error
   - error counter
   - timeout counter
   - drop/overflow counter
   - retry counter
   - busy/idle counter 필요 여부

6) 로그 원칙
   - ISR에서 금지
   - binary/event log 가능 여부
   - text log 최소화 전략
   - field diagnostics 최소 포맷

7) bring-up 순서
   - clock/pin
   - binding validation
   - adapter smoke test
   - 초기값 세팅
   - ISR이 참조할 컨텍스트/버퍼/큐 준비
   - callback/handler bridge 설치
   - stale pending/error/status flag clear
   - interrupt enable
   - 마지막에 peripheral open/start 또는 first RX/TX arm
   - loopback or dummy-path test
   - service timeout path
   - app integration

7-1) 인터럽트 초기화 안전성 검증
   아래 질문에 답하라.
   - 인터럽트 enable 전에 ISR이 읽는 모든 메모리가 준비되었는가?
   - peripheral start 직후 첫 ISR이 들어와도 null/stale/garbage 참조가 없는가?
   - disable/stop/deinit은 enable/start의 역순으로 정리되는가?
   - init 실패 중간 탈출 시 half-enabled 상태가 남지 않는가?

8) 위험 요소
   - integration 전에 unit/host test 없이 바로 보드 검증으로 가는 것
   - SDK generated 코드를 서비스/앱에 퍼뜨리는 것
   - 정상 경로만 확인하고 timeout/error를 뒤로 미루는 것

9) 최종 인수 조건
   - 구조 품질
   - 테스트 통과 범위
   - 진단 가능성
   - 보드/SDK 교체 내성

중요:
- 지금 단계에서는 구현 계획과 검증 계획만 정리하라.
- 대량 코드 출력보다 단계별 완료 조건이 더 중요하다.
```
