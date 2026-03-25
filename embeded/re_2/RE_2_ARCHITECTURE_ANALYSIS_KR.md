# re_2 전체 아키텍처 및 모듈 구조 상세 분석

## 1. 문서 목적

이 문서는 `re_2` 아래의 세 프로젝트를 각각 따로 읽지 않아도, 전체 시스템의 구조와 흐름을 한 번에 이해할 수 있도록 작성한 아키텍처 분석 문서다.

대상 프로젝트는 아래 3개다.

- `S32K_LinCan_master`
- `S32K_Lin_slave`
- `S32K_Can_slave`

이 문서에서는 다음을 중점적으로 설명한다.

- 전체 시스템이 어떤 역할로 나뉘어 동작하는지
- 코드가 어떤 레이어로 분리되어 있는지
- 각 모듈이 어떤 책임을 갖고 어떻게 연결되는지
- 부팅부터 주기 task 실행까지의 전체 흐름
- emergency 발생부터 해제까지의 end-to-end 시나리오
- 핵심 구조체, 전역 상태, 내부 플래그의 의미
- 유지보수 관점에서의 장점과 부족한 점

---

## 2. 가장 먼저 봐야 하는 핵심 요약

이 코드베이스는 **3개의 노드가 협력하는 분산형 임베디드 시스템**이다.

- `master`
  - 시스템의 판단 중심
  - LIN으로 센서 상태를 읽음
  - CAN으로 현장 노드에 명령 전송
  - UART 콘솔로 상태 표시 및 사용자 명령 입력 처리
- `slave2`
  - 센서 상태 제공 노드
  - ADC 값을 읽어 위험도(zone)를 판정
  - 그 결과를 LIN으로 master에 제공
  - emergency latch를 내부적으로 유지
- `slave1`
  - 현장 반응 노드
  - master의 CAN 명령에 따라 LED 상태를 바꿈
  - 사용자의 버튼 입력을 받아 master에 OK 요청 전달

시스템의 실제 의미 흐름은 아래 한 줄로 요약할 수 있다.

`ADC 상태 생성(slave2) -> 판단(master) -> 현장 반응(slave1) -> 사용자 승인(slave1 -> master) -> latch 해제(slave2) -> 최종 승인(slave1)`

---

## 3. 세 프로젝트의 관계

세 프로젝트는 겉보기에는 별도 폴더지만, 실제로는 **거의 완전히 동일한 공통 코드베이스를 3번 복제한 구조**다.

실제 차이는 거의 아래 두 가지뿐이다.

- `app/app_config.h`
  - `APP_ACTIVE_ROLE` 값이 다름
- `PROJECT_ANALYSIS_KR.md`
  - 각 프로젝트 설명 문서만 다름

즉, `.c/.h` 소스 구조는 사실상 공통이며, 역할만 다르게 활성화하는 방식이다.

### 프로젝트별 역할

| 프로젝트 | 활성 역할 | 노드 ID | 핵심 통신 | 실질 핵심 app 파일 |
| --- | --- | --- | --- | --- |
| `S32K_LinCan_master` | `APP_ROLE_MASTER` | `1` | LIN + CAN + UART | `app_master.c` |
| `S32K_Lin_slave` | `APP_ROLE_SLAVE2` | `3` | ADC + LIN + LED | `app_slave2.c` |
| `S32K_Can_slave` | `APP_ROLE_SLAVE1` | `2` | CAN + 버튼 + LED + UART | `app_slave1.c` |

### 이 구조가 의미하는 점

- 장점
  - 역할별 빌드를 쉽게 분리할 수 있다.
  - 각 보드 이미지가 독립적인 프로젝트처럼 보인다.
- 단점
  - 코드 중복이 심하다.
  - 공통 로직 수정 시 3개 프로젝트를 동시에 관리해야 한다.
  - 구조를 처음 보는 사람 입장에서는 "왜 slave 프로젝트에 master 코드가 다 있지?"라는 혼란이 생긴다.

---

## 4. 디렉토리 구조와 레이어 맵

각 프로젝트는 아래와 같은 공통 디렉토리 구조를 가진다.

```text
main.c
app/
runtime/
infra/
can/
lin/
uart/
adc/
led/
runtime_io.c/h
```

이 구조를 아키텍처 레이어로 다시 정리하면 다음과 같다.

```text
[Entry Point]
main.c

[Runtime Orchestration Layer]
runtime/

[Application Policy Layer]
app/

[Feature / Service Modules]
can/ lin/ uart/ adc/ led/

[Platform Binding Layer]
runtime/runtime_io.*

[Infrastructure Utilities]
infra/

[Generated SDK / Board Drivers]
S32 SDK, pin/clock/LIN/CAN/UART/ADC driver
```

