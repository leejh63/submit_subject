# S32K 세 프로젝트 구조체 해설

이 문서는 현재 폴더의 세 프로젝트 `S32K_LinCan_master`, `S32K_Can_slave`, `S32K_Lin_slave`에 있는 구조체를 기준으로 정리했다.
중복된 코드가 많은 구조체는 한 번만 설명하고, 프로젝트마다 실제로 채워지는 값이 다른 경우에는 따로 적었다.

## 0. 먼저 보는 법

이 코드베이스의 구조체는 크게 네 종류로 나뉜다.

- `Config` 계열: 시작할 때 한 번 채워 넣는 정적 설정 묶음이다.
- `Module` / `Service` / `Context` 계열: 런타임 상태를 들고 있는 주인 객체다.
- `Snapshot` / `Frame` / `Message` 계열: 최근 데이터나 교환 데이터 한 덩어리를 담는 운반체다.
- `Queue` / `Pending` 계열: 비동기 흐름을 끊김 없이 이어 주는 버퍼링 상태다.

구조체를 읽을 때는 항상 아래 순서로 보면 이해가 빠르다.

1. 누가 이 구조체를 소유하는가
2. 언제 값이 채워지는가
3. 값이 한 번만 쓰이는 설정인지, 계속 바뀌는 상태인지
4. 이 값이 없으면 어떤 로직이 불가능해지는가

## 1. 공통 Core 구조체

### `InfraQueue`
- 정의: `core/infra_queue.h`
- 성격: 공통 ring queue 메타데이터
- 실제 사용처:
  - master 콘솔 CAN 명령 queue: 용량 4
  - UART TX queue: 용량 8
  - CAN module request queue: 용량 8

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `buffer` | 실제 저장공간 시작 주소 | queue 구현을 공통화하면서도 저장공간 소유권은 각 모듈이 갖게 하려는 설계다. 구조체 안에 메모리를 직접 박지 않은 이유다. |
| `item_size` | 원소 하나 크기 | `AppConsoleCanCommand`, `UartTxChunk`, `CanModuleRequest`처럼 서로 다른 타입을 같은 queue 코드로 다루기 위해 필요하다. |
| `capacity` | 최대 원소 수 | 고정 메모리 환경에서 overflow 판단 기준이 된다. master 콘솔 queue는 4, UART TX는 8, CAN module 요청도 8이다. |
| `head` | 다음 pop 위치 | ring queue에서 가장 오래된 원소 위치를 가리킨다. |
| `tail` | 다음 push 위치 | 새 원소를 넣을 위치다. |
| `count` | 현재 저장 개수 | empty/full을 빠르게 판단하려고 둔다. `head == tail`만으로는 full/empty를 구분하기 까다롭기 때문이다. |

핵심 포인트는 이 구조체가 “데이터 자체”가 아니라 “버퍼를 운영하는 메타데이터”라는 점이다.

### `RuntimeTaskEntry`
- 정의: `core/runtime_task.h`
- 성격: cooperative scheduler의 task 테이블 한 칸

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `name` | task 이름 문자열 | 디버깅과 가독성용이다. 현재는 런타임 내부에서 주로 의미를 갖고, 로그 출력용 확장 여지도 있다. |
| `period_ms` | 실행 주기 | super-loop 기반 스케줄러가 “언제 돌려야 하는지” 판단하는 핵심 값이다. |
| `last_run_ms` | 마지막 실행 시각 | `Infra_TimeIsDue()`로 다음 실행 여부를 계산하려면 기준 시각이 필요하다. |
| `task_fn` | 실행할 함수 포인터 | scheduler가 데이터 중심으로 task를 돌리기 위해 필요하다. |
| `context` | task 함수에 넘길 대상 객체 | 대부분 `AppCore` 주소가 들어간다. task 함수 시그니처를 공통으로 유지하려고 둔다. |

프로젝트별 실제 task 테이블은 아래처럼 채워진다.

- `S32K_LinCan_master`
  - `uart`: 1ms
  - `lin_fast`: 1ms
  - `can`: 10ms
  - `lin_poll`: 20ms
  - `render`: 100ms
  - `heartbeat`: 1000ms
- `S32K_Can_slave`
  - `button`: 10ms
  - `can`: 10ms
  - `led`: 100ms
  - `heartbeat`: 1000ms
- `S32K_Lin_slave`
  - `lin_fast`: 1ms
  - `adc`: 20ms
  - `led`: 100ms
  - `heartbeat`: 1000ms

이 구조체가 좋은 이유는 task 추가/삭제를 코드 분기보다 “테이블 편집”으로 다룰 수 있다는 점이다.

### `RuntimeTickHookSlot`
- 정의: `core/runtime_tick.c` 내부 구조체
- 성격: ISR hook 등록 슬롯
- 내부 상수: hook 슬롯 수는 4개

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `hook` | ISR 시점에 호출할 함수 포인터 | base tick interrupt를 여러 모듈에 팬아웃하려면 콜백 저장소가 필요하다. |
| `context` | hook 호출 시 넘길 문맥 포인터 | 대체로 `AppCore` 주소가 들어간다. master와 LIN slave는 `AppCore_OnTickIsr`를 등록한다. |

이 구조체는 “ISR 안에서 누가 후속 처리할지”를 느슨하게 연결하는 장치다.

### `RuntimeContext`
- 정의: 각 프로젝트의 `runtime/runtime.c` 내부 구조체
- 성격: 실행 이미지 전체를 대표하는 최상위 runtime 컨테이너

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | runtime 준비 완료 여부 | `Runtime_Run()` 진입 전에 초기화 성공 여부를 확인하려고 둔다. |
| `init_status` | 마지막 초기화 결과 | 어떤 단계에서 실패했는지 유지하려는 진단용 값이다. 초기값은 `INFRA_STATUS_NOT_READY`다. |
| `app` | 현재 이미지의 `AppCore` | 실제 애플리케이션 상태의 소유자다. runtime은 이 객체를 task에 넘긴다. |
| `tasks[]` | task 테이블 배열 | 프로젝트별로 크기가 다르다. master는 6칸, 나머지 두 slave는 4칸이다. |

이 구조체는 “보드 초기화 + tick + app + task table”을 한 이미지 단위로 묶는 composition root 역할을 한다.

## 2. App 계층 구조체

