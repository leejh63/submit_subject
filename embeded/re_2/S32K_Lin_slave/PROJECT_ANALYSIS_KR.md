# S32K_Lin_slave 프로젝트 분석

## 한 줄 요약
- 이 프로젝트는 센서 상태 제공 노드입니다.
- ADC를 읽어서 위험도를 판단하고, 그 결과를 LIN으로 master에 전달합니다.

## 이 프로젝트를 처음 볼 때 핵심만 먼저
- 역할: slave2
- node id: `3`
- 직접 쓰는 핵심 모듈:
  - `runtime`
  - `infra`
  - `adc`
  - `lin`
  - `led`
  - `app_slave2`
  - `runtime_io`의 ADC/LIN binding

## 시작 흐름
1. `main.c`에서 `Runtime_Init()`
2. `AppCore_Init()`가 현재 역할이 slave2임을 확인
3. `AppSlave2_Init()` 호출
4. LED 초기화
5. ADC 모듈 초기화
6. LIN slave 모듈 초기화
7. 이후 periodic task로 ADC, LIN fast, LED를 돌림

## 이 프로젝트에서 중요한 파일
- `src/app/app_slave2.c`
  - 실제 정책 핵심
- `src/adc/adc_module.c`
  - ADC 샘플 해석
- `src/lin/lin_module.c`
  - LIN slave 상태기계
- `src/runtime/runtime_io.c`
  - ADC/LIN generated binding
- `src/led/led_module.c`
  - 상태 표시 LED

## 실제 동작 시나리오

### 주기적인 센서 샘플링
- ADC task가 주기적으로 raw ADC 값을 읽습니다.
- 그 값을 기준 threshold와 비교해 zone을 판정합니다.
  - safe
  - warning
  - danger
  - emergency

### emergency latch
- 한번 emergency zone에 들어가면 `emergency_latched`가 1이 됩니다.
- 이후 ADC 값이 내려가도 바로 완전히 해제되지 않습니다.
- 이 latch는 master의 명시적 OK token이 들어와야 풀 수 있습니다.

### LIN status 제공
- slave2는 자신의 최신 ADC 상태를 `LinStatusFrame`으로 cache합니다.
- master가 status PID를 보내면, LIN slave가 이 cache를 응답 frame으로 내보냅니다.

### OK token 수신
- master가 OK PID로 token을 보내면 slave2가 이를 받습니다.
- token 값이 맞으면 `ok_token_pending`을 세웁니다.
- app은 이 신호를 보고 `AdcModule_ClearEmergencyLatch()`를 호출합니다.
- 단, 현재 zone이 아직 emergency라면 latch는 지워지지 않습니다.

## task별로 보면

### `ADC task`
- `AdcModule_Task()` 수행
- 새 스냅샷 획득
- 화면용 ADC 문자열 갱신
- LIN slave status cache 갱신
- LED 상태 갱신

### `LIN fast task`
- LIN callback에서 들어온 event를 상태기계로 정리합니다.
- OK token이 들어왔는지 확인하고, 들어왔다면 latch clear를 시도합니다.

### `LED task`
- latch가 걸려 있으면 빨강 점멸
- safe면 초록 고정
- warning이면 노랑 고정
- danger면 빨강 고정

## 이 프로젝트에서 중요한 데이터

### `AdcConfig`
- ADC 하드웨어 read 함수와 threshold를 함께 담습니다.
- 중요한 값:
  - `sample_period_ms`
  - `range_max`
  - `safe_max`
  - `warning_max`
  - `emergency_min`

### `AdcSnapshot`
- 현재 ADC 해석 결과입니다.
- `raw_value`
  - 실제 읽은 값
- `zone`
  - safe/warning/danger/emergency
- `emergency_latched`
  - 긴급상태 잠금 여부

### `LinStatusFrame`
- master에게 보여줄 요약 데이터입니다.
- slave2의 핵심 출력 데이터라고 보면 됩니다.

### `LinModule.ok_token_pending`
- OK token이 막 들어왔음을 의미합니다.
- app이 소비하면 0으로 내려갑니다.

## 왜 이 구조가 좋은가
- ADC raw read와 정책 해석이 분리되어 있습니다.
- LIN 통신과 ADC 판단 로직이 섞이지 않습니다.
- app은 "ADC를 읽어 zone을 판단하고 LIN status로 publish하라"만 표현합니다.
- 실제 ADC SDK 호출과 LIN SDK callback은 `runtime_io.c`에 숨겨져 있습니다.

## LIN slave 관점에서 이해하기
- slave2는 poll을 시작하지 않습니다.
- master가 헤더를 보내면 거기에 맞춰 응답합니다.
- status PID면 자신의 상태를 보냅니다.
- OK PID면 1바이트 token을 받습니다.
- 즉 이 프로젝트의 LIN은 "수동 응답형"에 가깝습니다.

## 이 프로젝트를 디버깅할 때 보는 순서
1. `adc_module.c`에서 threshold와 latch 동작 확인
2. `app_slave2.c`에서 snapshot -> LIN status 매핑 확인
3. `lin_module.c`에서 slave PID 처리 흐름 확인
4. `runtime_io.c`에서 ADC/LIN binding과 timeout service 확인

## 평가

### 장점
- 센서 처리와 통신 처리의 경계가 깔끔합니다.
- emergency latch라는 제품 의미가 코드에서 잘 드러납니다.
- master 승인 절차와 연결되는 구조가 자연스럽습니다.

### 참고할 점
- 현재는 고정 baud slave 방향으로 구성되어 있습니다.
- LIN timing/timeout 관련 실제 동작은 `runtime_io.c`의 binding과 base tick에 의존합니다.