---

## 5. 전체 아키텍처를 한 그림으로 보면

```text
                    +-----------------------+
                    |  S32K_LinCan_master   |
                    |-----------------------|
                    | AppMaster policy      |
                    | AppCore orchestration |
                    | LIN master            |
                    | CAN command source    |
                    | UART console          |
                    +-----------+-----------+
                                |
                    LIN status  |  LIN ok token
                                |
              +-----------------+-----------------+
              |                                   |
              v                                   ^
     +-------------------+              +-------------------+
     |  S32K_Lin_slave   |              |  S32K_Can_slave   |
     |-------------------|              |-------------------|
     | AppSlave2 policy  |              | AppSlave1 policy  |
     | ADC sampling      |              | CAN command recv  |
     | LIN slave         |              | Button debounce   |
     | LED state         |              | LED response      |
     +-------------------+              +---------+---------+
                                                   |
                                                   | CAN ok request
                                                   v
                                          +-------------------+
                                          |      master       |
                                          +-------------------+
```

---

## 6. 부팅 시점의 전체 흐름

### 6.1 main 진입

`main.c`는 아주 얇다.

1. `Runtime_Init()` 호출
2. 실패 시 fault loop
3. 성공 시 `Runtime_Run()` 호출

즉, 진짜 시스템 조립은 `runtime` 레이어가 담당한다.

### 6.2 Runtime_Init()

`Runtime_Init()`은 시스템 전체를 조립하는 부트스트랩 함수다.

실행 순서는 아래와 같다.

1. `RuntimeIo_BoardInit()`
   - clock 설정
   - pin mux 초기화
   - LIN transceiver enable
2. `RuntimeTick_Init()`
   - LPTMR 기반 base tick 초기화
3. `AppCore_Init()`
   - 현재 역할 확인
   - 역할별 app 초기화 호출
4. `RuntimeTick_RegisterHook(AppCore_OnTickIsr, &g_runtime.app)`
   - tick ISR에 app hook 등록
5. task table 생성
6. task의 기준 시각 초기화

### 6.3 역할별 AppCore_Init() 분기

`AppCore_Init()`은 `APP_ACTIVE_ROLE`에 따라 역할별 초기화를 수행한다.

- master
  - `AppMaster_Init()`
  - console 초기화
  - CAN 모듈 초기화
  - LIN master 초기화
- slave1
  - `AppSlave1_Init()`
  - console 초기화
  - CAN 모듈 초기화
  - LED 초기화
- slave2
  - `AppSlave2_Init()`
  - LED 초기화
  - ADC 초기화
  - LIN slave 초기화

여기서 중요한 점은, **세 프로젝트 모두 같은 `AppCore_Init()`를 사용하지만 역할값에 따라 실제 활성 모듈이 달라진다**는 것이다.

---

## 7. 런타임 실행 구조: super-loop + periodic task

이 프로젝트는 RTOS 기반이 아니라, **super-loop + 주기 task dispatch** 구조를 사용한다.

### 7.1 task 테이블

`runtime.c`는 아래 9개 task를 고정적으로 등록한다.

| 순서 | task 이름 | 주기 | 담당 함수 | 의미 |
| --- | --- | --- | --- | --- |
| 1 | `uart` | 1 ms | `AppCore_TaskUart` | 콘솔 입력 처리 |
| 2 | `lin_fast` | 1 ms | `AppCore_TaskLinFast` | LIN 상태기계 빠른 처리 |
| 3 | `button` | 10 ms | `AppCore_TaskButton` | slave1 버튼 debounce |
| 4 | `can` | 10 ms | `AppCore_TaskCan` | CAN 송수신 처리 |
| 5 | `adc` | 20 ms | `AppCore_TaskAdc` | slave2 ADC 샘플링 |
| 6 | `lin_poll` | 20 ms | `AppCore_TaskLinPoll` | LIN poll / OK 요청 처리 |
| 7 | `led` | 100 ms | `AppCore_TaskLed` | LED pattern 진행 |
| 8 | `render` | 100 ms | `AppCore_TaskRender` | UART 화면 다시 그리기 |
| 9 | `heartbeat` | 1000 ms | `AppCore_TaskHeartbeat` | 생존 카운터 증가 |

### 7.2 구조적 특징

- 모든 task는 항상 등록되어 있다.
- 하지만 역할과 enable 플래그에 따라 대부분의 task는 초기에 바로 return한다.
- 예를 들어 slave2 프로젝트에도 `CAN task`, `UART task`, `button task`가 등록되어 있지만 실제로는 비활성 상태다.

### 7.3 이 설계의 장단점

- 장점
  - 공통 스케줄러 하나로 모든 역할을 커버할 수 있다.
  - 코드 흐름이 단순하다.