### `AppConsoleCanCommand`
- 정의: `S32K_LinCan_master/app/app_console.h`
- 성격: 콘솔이 파싱한 CAN 지향 명령 한 건

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `type` | 명령 종류 | `OPEN`, `CLOSE`, `OFF`, `TEST`, `TEXT`, `EVENT` 중 하나다. AppCore가 switch로 해석한다. |
| `target_node_id` | 목적 노드 ID | 특정 slave 대상으로 보낼지, broadcast로 보낼지 결정한다. `all`이면 내부적으로 255가 들어간다. |
| `target_is_broadcast` | 사용자가 broadcast를 의도했는지 | 파서가 `all`을 구분하려고 저장하지만, 현재 AppCore 쪽에서는 사실상 쓰지 않는다. `target_node_id == 255`만으로도 충분해서 다소 중복적이다. |
| `text` | 짧은 텍스트 payload | `text <id|all> <msg>` 명령에서 사용된다. 최대 길이는 11자다. |
| `event_code` | 이벤트 코드 | `event` 명령에서 사용된다. |
| `arg0` | 첫 번째 보조 인자 | `event` 또는 향후 명령 확장용이다. |
| `arg1` | 두 번째 보조 인자 | `event` 또는 향후 명령 확장용이다. |

이 구조체를 둔 이유는 콘솔 파서가 CAN 모듈 내부 타입을 직접 알지 않게 만들기 위해서다.

### `AppConsoleView`
- 정의: `S32K_LinCan_master/app/app_console.h`
- 성격: 터미널 화면 캐시와 dirty flag 묶음

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `input_text[64]` | 현재 입력 줄 표시용 캐시 | 타이핑 중인 명령을 화면에 다시 그리기 위해 필요하다. |
| `task_text[160]` | task 상태 영역 문자열 | heartbeat, CAN, LIN, UART 상태를 여러 줄로 렌더링한다. |
| `source_text[128]` | 최근 입력 출처 문자열 | `from [can]`, `from [lin]` 영역에 쓴다. |
| `result_text[96]` | 결과 메시지 문자열 | 명령 처리 결과나 오류, help 문구가 들어간다. |
| `value_text[128]` | 값 표시 영역 문자열 | `Mode`, `Button`, `ADC`를 보여 준다. |
| `full_refresh_required` | 전체 화면 다시 그리기 요청 | recover 직후나 초기화 직후 전체 레이아웃을 다시 그릴 때 쓴다. |
| `input_dirty` | input 줄만 갱신 필요 | 바뀐 줄만 다시 그려 UART 트래픽을 줄이려는 목적이다. |
| `task_dirty` | task 영역 갱신 필요 | 동일한 이유다. |
| `source_dirty` | source 영역 갱신 필요 | 동일한 이유다. |
| `result_dirty` | result 영역 갱신 필요 | 동일한 이유다. |
| `value_dirty` | value 영역 갱신 필요 | 동일한 이유다. |
| `layout_drawn` | 고정 레이아웃 출력 여부 | 제목, 섹션 프레임 같은 정적 레이아웃을 최초 1회만 그리기 위해 필요하다. |

이 구조체는 성능보다 “UART 링크를 덜 낭비하는 텍스트 UI”를 만들기 위한 캐시다.

### `AppConsole`
- 정의: `S32K_LinCan_master/app/app_console.h`
- 성격: master 이미지의 전체 UART 콘솔 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | 콘솔 준비 완료 여부 | task 진입 전에 초기화 성공 여부를 확인하려고 둔다. |
| `node_id` | 현재 콘솔이 올라간 로컬 노드 ID | 현재 프로젝트에서는 master라서 1이 들어간다. title/prompt 표시에 사용된다. |
| `state` | 콘솔 상태 | 기본값은 `APP_CONSOLE_STATE_IDLE`, UART 실패 시 `ERROR`로 바뀐다. |
| `local_ok_pending` | 로컬 `ok` 명령 보류 플래그 | 사용자가 콘솔에서 `ok`를 치면 1이 되고, AppCore가 소비한 뒤 0으로 되돌린다. queue가 아니라 flag라서 여러 번 눌러도 1건으로 합쳐진다. |
| `uart` | UART 서비스 전체 상태 | 실제 바이트 RX/TX, 오류 추적, line buffer를 다 품는다. |
| `can_cmd_queue` | 파싱된 CAN 명령 queue | AppCore가 나중에 꺼내서 CAN module 요청으로 변환한다. 용량은 4다. |
| `can_cmd_storage[4]` | 위 queue의 실제 저장소 | 동적 메모리 없이 명령 4건까지 버퍼링하려는 의도다. |
| `view` | 화면 캐시 | 터미널 렌더링 최적화와 상태 표시를 담당한다. |

초기화 시 기본 view 문자열은 `waiting` 중심으로 채워져서, 실제 데이터가 오기 전에도 화면 형태가 무너지지 않는다.

### `AppCore` - master 버전
- 정의: `S32K_LinCan_master/app/app_core.h`
- 성격: master 애플리케이션의 중앙 상태 객체
- 실제 노드 ID: 1

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | app 준비 완료 여부 | `AppMaster_Init()`까지 끝났는지 확인하는 최상위 플래그다. |
| `local_node_id` | 내 CAN/LIN 논리 노드 ID | `RuntimeIo_GetLocalNodeId()`에서 1을 받아 넣는다. |
| `console_enabled` | 콘솔 활성화 여부 | `AppConsole_Init()` 성공 시 1이 된다. |
| `can_enabled` | CAN 경로 활성화 여부 | `CanModule_Init()` 성공 시 1이 된다. |
| `lin_enabled` | LIN 경로 활성화 여부 | `LinModule_Init()` 성공 시 1이 된다. |
| `master_emergency_active` | master가 현재 emergency로 판단하는지 | `status.zone == EMERGENCY` 또는 `latch != 0`이면 1이 된다. emergency 전이를 감지하는 핵심 상태다. |
| `master_slave1_ok_pending` | slave1 승인 절차가 진행 중인지 | slave1 버튼 승인 요청이 들어온 뒤, slave2 latch 해제와 slave1 CAN OK 전송 사이를 잇는 플래그다. |
| `can_last_activity` | 최근 CAN task에서 실제 활동이 있었는지 | render에서 `CAN: ok/idle`를 표시하려고 둔다. |
| `lin_last_reported_zone` | 마지막으로 화면에 보고한 zone | 초기값은 `0xFF` sentinel이다. 중복 메시지 출력 방지용이다. |
| `lin_last_reported_lock` | 마지막으로 화면에 보고한 latch 값 | 초기값은 `0xFF` sentinel이다. zone과 같은 이유다. |
| `heartbeat_count` | heartbeat task 실행 횟수 | 살아 있음 표시와 단순 진단용이다. |
| `uart_task_count` | UART task가 돌았던 횟수 | render 화면에 누적 카운터로 보여 준다. |
| `can_task_count` | CAN task에서 활동이 발생한 횟수 | 실제 CAN 의미 활동 수를 보기 위한 카운터다. |
| `console` | 콘솔 상태 전체 | operator 인터페이스의 중심이다. |
| `can_module` | app 지향 CAN facade | AppCore가 서비스 내부 구조를 몰라도 되게 해 준다. |
| `lin_module` | LIN 상태기계 | master/slave2 상태 교환과 OK token 전송을 맡는다. |
| `mode_text[32]` | 화면용 mode 문자열 | 기본값 `normal`, emergency 상황이면 `emergency` 등으로 바뀐다. |
| `button_text[32]` | 화면용 button 상태 | 기본값 `waiting`, 요청/승인/거절 상태를 보여 준다. |
| `adc_text[48]` | 화면용 ADC 요약 | 예: `1234 (warning, lock=1)` 형태가 들어간다. |
| `can_input_text[48]` | 최근 CAN 입력 설명 | slave1에서 OK 요청이 들어오면 `button in` 등으로 채운다. |
| `lin_input_text[48]` | 최근 LIN 입력 설명 | slave2의 센서 값 요약이 들어간다. |
| `lin_link_text[32]` | LIN 링크 상태 문자열 | `waiting`, `ok`, `ok pending`, `binding req` 등이 들어간다. |

