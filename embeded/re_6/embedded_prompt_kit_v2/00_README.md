# 범용 임베디드 AI 프롬프트 키트 v2

이 키트는 **특정 기존 프로젝트를 복제하기 위한 문서가 아니다.**  
목표는 어떤 임베디드 프로젝트에도 적용 가능한 수준으로, AI가 **고품질 구조 설계 / 코드 생성 / 코드 리뷰 / 리팩터링 / SDK 의존성 절연**을 일관되게 수행하도록 만드는 것이다.

이번 버전은 특히 아래를 강화했다.

- SDK 의존성 최소화
- vendor 함수명/타입명뿐 아니라 **generated 변수명과 전역 상태 이름 누수 차단**
- 단계별 진행 방식
- 한 번에 전부 생성하지 않고, **설계 → 절연 → 계약 → 구현 → 테스트 → 리뷰** 순서로 가는 실행용 프롬프트 묶음
- AI가 읽고 다시 이어서 작업하기 쉬운 출력 형식 강제

---

## 이 키트의 추천 사용 순서

가장 좋은 사용법은 아래 순서다.

1. `01_master_system_prompt.md`
   - 전체 공통 규칙
   - 어떤 단계에서든 공통으로 붙이는 마스터 프롬프트

2. `02_stage_1_requirements_and_constraints.md`
   - 문제 재정의
   - 제약/시간/상태/동시성 정리

3. `03_stage_2_architecture_and_boundaries.md`
   - 레이어 구조
   - 책임/소유권/경계 정리

4. `04_stage_3_sdk_isolation_and_adapter_design.md`
   - SDK 의존성 절연
   - vendor 함수명/변수명/타입명/전역 상태 격리
   - binding / adapter / wrapper 구조 설계

5. `05_stage_4_module_contracts_and_naming.md`
   - 모듈 공통 계약
   - 네이밍 규칙
   - public/private 경계 확정

6. `06_stage_5_implementation_and_test_strategy.md`
   - 구현 순서
   - host-side test / target test 전략
   - 계측/진단/복구 설계

7. `07_stage_6_review_and_refactor_gate.md`
   - 최종 품질 게이트
   - 시니어 수준에 맞는 self-review / refactor 체크

---

## 가장 좋은 운용 방식

### 방식 A. 신규 프로젝트 설계
- 01 + 02 + 03 + 04 + 05 순서로 먼저 구조를 확정한다.
- 그 다음 06으로 구현 계획과 테스트 전략을 만든다.
- 마지막에 07로 품질 게이트를 통과시킨다.

### 방식 B. 기존 프로젝트 리팩터링
- 01 + 07로 현재 문제를 먼저 진단한다.
- 그 다음 04로 SDK 누수부터 끊는다.
- 그 다음 05로 모듈 계약과 네이밍을 정리한다.
- 마지막으로 06으로 구현/검증 순서를 짠다.

### 방식 C. 특정 주변장치 모듈 추가
- 01 + 02로 요구사항/시간/에러모델을 정리한다.
- 04로 SDK adapter와 binding을 설계한다.
- 05로 public API와 상태 모델을 정의한다.
- 06으로 구현/테스트 순서를 짠다.

---

## 이 키트가 특히 강제하는 것

- 현재 프로젝트를 기준으로 복제하지 말 것
- 참고 문서는 참고만 할 것
- SDK 타입/전역/함수/에러코드/변수명 누수를 public 경계에서 차단할 것
- generated symbol을 시스템 전반에 퍼뜨리지 말 것
- 레이어별 ownership / timing / error contract를 명시할 것
- 상태/시간/ISR/deferred work를 코드보다 먼저 정의할 것
- 인터럽트 초기화 순서: `초기값 세팅 -> ISR 컨텍스트 준비 -> handler 설치 -> interrupt enable -> peripheral start`를 기본 규칙으로 둘 것
- host-side test 가능한 로직을 분리할 것
- 리뷰 결과를 반드시 “좋은 점 / 문제점 / 수정 우선순위 / 남은 리스크” 형식으로 남길 것

---

## 결과물 원칙

AI는 항상 아래 중 일부 또는 전부를 산출해야 한다.

- 문제 재정의
- 아키텍처 다이어그램 또는 계층 설명
- 모듈별 책임표
- 상태 전이 요약
- 시간/timeout/retry/recovery 정책
- SDK 절연 전략
- public/private header 경계
- 파일 구조 제안
- 구현 순서
- 테스트 전략
- self-review