- 단점
  - 역할별 최적화가 덜 되어 있다.
  - 사용하지 않는 task도 계속 체크한다.
  - task 우선순위나 선점 개념이 없다.

---

## 8. infra 레이어 분석

`infra`는 시스템 공통 유틸리티를 담는 가장 하위의 내부 공통 계층이다.

### 8.1 infra_types.h

주요 역할:

- `InfraStatus`
  - 공통 반환 코드
- `INFRA_ARRAY_COUNT`
  - 배열 개수 계산
- 시간 관련 inline 함수
  - `Infra_TimeElapsedMs`
  - `Infra_TimeIsExpired`
  - `Infra_TimeIsDue`

이 파일은 사실상 모든 모듈의 공통 기반 타입 정의 역할을 한다.

### 8.2 runtime_task.c

주요 역할:

- task 테이블 초기화
- 현재 시각 기준으로 due task 실행

핵심 포인트:

- `RuntimeTask_ResetTable()`
  - 모든 task의 `last_run_ms`를 시작 시각으로 맞춤
- `RuntimeTask_RunDue()`
  - 각 task에 대해 due 여부를 검사하고 실행

구조적으로 매우 단순한 round-robin dispatcher다.

### 8.3 runtime_tick.c

주요 역할:

- LPTMR 기반 base tick 생성
- 500 us interrupt를 누적해서 ms time 생성
- hook 등록 기능 제공

중요한 내부 전역 상태:

- `g_runtime_tick_ms`
- `g_runtime_tick_base_count`
- `g_runtime_tick_us_accumulator`
- `g_runtime_tick_hooks[]`

핵심 의미:

- 시스템 전체의 시간 기준은 여기서 만들어진다.
- LIN timeout 서비스는 이 tick hook을 통해 ISR 레벨에서 유지된다.

즉, `lin_module`이 자체적으로 시간을 세는 것이 아니라, `runtime_tick`이 만든 흐름 위에 올라타는 구조다.

### 8.4 infra_queue.c

주요 역할:

- 범용 ring queue 제공

사용되는 곳:

- UART TX queue
- AppConsole CAN command queue
- CanModule request queue
- 기타 내부 메시지 완충

특징:

- 정적 메모리 기반
- item 크기/개수 지정 가능
- push/pop/peek 제공

장점:

- malloc 없이 deterministic 하다.

단점:

- thread-safe 하지 않다.
- full/empty 처리만 있고 고급 정책은 없다.

---

## 9. runtime 레이어 분석

`runtime`은 시스템 orchestration 계층이다.

### 9.1 runtime.c

이 파일은 시스템의 중앙 조립기다.

주요 구성 요소:

- `RuntimeContext`
  - `initialized`
  - `init_status`
  - `AppCore app`
  - `RuntimeTaskEntry tasks[9]`
- 전역 싱글턴 `g_runtime`

핵심 역할:

- 시스템 초기화
- task table 구성
- super-loop 실행
- `AppCore`의 lifetime 관리

### 9.2 runtime_io.c

이 파일은 가장 중요한 **platform binding layer**다.

모듈 관점에서 보면:

- `adc_module`
  - 하드웨어 샘플 함수 제공
- `lin_module`
  - SDK init/send/recv/timeout binding 제공
- `led_module`
  - GPIO pin 설정 정보 제공
- `app`
  - 현재 role/node id 제공
- slave1
  - 버튼 읽기 함수 제공

즉, 다른 모듈들은 가능한 한 SDK 헤더를 직접 몰라도 되고, `runtime_io`를 통해 하드웨어 자원을 주입받는다.

### 9.3 runtime_io가 중요한 이유

이 파일은 아래 두 세계를 연결한다.

- 상위 공통 모듈 세계
  - `LinModule`
  - `AdcModule`
  - `LedModule`
- 하위 보드/SDK 세계
  - `LIN_DRV_*`
  - `ADC_DRV_*`
  - `PINS_DRV_*`
  - clock manager

즉, `runtime_io`는 **포팅 포인트**다.

보드를 바꾸거나 generated peripheral 이름이 바뀌면 가장 먼저 수정해야 할 곳이 여기다.

---

## 10. app 레이어 분석

`app`은 시스템 정책을 담당하는 레이어다.

이 레이어는 "어떤 통신 API를 쓸까"보다, "지금 이 상태에서 무엇을 해야 하는가"를 결정한다.

### 10.1 app_core.c: 공통 app orchestration

`AppCore`는 시스템 핵심 상태 컨테이너다.

안에 들어 있는 것:

- 역할 정보
- 어떤 기능이 enable 되었는지
- slave1 버튼 debounce 상태
- master emergency 상태
- master 승인 대기 상태
- task 카운터
- UART console 인스턴스
- CAN 모듈 인스턴스
- LIN 모듈 인스턴스
- LED/ADC 인스턴스
- 화면 표시용 문자열 버퍼