이 구조체는 “정책 판단”과 “화면 표시 상태”가 함께 들어 있는 편이라, 학습용으로는 이해하기 쉽지만 실무에서는 표시 상태를 별도 view-model로 분리하기도 한다.

### `AppCore` - CAN slave 버전
- 정의: `S32K_Can_slave/app/app_core.h`
- 성격: slave1 현장 반응 노드의 중앙 상태
- 실제 노드 ID: 2

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | app 준비 완료 여부 | `AppSlave1_Init()` 완료 후 1이 된다. |
| `local_node_id` | 로컬 노드 ID | slave1이므로 2가 들어간다. |
| `can_enabled` | CAN 활성화 여부 | `CanModule_Init()` 성공 시 1이 된다. |
| `led1_enabled` | 로컬 RGB LED 활성화 여부 | `LedModule_Init()` 성공 시 1이 된다. |
| `slave1_mode` | 현재 현장 노드 모드 | `NORMAL`, `EMERGENCY`, `ACK_BLINK`로 바뀐다. 단순하지만 정책의 중심이다. |
| `slave1_last_sample_pressed` | 직전 raw 버튼 샘플 | debounce용이다. |
| `slave1_stable_pressed` | 확정된 버튼 상태 | 일정 횟수 같은 값이 들어왔을 때만 이 값이 바뀐다. |
| `slave1_same_sample_count` | 같은 raw 샘플 연속 횟수 | 바운스 필터 기준이다. 2회 이상 같아야 확정으로 본다. |
| `heartbeat_count` | heartbeat 누적 횟수 | 진단용이다. |
| `can_module` | CAN facade | master와의 통신 경로다. |
| `slave1_led` | 로컬 LED 모듈 | emergency red solid, approval green blink를 표현한다. |
| `mode_text[32]` | 내부 상태 문자열 | 기본값 `normal`이다. 콘솔은 없지만 상태 표현을 위해 남겨 둔 흔적이다. |
| `button_text[32]` | 버튼 상태 문자열 | 기본값 `ready`, 눌림/승인 등의 상태를 담는다. |
| `adc_text[48]` | ADC 상태 문자열 | 이 프로젝트는 ADC가 없어서 기본값 `n/a`다. 사실상 인터페이스 통일을 위한 필드다. |
| `can_input_text[48]` | 최근 CAN 입력 설명 | 예: `emergency in (1)` 같은 식으로 들어간다. |

이 구조체는 master용 `AppCore`와 인터페이스를 맞추려는 흔적이 있어, 실제 쓰이지 않는 표시용 텍스트가 일부 남아 있다.

### `AppCore` - LIN slave 버전
- 정의: `S32K_Lin_slave/app/app_core.h`
- 성격: slave2 센서 노드의 중앙 상태
- 실제 노드 ID: 3

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | app 준비 완료 여부 | `AppSlave2_Init()` 완료 후 1이 된다. |
| `local_node_id` | 로컬 노드 ID | slave2이므로 3이 들어간다. |
| `lin_enabled` | LIN 활성화 여부 | `LinModule_Init()` 성공 시 1이 된다. |
| `adc_enabled` | ADC 활성화 여부 | `AdcModule_Init()` 성공 시 1이 된다. |
| `led2_enabled` | 로컬 LED 활성화 여부 | `LedModule_Init()` 성공 시 1이 된다. |
| `heartbeat_count` | heartbeat 횟수 | 진단용 카운터다. |
| `lin_module` | LIN 상태기계 | master polling에 응답하는 핵심이다. |
| `slave2_led` | RGB LED 모듈 | 센서 zone/latch 상태를 로컬에 보여 준다. |
| `adc_module` | ADC 의미 해석 모듈 | raw 샘플을 zone과 latch로 바꾼다. |
| `adc_text[48]` | ADC 상태 문자열 | 예: `2500 (warning, lock=0)`처럼 채운다. |
| `lin_input_text[48]` | 최근 LIN 입력 설명 | `ok token in` 같은 문구가 들어간다. |
| `lin_link_text[32]` | LIN 링크 상태 문자열 | 기본값 `waiting`, 문제 시 `binding req` 등으로 바뀐다. |

이 구조체는 역할이 가장 단순해서, 공부 시작점으로 보기 좋은 편이다.

## 3. CAN 계층 구조체

### `CanFrame`
- 정의: `services/can_types.h`
- 성격: raw CAN frame 컨테이너

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `id` | CAN ID | 이 프로젝트는 표준 ID 네 개를 쓴다. command=`0x120`, response=`0x121`, event=`0x122`, text=`0x123`다. |
| `dlc` | 데이터 길이 | command/response/event는 8, text는 `5 + text_length`가 된다. |
| `data[16]` | payload 저장소 | 실제 프레임은 8바이트만 쓰지만, 상위 프로토콜 컨테이너와 맞추려고 16바이트 크기로 잡아 두었다. |
| `is_extended_id` | 확장 ID 사용 여부 | 현재 프로토콜은 표준 ID만 쓰므로 보통 0이다. |
| `is_remote_frame` | RTR 여부 | 현재 설계에서는 거의 0이다. |
| `timestamp_ms` | 수신 시각 | RX 완료 시 `now_ms`를 넣어 진단에 활용한다. |

