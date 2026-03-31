# 04. 단계 3 프롬프트 — SDK 의존성 절연 / Binding / Adapter 설계

이 단계가 이번 버전의 핵심이다.  
목표는 vendor SDK와 generated 코드를 시스템 전반으로 퍼뜨리지 않고, 한정된 경계 안에 가두는 것이다.

```text
마스터 시스템 프롬프트와 단계 1, 2 결과를 전제로 한다.

지금은 아래만 수행하라.
- SDK 의존성 절연 전략 설계
- binding / adapter / wrapper 책임 분리
- vendor symbol 누수 차단 규칙 정의
- generated 변수명, instance 번호, vendor status를 외부에서 숨기는 방법 설계

반드시 아래 형식으로 답하라.

1) SDK 누수 위험 목록
   아래 각각에 대해 누수 가능성을 점검하라.
   - vendor include
   - vendor type
   - vendor enum/status code
   - vendor callback 시그니처
   - vendor 함수명
   - generated config/state 변수명
   - instance/channel/MB 번호
   - register bit 의미

2) 절연 전략
   아래 층으로 나눠 설명하라.
   - Binding: 보드/SDK/generated symbol 수용층
   - HAL Adapter: vendor API를 프로젝트 API로 변환하는 층
   - Driver: 기능 단위 제어와 상태 관리
   - Service/App: vendor 독립 정책층

3) 반드시 지켜야 할 규칙
   - public header에서 vendor include 금지
   - public struct에서 vendor type 금지
   - public enum에서 vendor status 값 재사용 금지
   - generated 변수명은 binding.c 내부 static 또는 binding table 내부로 한정
   - instance 번호는 프로젝트 의미 이름으로 치환
   - vendor callback은 bridge 함수에서 프로젝트 이벤트로 변환
   - vendor status/error는 프로젝트 공통 status/error로 매핑

4) 함수명/변수명 절연 규칙
   예시와 함께 정리하라.

   4-1. 외부 공개 이름 예시
   - UartPort
   - UartPortConfig
   - UartPort_Open
   - UartPort_WriteAsync
   - CanBus_Start
   - AdcSampler_Request

   4-2. 내부 모듈 이름 예시
   - UartDriverContext
   - UartAdapterBinding
   - CanAdapterInstance
   - AdcBindingEntry

   4-3. binding 내부에만 남겨야 할 vendor/generated 이름 예시
   - lpuart_1_InitConfig0
   - lpUartState1
   - INST_LPUART_1
   - flexcanState0
   - canCom0_InitConfig0

5) generated 변수명 숨기기 전략
   아래 중 어떤 방식을 쓸지 선택하고 이유를 설명하라.
   - binding.c 내부 static 포인터 테이블
   - binding descriptor 배열
   - opaque binding handle
   - board-specific registration 함수

6) 보드/SDK 교체 내성 전략
   - pin 변경
   - instance 변경
   - SDK 함수 시그니처 변경
   - generated 변수명 변경
   - status code 변경
   - callback signature 변경
   에 대해 어떤 파일만 수정되면 되는지 명확히 적어라.

7) public/private header 설계 규칙
   - 무엇을 .h에 두고 무엇을 .c에 숨길지
   - opaque pointer가 필요한지
   - private header를 둘지 말지
   - inline 함수 허용 범위

8) 상태/에러 매핑 규칙
   - vendor status → project status
   - vendor irq reason → project event
   - vendor timeout/busy → project error contract
   - 여러 SDK가 공존할 때 공통 분모를 어떻게 잡을지

9) 산출물 형식
   아래를 반드시 포함하라.
   - recommended file tree
   - binding 책임표
   - adapter 책임표
   - 외부 공개 심볼 목록
   - 내부 전용 심볼 목록
   - 절대 노출 금지 심볼 목록

10) 마지막 self-check
   아래 질문에 전부 답하라.
   - public header에서 vendor include가 완전히 사라졌는가?
   - generated 변수명이 binding 밖으로 새는가?
   - 상위 레이어가 instance 번호를 아는가?
   - 테스트 코드가 vendor SDK 없이 돌아갈 수 있는가?
   - 보드 교체 시 몇 파일을 수정해야 하는가?

중요:
- 이번 단계에서는 실제 vendor 코드에 직접 묶이지 않는 일반 원칙 중심으로 답하라.
- 특정 SDK 이름은 예시로만 쓰고, 구조는 범용적으로 설계하라.
```