즉, `AppCore`는 "전역 상태를 구조체 하나로 모은 형태"다.

#### AppCore의 실질 책임

- 역할별 초기화 분기
- 각 주기 task에서 역할별 로직 연결
- 통신 결과를 사람이 읽는 문자열로 변환
- UI 표시 상태 유지

#### AppCore가 좋은 점

- 전체 상태를 한 곳에서 보기 쉽다.
- task별 entry point가 깔끔하다.

#### AppCore가 아쉬운 점

- 역할별 필드가 한 구조체에 모두 섞여 있다.
- master/slave1/slave2 전용 상태가 혼재한다.
- 결과적으로 구조체가 비대하고 역할 의미가 흐려진다.

### 10.2 app_master.c: 판단 정책

이 파일은 시스템 판단의 중심이다.

핵심 기능:

- console + CAN + LIN master 초기화
- LIN status 해석
- emergency 상태 판단
- slave1 OK 요청 처리
- slave2 latch 해제 요청(LIN OK token)
- slave1 최종 승인(CAN OK)

#### 핵심 정책 흐름

1. slave2의 LIN status 수신
2. `zone == emergency` 또는 `emergency_latched == 1`이면 emergency로 판단
3. emergency edge 발생 시 slave1에 `CAN_CMD_EMERGENCY` 전송
4. slave1이 버튼을 눌러 `CAN_CMD_OK`를 보내면 master가 이를 승인 요청으로 해석
5. ADC가 아직 emergency면 승인 거부
6. emergency는 해제됐지만 latch가 남아 있으면 LIN OK token 전송
7. slave2가 latch를 해제하면 slave1에 `CAN_CMD_OK` 전송

즉, 이 파일은 "센서 상태와 현장 승인 절차를 조합하는 정책 엔진"이다.

### 10.3 app_slave1.c: 현장 반응 정책

이 파일의 책임은 단순하고 명확하다.

- CAN 명령을 받아 현장 장치(LED)에 반영
- 버튼 입력을 debounce하여 master에 보고

모드 개념:

- `NORMAL`
- `EMERGENCY`
- `ACK_BLINK`

#### 주요 의미

- `CAN_CMD_EMERGENCY`
  - 빨강 고정
  - "press ok" 상태 표시
- `CAN_CMD_OK`
  - 초록 blink
  - 승인 완료 표시
- 버튼 눌림
  - emergency 상태일 때만 master로 `CAN_CMD_OK` 송신

### 10.4 app_slave2.c: 센서 제공 정책

이 파일은 ADC와 LIN을 연결하는 역할을 한다.

핵심 기능:

- ADC snapshot 읽기
- ADC 값을 zone으로 해석
- `LinStatusFrame` 생성
- LIN slave cache에 게시
- zone/latch 상태에 따라 LED 패턴 변경
- OK token 수신 시 latch clear 시도

즉, `slave2`는 단순 센서 raw provider가 아니라, **센서 의미 해석 + 상태 제공 노드**다.

### 10.5 app_console.c: 사람과 시스템 사이의 인터페이스

이 파일은 UART 콘솔 처리 전담이다.

책임은 크게 두 가지다.

- 입력 명령 파싱
- 상태 화면 렌더링

지원 명령:

- `help`
- `hello`
- `ping`
- `status`
- `ok`
- `open <id|all>`
- `close <id|all>`
- `off <id|all>`
- `test <id|all>`
- `text <id|all> <msg>`
- `event <id|all> <eventCode> <arg0> <arg1>`

구조적으로 보면:

- 콘솔은 CAN 요청을 직접 보내지 않는다.
- 먼저 `AppConsoleCanCommand` queue에 적재한다.
- 그다음 `AppCore_TaskCan()`이 이 queue를 꺼내 실제 `CanModule` 요청으로 변환한다.

이 설계 덕분에 `console`과 `CAN transport`가 직접 결합되지 않는다.

---

## 11. UART 레이어 분석

UART는 두 단계로 분리되어 있다.

### 11.1 uart_hw.c

역할:

- SDK와 직접 연결되는 저수준 레이어
- 초기화
- 전송 시작
- 전송 상태 확인
- RX callback 수신

특징:

- RX는 인터럽트 callback 기반
- 수신된 바이트를 `rx_pending` ring에 넣음

즉, ISR 레벨에서는 가능한 한 짧게 끝내고, 실제 line 조립은 상위 서비스 레이어에서 처리한다.

### 11.2 uart_service.c

역할:

- RX pending byte를 line buffer로 조립
- backspace 처리
- line overflow 처리
- TX chunk queue 관리
- TX timeout 감시
- 오류 복구 지원