### `CanMessage`
- 정의: `services/can_types.h`
- 성격: app/service가 이해하는 논리 메시지

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `version` | 프로토콜 버전 | 현재는 항상 `CAN_PROTO_VERSION_V1 = 1`이다. 향후 프로토콜 변경 여지를 위해 필요하다. |
| `message_type` | 메시지 종류 | `COMMAND`, `RESPONSE`, `EVENT`, `TEXT` 중 하나다. |
| `source_node_id` | 송신 노드 ID | master=1, slave1=2가 주로 사용된다. |
| `target_node_id` | 수신 대상 노드 ID | 특정 노드 또는 broadcast(255)다. |
| `request_id` | request-response 매칭 ID | 응답을 기대하는 command일 때만 1 이상 값이 들어간다. |
| `flags` | 메시지 플래그 | 현재 핵심은 `NEED_RESPONSE`다. |
| `payload_kind` | payload 해석 종류 | `CTRL_CMD`, `CTRL_RESULT`, `EVENT_DATA`, `TEXT_DATA`로 나뉜다. |
| `payload_length` | payload 길이 | command/response/event는 보통 3이다. |
| `payload[16]` | 일반 payload | command면 `[cmd,arg0,arg1]`, response면 `[result,detail,...]`, event면 `[event,arg0,arg1]`다. |
| `text_type` | text payload 종류 | 현재는 주로 `CAN_TEXT_USER`가 들어간다. |
| `text_length` | 텍스트 길이 | 1~11 범위여야 한다. |
| `text[12]` | NUL 종료 짧은 문자열 | text 메시지일 때만 의미 있다. |

이 구조체를 보면 이 CAN 프로토콜이 “짧은 command/event/text” 중심이라는 게 보인다.

### `CanServiceResult`
- 정의: `services/can_types.h`
- 성격: 추적 중인 요청의 완료 결과

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `kind` | 결과 종류 | 현재 구현상 실제로는 `RESPONSE`와 `TIMEOUT`만 나온다. `SEND_FAIL` enum은 정의만 있고 아직 생성되지 않는다. |
| `request_id` | 대응되는 요청 ID | 어떤 command의 결과인지 맞추기 위해 필요하다. |
| `source_node_id` | 결과를 보낸 쪽 노드 | response면 원격 응답 노드, timeout이면 원래 target 노드가 들어간다. 이름만 보면 헷갈릴 수 있어 주의가 필요하다. |
| `target_node_id` | 결과를 받는 로컬 노드 | 보통 local node ID다. |
| `command_code` | 원래 보낸 명령 코드 | timeout/result 메시지를 사람이 이해하는 텍스트로 만들 때 필요하다. |
| `result_code` | 응답 코드 | `OK`, `FAIL`, `TIMEOUT` 등으로 해석된다. |
| `detail_code` | 세부 오류 코드 | 지금은 대부분 0이지만 확장용 공간이다. |

### `CanProtoConfig`
- 정의: `services/can_proto.h`
- 성격: 프로토콜 codec 초기 설정

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `local_node_id` | 이 codec이 속한 로컬 노드 ID | master에서는 1, slave1에서는 2다. codec이 자기 정체성을 유지하게 하려는 값이다. |

### `CanEncodedFrameList`
- 정의: `services/can_proto.h`
- 성격: encode 결과 묶음

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `frames[1]` | encode된 frame 배열 | 현재 프로토콜이 single-frame만 지원해서 크기를 1로 고정했다. |
| `count` | 실제 frame 수 | 지금은 사실상 항상 1이다. 나중에 분할 전송을 붙일 여지를 남겨 둔 형태다. |

### `CanProto`
- 정의: `services/can_proto.h`
- 성격: CAN encoder/decoder 런타임 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | 초기화 완료 여부 | encode/decode 호출 보호용이다. |
| `local_node_id` | 로컬 노드 ID | config에서 복사해 온 값이다. |
| `decode_ok_count` | 정상 decode 횟수 | 프로토콜 진단용 통계다. |
| `decode_ignored_count` | 무시된 frame 수 | target 불일치 등에서 도움 된다. |
| `decode_invalid_count` | 잘못된 frame 수 | malformed frame 감시에 필요하다. |
| `decode_unsupported_count` | 지원하지 않는 frame 수 | 향후 프로토콜 버전/ID 확장 시 진단에 쓴다. |

### `CanPendingRequest`
- 정의: `services/can_service.h`
- 성격: 응답 대기 중인 요청 슬롯 한 칸

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `in_use` | 이 슬롯이 사용 중인지 | pending table 재사용을 위해 필요하다. |
| `request_id` | 요청 식별자 | response와 timeout을 정확히 매칭하려고 둔다. 1부터 증가하며 0은 피한다. |
| `target_node_id` | 원래 보낸 대상 노드 | 응답 source와 맞는지 검사하려고 필요하다. |
| `command_code` | 보낸 명령 | timeout/result를 사람이 읽는 메시지로 만들려면 필요하다. |
| `start_tick_ms` | 요청 시작 시각 | timeout 계산 기준이다. |
| `timeout_ms` | 허용 응답 대기 시간 | 현재 상위 설정에서 300ms가 들어간다. |

### `CanTransport`
- 정의: `services/can_service.h`
- 성격: raw CAN transport 운영 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | transport 준비 완료 여부 | 하드웨어 초기화 완료를 나타낸다. |
| `tx_in_flight` | HW 전송 진행 중 여부 | mailbox 완료를 polling으로 추적하려고 둔다. |
| `last_error` | transport 수준 마지막 오류 | `TX_QUEUE_FULL`, `HW_TX_FAIL`, `RX_QUEUE_FULL` 등을 기록한다. |
| `hw` | 실제 CAN 하드웨어 context | 아래 `CanHw` 전체를 품는다. |
| `tx_queue[8]` | 소프트웨어 TX queue 저장소 | App/service가 바로 HW 상태에 묶이지 않게 하려는 버퍼다. |
| `tx_head` | TX pop 인덱스 | ring buffer 관리용이다. |
| `tx_tail` | TX push 인덱스 | ring buffer 관리용이다. |
| `tx_count` | TX queue 개수 | full/empty 판단용이다. |
| `rx_queue[8]` | 소프트웨어 RX queue 저장소 | HW에서 꺼낸 frame을 protocol decode 전에 잠시 적재한다. |
| `rx_head` | RX pop 인덱스 | ring buffer 관리용이다. |
| `rx_tail` | RX push 인덱스 | ring buffer 관리용이다. |
| `rx_count` | RX queue 개수 | full/empty 판단용이다. |
| `current_tx_frame` | 현재 HW에 올린 frame 복사본 | TX 완료 후 어떤 프레임이었는지 bookkeeping하려고 둔다. |
| `hw_tx_ok_count_seen` | 직전 확인한 HW TX 성공 카운터 | polling 완료 감지용이다. |
| `hw_tx_error_count_seen` | 직전 확인한 HW TX 실패 카운터 | polling 완료 감지용이다. |

이 구조체는 인터럽트 기반 큐가 아니라 polling 기반 queue bookkeeping이라는 점이 핵심이다.

### `CanService`
- 정의: `services/can_service.h`
- 성격: request/response 추적이 들어간 상위 CAN 서비스

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | 서비스 준비 완료 여부 | 호출 안전성 확보용이다. |
| `local_node_id` | 현재 서비스 소유 노드 | master는 1, slave1은 2다. |
| `next_request_id` | 다음에 쓸 요청 ID | 초기값은 1이고, 0은 예약처럼 비워 둔다. |
| `last_error` | 최근 service 오류 | invalid target, protocol error, queue full 등을 기록한다. |
| `default_timeout_ms` | 기본 응답 대기 시간 | 현재 `AppCore_InitConsoleCan()`에서 300ms로 넣는다. |
| `current_tick_ms` | 마지막 task 기준 현재 시각 | timeout 계산과 flush 시점에 쓴다. |
| `proto` | protocol codec | raw frame과 논리 메시지 사이를 변환한다. |
| `transport` | raw transport | 실제 HW/소프트웨어 queue 계층이다. |
| `pending_table[4]` | 응답 대기 슬롯 배열 | 최대 4개 요청만 동시에 추적한다. broadcast는 여기 안 들어간다. |
| `incoming_queue[8]` | AppCore로 넘길 수신 메시지 queue | response가 아닌 command/event/text가 들어온다. |
| `incoming_head` | incoming pop 인덱스 | ring buffer 관리용이다. |
| `incoming_tail` | incoming push 인덱스 | ring buffer 관리용이다. |
| `incoming_count` | incoming 개수 | full/empty 판단용이다. |
| `result_queue[8]` | 완료 결과 queue | response 매칭 결과나 timeout 결과가 들어간다. |
| `result_head` | result pop 인덱스 | ring buffer 관리용이다. |
| `result_tail` | result push 인덱스 | ring buffer 관리용이다. |
| `result_count` | result 개수 | full/empty 판단용이다. |

### `CanModuleRequest`
- 정의: `services/can_module.h`
- 성격: app 계층이 던지는 고수준 CAN 작업 한 건

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `kind` | 작업 종류 | `COMMAND`, `RESPONSE`, `EVENT`, `TEXT` 중 하나다. |
| `target_node_id` | 대상 노드 | 모듈이 아래 service에 넘길 목적지다. |
| `code0` | 첫 번째 코드성 값 | kind에 따라 의미가 다르다. command면 command code, response면 request_id, event면 event_code다. |
| `code1` | 두 번째 코드성 값 | command면 `arg0`, response면 `result_code`, event면 `arg0`다. |
| `code2` | 세 번째 코드성 값 | command면 `arg1`, response면 `detail_code`, event면 `arg1`다. |
| `need_response` | 응답 필요 여부 | command일 때만 사실상 의미가 있다. |
| `text[12]` | text payload | `TEXT` kind일 때 사용한다. |

이 구조체는 payload 해석이 kind에 따라 달라서, 실무에서는 union으로 더 명확히 표현할 수도 있다.

### `CanModuleConfig`
- 정의: `services/can_module.h`
- 성격: app-facing CAN module 초기 설정

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `local_node_id` | 로컬 노드 ID | master=1, slave1=2다. |
| `default_target_node_id` | 기본 대상 노드 | 현재 초기화 코드에서는 `memset` 후 따로 채우지 않아서 0이 들어간다. 저장은 되지만 실사용은 거의 없다. 설계 잔재에 가깝다. |
| `default_timeout_ms` | 기본 응답 timeout | 현재 300ms다. |
| `max_submit_per_tick` | 한 task tick에서 아래로 내려보낼 최대 요청 수 | 현재 2다. App이 한 번에 너무 많은 요청을 transport에 밀어 넣지 않게 하려는 값이다. |

### `CanModule`
- 정의: `services/can_module.h`
- 성격: AppCore가 직접 보는 CAN facade

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | module 준비 완료 여부 | 안전한 진입 보호용이다. |
| `local_node_id` | 내 노드 ID | config에서 복사된다. |
| `default_target_node_id` | 기본 대상 노드 | 현재는 저장만 하고 사실상 안 쓴다. |
| `max_submit_per_tick` | 틱당 제출 제한 | 현재 2다. |
| `last_activity` | 최근 tick에서 제출 활동이 있었는지 | AppCore가 activity 감지에 사용한다. |
| `service` | 실제 CAN service | protocol, transport, pending까지 다 품는다. |
| `request_queue` | app 요청 queue | AppCore가 넣고 module이 빼 간다. |
| `request_storage[8]` | 위 queue 저장소 | 최대 8건을 보류할 수 있다. |

### `CanHw`
- 정의: `drivers/can_hw.h`
- 성격: FlexCAN mailbox 기반 저수준 하드웨어 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | HW 초기화 완료 여부 | 드라이버 호출 보호용이다. |
| `local_node_id` | 현재 노드 ID | 하드웨어 입장에서는 직접 쓰는 값이 많지 않지만, 상위와 문맥을 맞추는 진단용으로 들고 있다. |
| `instance` | CAN 컨트롤러 인스턴스 | `IsoSdk_CanGetDefaultInstance()` 값이 들어간다. literal 값은 generated 설정에 숨어 있다. |
| `tx_mb_index` | TX mailbox 번호 | 현재 1이다. |
| `rx_mb_index` | RX mailbox 번호 | 현재 0이다. |
| `tx_busy` | 전송 중 여부 | mailbox polling 완료 전까지 1이다. |
| `last_error` | 마지막 HW 오류 | init 실패, RX queue full, restart 실패 등을 기록한다. |
| `rx_queue[8]` | HW에서 읽은 frame 버퍼 | polling 한 번에 여러 frame을 상위로 넘기기 전에 쌓아 둔다. |
| `rx_head` | RX pop 인덱스 | ring buffer 관리용이다. |
| `rx_tail` | RX push 인덱스 | ring buffer 관리용이다. |
| `rx_count` | RX 버퍼 개수 | full/empty 판단용이다. |
| `tx_ok_count` | TX 성공 누적 횟수 | 상위 transport가 완료를 감지할 때 본다. |
| `tx_error_count` | TX 실패 누적 횟수 | 동일한 이유다. |
| `rx_ok_count` | RX 성공 누적 횟수 | 진단용이다. |
| `rx_error_count` | RX 실패 누적 횟수 | 진단용이다. |
| `rx_drop_count` | HW RX는 성공했지만 소프트웨어 queue에 못 넣은 횟수 | burst traffic 취약성을 드러내는 중요한 카운터다. |