핵심 데이터:

- `UartRxPendingRing`
- `UartLineBuffer`
- `UartTxContext`
- `UartService`

설계적으로 보면:

- `uart_hw`는 바이트 수준 하드웨어 처리
- `uart_service`는 줄 단위 사용자 입력 처리

즉, 하드웨어 제어와 사용자 인터페이스 의미가 잘 분리되어 있다.

---

## 12. CAN 레이어 분석

CAN 쪽은 이 프로젝트에서 가장 계층 분리가 잘 된 부분이다.

### 12.1 CAN 레이어 전체 구조

```text
AppCore / AppMaster / AppSlave1
        |
        v
   CanModule
        |
        v
   CanService
        |
        +--> CanProto
        |
        v
   CanTransport
        |
        v
     CanHw
        |
        v
   FLEXCAN SDK
```

### 12.2 can_hw.c

역할:

- FlexCAN mailbox 초기화
- TX 시작
- RX mailbox 상태 확인
- RX frame queue 보관

특징:

- `g_can_hw_rx_msg`라는 SDK RX 버퍼를 static으로 유지
- 수신된 frame을 내부 queue에 복사
- transport 상위 레이어는 raw frame queue만 보면 됨

### 12.3 can_proto.c

역할:

- `CanMessage` <-> `CanFrame` 변환
- protocol version 검증
- node id 검증
- text payload 검증

message type:

- command
- response
- event
- text

즉, "CAN 전기적 frame"이 아니라, "논리적 메시지 포맷"을 정의하는 계층이다.

### 12.4 can_service.c

역할:

- request id 관리
- pending request table 유지
- timeout 감시
- response/result queue 관리
- incoming message filtering

핵심 구조체:

- `CanPendingRequest`
- `CanTransport`
- `CanService`
- `CanServiceResult`

핵심 기능:

- 응답이 필요한 command를 보낼 때 pending slot 확보
- response가 오면 request_id와 source_node_id로 매칭
- timeout이 나면 result queue에 timeout 결과 적재

즉, `CanService`는 이 코드베이스의 **신뢰성 레이어**다.

### 12.5 can_module.c

역할:

- app이 사용하기 쉬운 high-level CAN API 제공
- request queue를 두고 점진적으로 `CanService`에 submit
- 한 tick당 최대 submit 개수 제한

왜 필요한가:

- app은 매 순간 여러 명령을 생성할 수 있다.
- 하지만 하드웨어/서비스 레이어는 한 번에 너무 많은 요청을 밀어넣는 것을 원치 않는다.

따라서 `CanModule`은 앱과 CAN 서비스 사이의 완충지대 역할을 한다.

---

## 13. LIN 레이어 분석

LIN은 CAN보다 더 "상태기계 중심"으로 설계되어 있다.

### 13.1 LIN 레이어 구조

```text
AppMaster / AppSlave2
        |
        v
     LinModule
        |
        v
   runtime_io binding
        |
        v
     LIN SDK callback
```

### 13.2 lin_module.c의 기본 개념

`LinModule`은 master/slave 공용 상태기계다.

내부 핵심 상태:

- `state`
  - `IDLE`
  - `WAIT_PID`
  - `WAIT_RX`
  - `WAIT_TX`
- `flags`
  - PID OK
  - RX DONE
  - TX DONE
  - ERROR
- `current_pid`
- `ok_tx_pending`
- `ok_token_pending`
- `latest_status`
- `slave_status_cache`

### 13.3 master 모드 동작

master는 두 개의 주기 흐름으로 움직인다.

- `TaskPoll`
  - poll 주기가 되면 status PID 또는 OK PID를 보냄
- `TaskFast`
  - callback에서 들어온 event flag를 보고 상태기계를 진행

즉, header 전송은 poll task가 하고, 그 이후 RX/TX 완료 처리와 에러 처리는 fast task가 한다.

### 13.4 slave 모드 동작

slave는 스스로 poll을 시작하지 않는다.

동작 방식:

1. master가 PID를 보냄
2. callback이 `PID_OK`를 알림
3. PID가 status면 slave status cache를 TX 시작
4. PID가 OK면 1바이트 token RX 시작
5. token 값이 맞으면 `ok_token_pending = 1`

즉, slave LIN은 **수동 응답형 구조**다.

### 13.5 LIN과 App의 관계

- master app
  - `LinModule_ConsumeFreshStatus()`로 새 ADC 상태 소비
  - `LinModule_RequestOk()`로 OK token 요청
- slave2 app
  - `LinModule_SetSlaveStatus()`로 최신 상태 게시
  - `LinModule_ConsumeSlaveOkToken()`으로 승인 token 소비