## 4. LIN 계층 구조체

### `LinBinding`
- 정의: `services/lin_module.h`
- 성격: portable LIN 상태기계가 의존하는 하드웨어 콜백 표

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `init_fn` | LIN HW 초기화 함수 | 현재 `LinHw_Init`이 들어간다. |
| `master_send_header_fn` | master header 송신 함수 | 현재 `LinHw_MasterSendHeader`가 들어간다. |
| `start_receive_fn` | 수신 시작 함수 | 현재 `LinHw_StartReceive`가 들어간다. |
| `start_send_fn` | 송신 시작 함수 | 현재 `LinHw_StartSend`가 들어간다. |
| `goto_idle_fn` | HW idle 복귀 함수 | 현재 `LinHw_GotoIdle`가 들어간다. |
| `set_timeout_fn` | timeout 갱신 함수 | 현재 `LinHw_SetTimeout`이 들어간다. |
| `service_tick_fn` | base tick 서비스 함수 | 현재 `LinHw_ServiceTick`이 들어간다. |
| `context` | 위 콜백들에 넘길 사용자 문맥 | 현재는 전부 `NULL`이다. 실제 문맥은 `LinHwState` 싱글톤이 대신 들고 있다. |

이 구조체 덕분에 `LinModule`은 SDK 심볼을 모른 채로 상태기계를 유지할 수 있다.

### `LinStatusFrame`
- 정의: `services/lin_module.h`
- 성격: slave2와 master 사이의 해석된 상태 프레임

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `adc_value` | 최신 ADC raw 값 | slave2가 샘플링한 0~4095 값이다. 실제 payload에서는 하위 2바이트에 들어간다. |
| `zone` | 의미 구간 | `SAFE`, `WARNING`, `DANGER`, `EMERGENCY` 중 하나다. master 정책 판단의 핵심이다. |
| `emergency_latched` | emergency latch 여부 | 센서가 emergency를 한 번 밟았는지 기억한다. master는 zone이 내려가도 이 latch를 보고 승인 절차를 유지한다. |
| `valid` | 이 상태가 유효한지 | 아직 한 번도 정상 수신/샘플링하지 못한 경우와 구분하려고 필요하다. |
| `fresh` | 새로 들어온 상태인지 | master가 같은 상태를 여러 번 처리하지 않도록 1회성 소비 플래그로 쓴다. |

실제 LIN status frame은 8바이트를 쓰지만, 의미 있게 채워지는 값은 주로 `adc_value`, `zone`, `emergency_latched`다.

### `LinConfig`
- 정의: `services/lin_module.h`
- 성격: LIN 모듈 정적 설정

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `role` | master/slave 역할 | master 프로젝트에서는 `LIN_ROLE_MASTER`, slave2에서는 `LIN_ROLE_SLAVE`가 들어간다. |
| `pid_status` | status frame용 PID | 현재 `0x24`다. master가 slave2 상태를 읽을 때 쓴다. |
| `pid_ok` | OK token용 PID | 현재 `0x25`다. master가 승인 token을 slave2에 보낼 때 쓴다. |
| `ok_token` | 승인 토큰 값 | 현재 `0xA5`다. slave2가 임의 데이터와 구분하려고 필요하다. |
| `status_frame_size` | status frame 길이 | 현재 8이다. |
| `ok_frame_size` | ok token frame 길이 | 현재 1이다. |
| `timeout_ticks` | LIN timeout tick 수 | 현재 100이다. |
| `poll_period_ms` | master polling 주기 | 현재 20ms다. master만 실질적으로 사용한다. |
| `binding` | 하드웨어 callback 묶음 | 위 `LinBinding`이 들어간다. |

master와 slave가 같은 구조체를 쓰되 `role`과 의미가 갈린다는 점이 이 설계의 핵심이다.

### `LinModule`
- 정의: `services/lin_module.h`
- 성격: LIN 상태기계 전체 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | 모듈 준비 완료 여부 | 진입 보호용이다. |
| `config` | 정적 LIN 설정 | role, PID, timeout, binding 전체를 품는다. |
| `state` | 현재 상태기계 상태 | 내부 구현상 `IDLE=0`, `WAIT_PID=1`, `WAIT_RX=2`, `WAIT_TX=3`이다. |
| `flags` | ISR/콜백 이벤트 비트들 | `PID_OK`, `RX_DONE`, `TX_DONE`, `ERROR` 비트를 누적해 fast task가 소비한다. |
| `current_pid` | 현재 처리 중인 PID | master/slave 모두 어떤 transaction인지 판단하려고 필요하다. |
| `last_poll_ms` | 마지막 poll 시각 | master의 20ms poll 주기를 맞추는 기준이다. |
| `ok_tx_pending` | master가 다음 poll에서 OK token을 보내야 하는지 | slave1 승인 요청이 들어오면 1이 된다. master 역할에서만 의미가 크다. |
| `ok_token_pending` | slave가 받은 OK token이 아직 소비되지 않았는지 | slave2가 token 수신 후 ADC latch clear 로직으로 넘기기 위해 필요하다. |
| `rx_buffer[8]` | HW 수신 버퍼 | status frame이나 OK token을 일단 담는다. |
| `tx_buffer[8]` | HW 송신 버퍼 | slave status frame, master OK token payload를 담는다. |
| `latest_status` | master가 최근에 받은 상태 | master 쪽에서 `ConsumeFreshStatus()`로 읽는다. |
| `slave_status_cache` | slave가 최근 ADC 상태를 캐시한 값 | slave2가 master poll에 응답할 때 이 값으로 frame을 만든다. |

중요한 포인트는 하나다. 같은 구조체지만 master에서는 `latest_status`와 `ok_tx_pending`이 중요하고, slave에서는 `slave_status_cache`와 `ok_token_pending`이 중요하다.

### `LinHwState`
- 정의: `drivers/lin_hw.c` 내부 구조체
- 성격: LIN 하드웨어 싱글톤 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `module` | 연결된 `LinModule` 포인터 | IsoSdk callback이 들어왔을 때 어느 모듈로 이벤트를 전달할지 알아야 한다. |
| `sdk_context` | SDK 어댑터 context | 아래 `IsoSdkLinContext`를 직접 품는다. |
| `role` | 현재 HW 역할 | `LinHw_Configure()`에서 master/slave 값이 들어간다. |
| `timeout_ticks` | 현재 timeout 값 | 초기값/갱신값을 SDK와 동기화하려고 둔다. 현재 기본은 100이다. |

`binding.context`를 `NULL`로 두고도 동작하는 이유가 바로 이 싱글톤 구조체 때문이다.

## 5. UART 계층 구조체

### `UartRxPendingRing`
- 정의: `S32K_LinCan_master/services/uart_types.h`
- 성격: ISR이 밀어 넣고 task가 꺼내는 RX 바이트 ring

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `buffer[32]` | pending RX 바이트 저장소 | ISR에서 line parsing까지 하면 부담이 커서, 일단 바이트만 여기에 쌓는다. |
| `head` | task가 읽을 위치 | ISR과 task가 공유하므로 `volatile`이다. |
| `tail` | ISR이 쓸 위치 | 동일한 이유로 `volatile`이다. |
| `overflow` | ring overflow 발생 여부 | task가 다음 cycle에 에러 처리하도록 신호를 남긴다. |
| `overflow_count` | overflow 누적 횟수 | 진단용이다. |

### `UartLineBuffer`
- 정의: `S32K_LinCan_master/services/uart_types.h`
- 성격: 현재 조립 중인 한 줄 명령

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `buffer[64]` | 현재 입력 줄 문자 저장 | 콘솔 명령 최대 길이는 64 기준이다. |
| `length` | 현재 문자열 길이 | backspace와 완성 line 복사 시 필요하다. |
| `line_ready` | 한 줄 완성 여부 | CR/LF를 만나면 1이 된다. |
| `overflow` | 줄 길이 초과 여부 | 너무 긴 명령을 오류 처리하려고 둔다. |

### `UartTxChunk`
- 정의: `S32K_LinCan_master/services/uart_types.h`
- 성격: TX queue에 들어가는 전송 조각

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `data[128]` | chunk 데이터 | 긴 화면 갱신 문자열을 고정 크기 조각으로 쪼개기 위해 필요하다. |
| `length` | 실제 길이 | 빈 영역까지 보내지 않게 하려는 값이다. |

### `UartTxContext`
- 정의: `S32K_LinCan_master/services/uart_types.h`
- 성격: UART 송신 상태 전체

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `current_buffer[129]` | 현재 HW에 걸려 있는 NUL 종료 문자열 | 디버깅 편의와 안전한 종료를 위해 1바이트 더 잡았다. |
| `current_length` | 현재 송신 중 길이 | driver status와 비교하려고 필요하다. |
| `busy` | 전송 중 여부 | 아직 끝나지 않았는지 판단한다. |
| `start_ms` | 현재 전송 시작 시각 | timeout 기준이다. |
| `timeout_ms` | 전송 timeout | 기본 100ms다. |
| `queue` | 대기 chunk queue | 비동기 문자열 전송을 위해 필요하다. |
| `queue_storage[8]` | queue 실제 저장소 | 최대 8개 chunk를 대기시킨다. |

### `UartService`
- 정의: `S32K_LinCan_master/services/uart_types.h`
- 성격: master 콘솔의 전체 UART 서비스 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | UART service 준비 여부 | Init 성공 후 1이 된다. |
| `instance` | UART 하드웨어 인스턴스 | `IsoSdk_UartGetDefaultInstance()` 값이 들어간다. literal 번호는 generated 설정에 있다. |
| `rx_byte` | ISR가 막 받은 1바이트 임시 저장소 | byte-by-byte receive API를 이어 붙이기 위해 필요하다. |
| `rx_pending` | ISR-task 사이 pending ring | low-level RX와 line parser를 분리하는 핵심이다. |
| `rx_line` | 현재 조립 중인 line | 콘솔 parser 입력이다. |
| `tx` | 송신 상태 전체 | queue, timeout, busy 상태를 함께 묶는다. |
| `error_flag` | 오류 발생 여부 | AppConsole이 에러 상태로 전환할 때 본다. |
| `error_count` | 오류 누적 횟수 | recover 전후 진단용이다. |
| `error_code` | 마지막 오류 종류 | HW init 실패, RX overflow, TX timeout 등을 구분한다. |

실무 관점에서 보면 `rx_pending`을 줄 단위와 분리한 건 좋다. 다만 현재 구현은 `ReadLine()`이 `rx_pending`까지 같이 reset해서, 줄 끝 직후에 이미 들어온 다음 바이트를 잃을 수 있다.

## 6. ADC / LED 계층 구조체

### `AdcConfig`
- 정의: `S32K_Lin_slave/services/adc_module.h`
- 성격: ADC 해석 모듈의 정적 설정
- 실제 값은 `runtime/runtime_io.c`에서 채운다.

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `init_fn` | ADC HW 초기화 함수 | 현재 `AdcHw_Init`이 들어간다. |
| `sample_fn` | 샘플링 함수 | 현재 `AdcHw_Sample`이 들어간다. |
| `hw_context` | HW 문맥 포인터 | 현재는 `NULL`이다. |
| `sample_period_ms` | 샘플 주기 | 현재 20ms다. |
| `range_max` | ADC 최대 범위 | 현재 4096이다. 12-bit ADC를 염두에 둔 값이다. |
| `safe_max` | safe 상한 | 현재 1365다. `4096 / 3`으로 계산한다. |
| `warning_max` | warning 상한 | 현재 2730이다. `4096 * 2 / 3`이다. |
| `emergency_min` | emergency 시작 임계값 | 현재 3413이다. `4096 * 5 / 6` 정수 계산 결과다. |
| `blocking_mode` | 샘플링 모드 힌트 | 현재 1이 들어가지만 실제 모듈 로직에서 거의 쓰이지 않는다. 설계 잔재 성격이 강하다. |

이 구조체는 공부하기 좋다. threshold를 코드 안에 박지 않고 config로 빼는 패턴이 잘 드러난다.