이 경계가 명확해서, app 쪽은 LIN callback 세부 구현을 직접 몰라도 된다.

---

## 14. ADC 레이어 분석

ADC는 비교적 단순하지만 의미가 매우 중요하다.

### 14.1 adc_module.c의 책임

- ADC raw sample 획득
- threshold 기반 zone 판정
- emergency latch 유지
- snapshot 제공

zone 구분:

- `SAFE`
- `WARNING`
- `DANGER`
- `EMERGENCY`

### 14.2 emergency latch의 의미

이 모듈의 핵심은 `emergency_latched`다.

동작 규칙:

- 한번 `EMERGENCY`에 들어가면 latch = 1
- 이후 raw 값이 내려와도 자동으로 clear되지 않음
- master가 승인 과정을 수행하고, slave2가 OK token을 받아야 clear 가능

즉, 시스템은 단순 threshold 경보가 아니라 **승인 기반 복구 모델**을 갖는다.

### 14.3 App과의 관계

- `AdcModule_Task()`
  - 주기 sample
- `AdcModule_GetSnapshot()`
  - app이 최신 상태 읽기
- `AdcModule_ClearEmergencyLatch()`
  - 승인 후 latch clear 시도

---

## 15. LED 레이어 분석

LED 제어는 semantic pattern 기반이다.

### 15.1 led_module.c의 역할

- LED 핀 초기화
- 빨강/초록 GPIO 제어
- 의미 단위 패턴 적용
- blink 토글 진행

패턴 종류:

- `OFF`
- `GREEN_SOLID`
- `RED_SOLID`
- `YELLOW_SOLID`
- `RED_BLINK`
- `GREEN_BLINK`

### 15.2 설계 특징

app은 아래처럼 의미만 표현한다.

- emergency면 red solid
- 승인 완료면 green blink
- warning이면 yellow solid

그리고 실제 active level, red pin, green pin은 `runtime_io`와 `led_module`이 숨긴다.

즉, `policy`와 `hardware polarity`가 분리되어 있다.

---

## 16. 역할별 end-to-end 동작 시나리오

### 16.1 평상시 모니터링 흐름

1. slave2가 ADC를 읽음
2. ADC zone을 계산
3. slave2가 `LinStatusFrame`을 갱신
4. master가 poll 주기에 status PID 송신
5. slave2가 status frame 응답
6. master가 `latest_status`를 읽어 화면 표시

### 16.2 emergency 발생 흐름

1. ADC 값이 emergency 구간에 진입
2. slave2 snapshot의 `zone = emergency`
3. `emergency_latched = 1`
4. master가 새 LIN status 수신
5. master가 `master_emergency_active = 1`로 전환
6. master가 slave1에 `CAN_CMD_EMERGENCY` 송신
7. slave1이 빨강 고정 LED로 변경

### 16.3 사용자 확인 흐름

1. 사용자가 slave1 버튼 누름
2. slave1이 debounce 후 master에 `CAN_CMD_OK` 송신
3. master가 승인 요청으로 해석
4. ADC가 아직 emergency면 거절
5. ADC가 emergency에서 벗어났고 latch만 남아 있으면 LIN OK token 송신
6. slave2가 token을 받고 latch clear 시도
7. master가 이후 status에서 latch 해제 확인
8. master가 slave1에 `CAN_CMD_OK` 송신
9. slave1이 green blink 수행 후 normal 복귀

### 16.4 UART 콘솔 흐름

1. UART RX callback이 바이트 수신
2. `UartService_ProcessRx()`가 line buffer 조립
3. `AppConsole_Task()`가 명령 파싱
4. CAN 관련 명령이면 console 내부 queue에 적재
5. `AppCore_TaskCan()`이 이를 실제 CAN 요청으로 변환
6. 결과는 다시 console 화면 문자열로 표시

---

## 17. 모듈 간 상관관계

### 17.1 높은 수준 의존 방향

```text
main
 -> runtime
 -> app_core
 -> role-specific app
 -> feature modules
 -> runtime_io binding
 -> SDK
```

### 17.2 앱 중심 관계

- `AppCore`
  - 모든 task entry의 허브
  - 역할별 정책 파일과 feature module 사이 연결자
- `AppMaster`
  - `LinModule`과 `CanModule`을 조합
- `AppSlave1`
  - `CanModule`과 `LedModule`과 button input을 조합
- `AppSlave2`
  - `AdcModule`과 `LinModule`과 `LedModule`을 조합
- `AppConsole`
  - `UartService`를 통해 사람과 상호작용

### 17.3 숨은 결합 포인트

아래는 코드상 깔끔해 보이지만 실제로는 강하게 연결된 부분이다.