### `AdcSnapshot`
- 정의: `S32K_Lin_slave/services/adc_module.h`
- 성격: 최신 해석 ADC 상태 한 묶음

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `raw_value` | 최근 raw ADC 값 | 0~4095 범위로 clamp된 값이 들어간다. |
| `zone` | 의미 구간 | `SAFE`, `WARNING`, `DANGER`, `EMERGENCY` 중 하나다. |
| `emergency_latched` | emergency latch 상태 | 한 번 emergency면 zone이 내려가도 latch는 유지된다. 승인 token이 오고, zone이 emergency가 아닐 때만 해제된다. |
| `has_sample` | 정상 샘플 존재 여부 | 첫 샘플 이전 상태와 구분하려고 필요하다. |
| `sample_error` | 최근 샘플링 실패 여부 | ADC 읽기 실패를 상위에 알려 주기 위한 플래그다. |

### `AdcModule`
- 정의: `S32K_Lin_slave/services/adc_module.h`
- 성격: ADC 의미 해석 모듈 전체 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | 모듈 준비 완료 여부 | init 성공 후 1이다. |
| `config` | 정적 설정 | threshold와 callback 전체가 들어 있다. |
| `snapshot` | 최신 해석 결과 | app과 LIN 게시가 이 값을 본다. |
| `last_sample_ms` | 마지막 샘플 시각 | 20ms 주기 실행 여부를 판단하는 기준이다. |

### `LedConfig`
- 정의: `drivers/led_module.h`
- 성격: RGB LED 배선 정보
- 실제 값은 `board_hw -> isosdk_board_profile.h`에서 온다.

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `gpio_port` | LED가 연결된 GPIO 포트 | 현재 `PTD`가 들어간다. |
| `red_pin` | 빨강 핀 번호 | 현재 15다. |
| `green_pin` | 초록 핀 번호 | 현재 16이다. |
| `active_on_level` | 켜짐에 해당하는 논리 레벨 | 현재 0이다. 즉 active-low LED다. |

이 구조체를 둔 이유는 LED 로직이 `PTD/15/16` 같은 보드 세부사항을 직접 몰라도 되게 하기 위해서다.

### `LedModule`
- 정의: `drivers/led_module.h`
- 성격: 의미 기반 LED 출력 상태

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | 모듈 준비 완료 여부 | init 보호용이다. |
| `config` | 배선 설정 | 위 `LedConfig`가 복사되어 들어간다. |
| `pattern` | 현재 패턴 | `OFF`, `GREEN_SOLID`, `RED_SOLID`, `YELLOW_SOLID`, `RED_BLINK`, `GREEN_BLINK` 중 하나다. |
| `output_phase_on` | blink의 현재 on/off 위상 | blink 구현의 핵심 상태다. |
| `finite_blink_enabled` | 유한 횟수 blink인지 | slave1 승인 표시처럼 정해진 횟수만 깜빡일 때 1이 된다. |
| `blink_toggles_remaining` | 남은 토글 횟수 | slave1에서는 `APP_SLAVE1_ACK_TOGGLES = 6`이 들어간다. |

실무적으로도 이 구조체는 잘 만든 편이다. GPIO가 아니라 “의미 패턴” 중심으로 API를 열어 둔 점이 좋다.

## 7. Platform adapter 구조체

### `IsoSdkLinContext`
- 정의: `platform/s32k_sdk/isosdk_lin.h`
- 성격: vendor SDK와 portable LIN 사이의 어댑터 context

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | SDK 쪽 LIN 준비 완료 여부 | adapter 내부 호출 보호용이다. |
| `timeout_ticks` | 현재 timeout 설정 | runtime_io에서 100으로 시작한다. |
| `role` | master/slave 역할 | master 이미지면 1, slave2 이미지면 2가 들어간다. |
| `event_cb` | LIN 이벤트 콜백 | 현재 `LinHw_OnIsoSdkEvent`가 들어간다. |
| `event_context` | 콜백 문맥 | 현재 `NULL`이다. 실제 module 연결은 `LinHwState.module` 싱글톤이 들고 있다. |

### `IsoSdkAdcContext`
- 정의: `platform/s32k_sdk/isosdk_adc.h`
- 성격: ADC SDK 어댑터 context

| 필드 | 의미 | 왜 필요한가 / 실제 값 |
| --- | --- | --- |
| `initialized` | ADC SDK 초기화 여부 | 현재는 이 한 필드만 있다. 상위 `AdcModule`이 vendor 상태를 직접 몰라도 되게 하려는 최소 래퍼다. |

이 프로젝트에서 CAN/UART는 richer adapter struct를 따로 드러내지 않고, generated SDK 상태를 내부로 숨겼다. LIN/ADC만 비교적 얇은 context를 공개하고 있다.

## 8. 구조체 품질 메모

구조체를 공부할 때 아래 항목은 꼭 눈여겨보면 좋다.

- `default_target_node_id`
  - `CanModuleConfig`, `CanModule`에 있지만 현재 실사용이 거의 없다.
  - 설계 초안의 흔적이 남은 필드로 보인다.
- `blocking_mode`
  - `AdcConfig`에 들어가지만 현재 `AdcModule` 로직에서는 사실상 소비하지 않는다.
- `target_is_broadcast`
  - `AppConsoleCanCommand`에 저장되지만 현재 처리 경로에서는 크게 의미가 없다.
  - `target_node_id == 255`로도 충분하다.
- `CAN_SERVICE_RESULT_SEND_FAIL`
  - enum은 있는데 실제 결과 구조체로 생성되는 경로는 없다.
- master `AppCore`의 문자열 필드들
  - 학습용/시연용으로는 이해하기 쉽다.
  - 실무에서는 표시용 상태를 별도 view-model로 빼는 경우가 많다.

## 9. 이 문서를 어떻게 공부하면 좋은가

추천 순서는 아래와 같다.

1. `InfraQueue`, `RuntimeTaskEntry`, `RuntimeContext`
2. `AppCore` 세 버전 비교
3. `CanMessage -> CanService -> CanModule -> CanHw`
4. `LinConfig -> LinModule -> LinStatusFrame`
5. `AdcConfig -> AdcSnapshot -> AdcModule`
6. `LedConfig -> LedModule`
7. `AppConsole`, `UartService`

이 순서로 보면 “공통 런타임 -> 앱 상태 -> 통신 -> 센서 -> UI” 흐름이 자연스럽게 이어진다.

## 10. 한 줄 총평

이 저장소의 구조체 설계는 전반적으로 “학습하기 좋게 역할을 분리한 편”이다.
특히 `Config`, `Module`, `Snapshot`, `Queue`를 따로 둔 점이 좋다.
다만 일부 필드는 아직 사용성이 약하거나 이름이 애매해서, 공부할 때는 “왜 남아 있는지”까지 같이 비판적으로 보는 게 도움이 된다.