- `RuntimeTick` -> `AppCore_OnTickIsr` -> `LinModule_OnBaseTick`
  - LIN timeout 유지가 tick hook에 숨어 있음
- `RuntimeIo_AttachLinModule`
  - 현재 활성 LIN module 포인터를 global로 보관
  - SDK callback이 이 포인터를 통해 상위 module에 event 전달
- `AppCore`의 화면 문자열 버퍼
  - 정책/상태/UI가 한 구조체에 섞여 있음
- `CanService`의 `request_id` 관리
  - app은 몰라도 되지만 response 매칭이 서비스 내부 정책에 의존

---

## 18. 핵심 구조체 정리

### 18.1 RuntimeContext

역할:

- 시스템 전체 runtime 인스턴스

구성:

- 초기화 상태
- `AppCore`
- task table

의미:

- 프로그램의 실제 전역 런타임 루트

### 18.2 AppCore

역할:

- 앱 전체의 상태 허브

중요 필드:

- `role`
- `local_node_id`
- `console_enabled`, `can_enabled`, `lin_enabled`, `adc_enabled`
- `slave1_mode`
- `master_emergency_active`
- `master_slave1_ok_pending`
- `lin_last_reported_zone`
- `lin_last_reported_lock`
- `console`, `can_module`, `lin_module`, `adc_module`, `slave1_led`, `slave2_led`

### 18.3 AppConsole

역할:

- UART 기반 HMI 컨텍스트

중요 필드:

- `uart`
- `can_cmd_queue`
- `local_ok_pending`
- `view`

### 18.4 CanService

역할:

- CAN request/response/timeout 관리의 핵심

중요 필드:

- `next_request_id`
- `pending_table`
- `incoming_queue`
- `result_queue`
- `transport`
- `proto`

### 18.5 LinModule

역할:

- master/slave 공용 LIN 상태기계

중요 필드:

- `state`
- `flags`
- `current_pid`
- `ok_tx_pending`
- `ok_token_pending`
- `latest_status`
- `slave_status_cache`

### 18.6 AdcModule

역할:

- ADC snapshot과 zone/latch 상태 유지

### 18.7 LedModule

역할:

- semantic LED pattern 실행

### 18.8 UartService

역할:

- line-oriented UART IO 관리

---

## 19. 전역/static 상태 정리

이 코드베이스는 외부 global을 남발하지는 않지만, file-static 상태는 꽤 사용한다.

대표 예시:

- `g_runtime`
  - runtime 전체 singleton
- `g_runtime_tick_ms`, `g_runtime_tick_hooks`
  - 시스템 시간 및 ISR hook
- `g_runtime_io_lin_module`
  - 현재 LIN callback 대상 module 포인터
- `g_runtime_io_adc_context`
  - ADC SDK context
- `g_runtime_io_lin_context`
  - LIN SDK context
- `g_can_hw_rx_msg`
  - FlexCAN SDK RX mailbox 버퍼
- `g_app_console_help_text`
  - 콘솔 help 문자열

이 구조는 단일 인스턴스 시스템에는 적합하지만, 다중 인스턴스/테스트 환경에는 불리하다.

---

## 20. 설계상 장점

### 20.1 역할 분리의 의도가 명확하다

- 판단은 master
- 현장 반응은 slave1
- 센서와 latch는 slave2

이 역할 분리가 도메인 의미와 잘 맞는다.

### 20.2 레이어 경계가 비교적 깔끔하다

- app은 정책
- module은 기능
- runtime_io는 SDK binding
- infra는 공통 유틸

특히 CAN은 레이어 분리도가 상당히 좋다.

### 20.3 동적 메모리를 쓰지 않는다

- queue와 context가 모두 정적 메모리 기반
- 임베디드에서 예측 가능성이 높다

### 20.4 에러 승인 절차가 단계적으로 드러난다

- 단순 threshold 경보가 아니라
- 사용자 승인
- latch 해제
- 최종 승인 LED 반영

이라는 절차가 명확하다.

---

## 21. 부족한 점과 개선 제안

### 21.1 프로젝트 중복 제거 필요

현재 가장 큰 구조적 문제는 3개 프로젝트가 거의 완전히 동일하다는 점이다.

개선 방향:

- 공통 라이브러리 1개
- 역할별 얇은 entry project 3개
- `APP_ACTIVE_ROLE`만 역할별 빌드 설정으로 분리

### 21.2 AppCore가 너무 많은 책임을 가진다

현재 `AppCore`는 아래를 모두 가진다.

- 상태 저장
- UI 텍스트 저장
- 역할 전환 허브
- task entry
- module 인스턴스 보관

개선 방향:

- `AppCoreCommon`
- `AppMasterContext`
- `AppSlave1Context`
- `AppSlave2Context`

처럼 역할별 context 분리

### 21.3 역할별 미사용 모듈이 같은 이미지에 남아 있다

slave 프로젝트에도 master용 CAN/LIN/UART 코드가 함께 들어 있다.

개선 방향:

- conditional compilation
- 역할별 빌드 타깃 분리

### 21.4 ISR와 main loop 간 공유 상태 보호가 약하다

특히 `LinModule`은 callback과 main loop가 같은 상태를 공유한다.

개선 방향:

- event queue 도입
- critical section 보호
- 더 명확한 ISR-to-main handoff 구조

### 21.5 오류 처리와 재시도 정책이 약하다

예시:

- CAN 전송 실패 시 상위 정책 차원의 복구가 약함
- ADC sample error가 시스템 정책에 충분히 반영되지 않음
- LIN 에러 발생 시 상태 표시가 제한적임

개선 방향:

- 오류 상태 전파 강화
- retry/backoff 정책 명시
- 콘솔에 더 구체적인 fault 상태 노출

### 21.6 하드웨어 바인딩이 generated SDK 심볼에 강하게 결합되어 있다

예시:

- `INST_LIN2`
- `INST_ADC_CONFIG_1`
- `INST_LPUART_1`
- `INST_FLEXCAN_CONFIG_1`

개선 방향:

- 보드 포팅 가이드 정리
- binding 설정을 헤더/설정 파일로 추상화

### 21.7 일부 필드/설정은 실사용도가 낮다

예시:

- `CanModule.default_target_node_id`
- `AppConsoleCanCommand.target_is_broadcast`
- `AdcConfig.blocking_mode`

개선 방향:

- 실제 쓰지 않는 필드 정리
- future use라면 주석으로 목적 명시

### 21.8 task 스케줄링 정밀도 개선 여지

현재는 due가 되면 `last_run_ms = now_ms`로 갱신하는 단순 방식이라 drift가 누적될 수 있다.

개선 방향:

- fixed-rate scheduling
- overruns 감지
- task 실행 시간 측정

### 21.9 테스트 레이어가 보이지 않는다

현재 구조는 모듈화는 잘 되어 있지만, 테스트 코드가 없다면 회귀 방지가 어렵다.

추천 테스트 우선순위:

- `CanProto` encode/decode 테스트
- `CanService` timeout/response 매칭 테스트
- `AdcModule` threshold/latch 테스트
- `LinModule` master/slave state machine 테스트
- `AppMaster` 승인 시나리오 테스트

---

## 22. 이 코드를 이해할 때 추천하는 읽기 순서

### 1단계: 시스템 뼈대 이해

- `main.c`
- `runtime/runtime.c`
- `infra/runtime_task.c`
- `infra/runtime_tick.c`

### 2단계: 앱 허브 이해

- `app/app_core.h`
- `app/app_core.c`

### 3단계: 역할별 정책 이해

- `app/app_master.c`
- `app/app_slave1.c`
- `app/app_slave2.c`

### 4단계: 통신/기능 모듈 이해

- `lin/lin_module.c`
- `can/can_module.c`
- `can/can_service.c`
- `can/can_proto.c`
- `adc/adc_module.c`
- `led/led_module.c`
- `uart/uart_service.c`
- `app/app_console.c`

### 5단계: 보드 바인딩 이해

- `runtime/runtime_io.c`

이 순서로 보면 "왜 이런 구조인지"가 가장 잘 보인다.

---

## 23. 최종 정리

`re_2`는 단순한 주변장치 예제가 아니라, **세 개의 역할 노드가 협력하는 상태 기반 제어 시스템**이다.

핵심 설계 의도는 아래와 같이 정리할 수 있다.

- `slave2`
  - 센서 의미를 만들어 제공한다.
- `master`
  - 그 의미를 해석해 시스템 결정을 내린다.
- `slave1`
  - 결정에 따라 사용자에게 반응하고, 다시 사용자 승인 의사를 전달한다.

코드 구조 역시 이 도메인 모델을 상당 부분 잘 반영하고 있다.

- 정책은 `app`
- 기능은 `can/lin/adc/led/uart`
- 하드웨어 연결은 `runtime_io`
- 실행 기반은 `runtime/infra`

다만 현재는 이 좋은 구조가 **프로젝트 복제 방식** 때문에 유지보수 측면에서 다소 약해져 있다.

따라서 앞으로 이 코드를 발전시키려면 아래 순서가 가장 효과적이다.

1. 공통 코드베이스 통합
2. 역할별 context 분리
3. LIN/CAN 오류 처리 강화
4. 테스트 추가
5. 포팅 설정 정리

그렇게 되면 현재의 구조적 장점은 유지하면서도, 훨씬 읽기 쉽고 안전한 코드베이스로 발전시킬 수 있다.
