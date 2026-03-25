# S32K 레이어 분리 다이어그램

이 문서는 `re_5`의 세 프로젝트를 기준으로, 함수 호출 세부보다 한 단계 위에서
레이어가 어떻게 분리되어 있는지 보이도록 정리한 문서다.

보조 자료:
- [s32k_task_layer_maps.md](./s32k_task_layer_maps.md)
- [s32k_project_function_variable_outline.md](./s32k_project_function_variable_outline.md)

## 공통 레이어 구조

```mermaid
flowchart TD
  MAIN["main.c<br/>프로그램 시작점"]
  RT["runtime<br/>스케줄링 / task orchestration / project binding"]
  APP["app<br/>프로젝트 정책 / 상태 전이 / 문자열 상태"]
  SVC["services<br/>프로토콜 / 도메인 로직 / 상태기계"]
  DRV["drivers<br/>보드 독립에 가까운 HW adapter"]
  PLT["platform/s32k_sdk<br/>S32K SDK wrapper"]
  VND["platform/generated + NXP SDK<br/>실제 벤더 드라이버"]
  CORE["core<br/>queue / tick / task 유틸리티"]

  MAIN --> RT --> APP --> SVC --> DRV --> PLT --> VND
  RT --> CORE
  APP --> CORE
  SVC --> CORE

  CORE -. tick hook .-> APP
  PLT -. callback / IRQ .-> DRV
  DRV -. event notify / function pointer .-> SVC
```

### re_5에서 특히 봐야 할 분리 포인트

- `RuntimeTick_RegisterHook(AppCore_OnTickIsr)`로 `core -> app` 역방향 호출이 있다.
- `LinModule`은 function pointer binding으로 `services -> drivers` 의존을 느슨하게 만든다.
- UART RX는 `platform -> drivers/uart_hw -> services/uart_service`로 callback이 올라온다.
- CAN RX는 `re_5`에서 `platform/s32k_sdk/isosdk_can -> drivers/can_hw` callback이 먼저 들어오고, 이후 task 문맥에서 service가 queue를 drain한다.

---

## S32K_Can_slave 레이어 분리

```mermaid
flowchart TD
  subgraph L1["S32K_Can_slave"]
    direction TB

    subgraph M1["Main"]
      MAIN1["main.c"]
    end

    subgraph R1["Runtime Layer"]
      RT1["runtime/runtime.c"]
      RIO1["runtime/runtime_io.c"]
    end

    subgraph C1["Core Layer"]
      CTASK1["core/runtime_task.c"]
      CTICK1["core/runtime_tick.c"]
      CQUEUE1["core/infra_queue.c"]
    end

    subgraph A1["App Layer"]
      AC1["app/app_core.c"]
      AS1["app/app_slave1.c"]
    end

    subgraph S1["Service Layer"]
      SM1["services/can_module.c"]
      SS1["services/can_service.c"]
      SP1["services/can_proto.c"]
    end

    subgraph D1["Driver Layer"]
      DB1["drivers/board_hw.c"]
      DC1["drivers/can_hw.c"]
      DL1["drivers/led_module.c"]
      DT1["drivers/tick_hw.c"]
    end

    subgraph P1["Platform Layer"]
      PB1["platform/s32k_sdk/isosdk_board.c"]
      PC1["platform/s32k_sdk/isosdk_can.c"]
      PT1["platform/s32k_sdk/isosdk_tick.c"]
    end
  end

  MAIN1 --> RT1

  RT1 --> AC1
  RT1 --> CTASK1
  RT1 --> CTICK1
  RT1 --> RIO1

  RIO1 --> DB1
  CTICK1 --> DT1

  AC1 --> AS1
  AC1 --> SM1
  AC1 --> RIO1

  AS1 --> DL1
  AS1 --> RIO1

  SM1 --> SS1
  SM1 --> CQUEUE1
  SS1 --> SP1
  SS1 --> DC1

  DB1 --> PB1
  DC1 --> PC1
  DL1 --> PB1
  DT1 --> PT1

  PC1 -. FlexCAN callback .-> DC1
```

### 이 프로젝트에서 레이어가 나뉘는 느낌

- `app_core.c`, `app_slave1.c`가 정책 레이어다.
- CAN protocol/transport는 `services`에 있고, 실제 mailbox 처리와 callback 수신은 `drivers/can_hw.c`가 맡는다.
- `re_5`에서는 CAN RX 완료가 `platform -> driver` callback으로 먼저 올라온다는 점이 `re_4`와 다르다.
- `runtime_io.c`는 프로젝트 전용 보드 binding을 묶어 주는 조립점이다.

---

## S32K_LinCan_master 레이어 분리

```mermaid
flowchart TD
  subgraph L2["S32K_LinCan_master"]
    direction TB

    subgraph M2["Main"]
      MAIN2["main.c"]
    end

    subgraph R2["Runtime Layer"]
      RT2["runtime/runtime.c"]
      RIO2["runtime/runtime_io.c"]
    end

    subgraph C2["Core Layer"]
      CTASK2["core/runtime_task.c"]
      CTICK2["core/runtime_tick.c"]
      CQUEUE2["core/infra_queue.c"]
    end

    subgraph A2["App Layer"]
      AC2["app/app_core.c"]
      AM2["app/app_master.c"]
      ACON2["app/app_console.c"]
    end

    subgraph S2["Service Layer"]
      SCM2["services/can_module.c"]
      SCS2["services/can_service.c"]
      SCP2["services/can_proto.c"]
      SL2["services/lin_module.c"]
      SU2["services/uart_service.c"]
    end

    subgraph D2["Driver Layer"]
      DB2["drivers/board_hw.c"]
      DC2["drivers/can_hw.c"]
      DLIN2["drivers/lin_hw.c"]
      DU2["drivers/uart_hw.c"]
      DT2["drivers/tick_hw.c"]
    end

    subgraph P2["Platform Layer"]
      PB2["platform/s32k_sdk/isosdk_board.c"]
      PC2["platform/s32k_sdk/isosdk_can.c"]
      PLIN2["platform/s32k_sdk/isosdk_lin.c"]
      PU2["platform/s32k_sdk/isosdk_uart.c"]
      PT2["platform/s32k_sdk/isosdk_tick.c"]
    end
  end

  MAIN2 --> RT2

  RT2 --> AC2
  RT2 --> RIO2
  RT2 --> CTASK2
  RT2 --> CTICK2

  AC2 --> AM2
  AC2 --> ACON2
  AC2 --> SCM2
  AC2 --> SL2

  AM2 --> SL2
  AM2 --> AC2
  ACON2 --> SU2
  ACON2 --> CQUEUE2

  SCM2 --> SCS2
  SCM2 --> CQUEUE2
  SCS2 --> SCP2
  SCS2 --> DC2
  SL2 --> DLIN2
  SU2 --> DU2
  SU2 --> CQUEUE2

  RIO2 --> DB2
  RIO2 --> DLIN2

  CTICK2 --> DT2

  DB2 --> PB2
  DC2 --> PC2
  DLIN2 --> PLIN2
  DU2 --> PU2
  DT2 --> PT2

  CTICK2 -. tick hook .-> AC2
  PLIN2 -. LIN callback .-> DLIN2
  DLIN2 -. event notify .-> SL2
  PU2 -. UART RX callback .-> DU2
  DU2 -. byte/event 전달 .-> SU2
  PC2 -. FlexCAN callback .-> DC2
```

### 이 프로젝트에서 레이어가 나뉘는 느낌

- `app_core.c`가 전체 orchestration 중심이다.
- `app_master.c`는 정책 판단, `app_console.c`는 operator UI, `services`는 실제 protocol/state machine을 맡는다.
- LIN/UART뿐 아니라 CAN도 callback 보조 경로가 있어 역방향 흐름이 가장 많다.
- `runtime_io.c`는 master 전용 LIN binding과 보드 조립을 맡는다.

---

## S32K_Lin_slave 레이어 분리

```mermaid
flowchart TD
  subgraph L3["S32K_Lin_slave"]
    direction TB

    subgraph M3["Main"]
      MAIN3["main.c"]
    end

    subgraph R3["Runtime Layer"]
      RT3["runtime/runtime.c"]
      RIO3["runtime/runtime_io.c"]
    end

    subgraph C3["Core Layer"]
      CTASK3["core/runtime_task.c"]
      CTICK3["core/runtime_tick.c"]
      CQUEUE3["core/infra_queue.c"]
    end

    subgraph A3["App Layer"]
      AC3["app/app_core.c"]
      AS3["app/app_slave2.c"]
    end

    subgraph S3["Service Layer"]
      SA3["services/adc_module.c"]
      SL3["services/lin_module.c"]
    end

    subgraph D3["Driver Layer"]
      DB3["drivers/board_hw.c"]
      DA3["drivers/adc_hw.c"]
      DLIN3["drivers/lin_hw.c"]
      DL3["drivers/led_module.c"]
      DT3["drivers/tick_hw.c"]
    end

    subgraph P3["Platform Layer"]
      PB3["platform/s32k_sdk/isosdk_board.c"]
      PA3["platform/s32k_sdk/isosdk_adc.c"]
      PLIN3["platform/s32k_sdk/isosdk_lin.c"]
      PT3["platform/s32k_sdk/isosdk_tick.c"]
    end
  end

  MAIN3 --> RT3

  RT3 --> AC3
  RT3 --> RIO3
  RT3 --> CTASK3
  RT3 --> CTICK3

  AC3 --> AS3
  AS3 --> SA3
  AS3 --> SL3

  RIO3 --> DB3
  RIO3 --> DA3
  RIO3 --> DLIN3

  SA3 --> DA3
  AS3 --> DL3

  SL3 --> DLIN3
  CTICK3 --> DT3

  DB3 --> PB3
  DA3 --> PA3
  DLIN3 --> PLIN3
  DL3 --> PB3
  DT3 --> PT3

  CTICK3 -. tick hook .-> AC3
  PLIN3 -. LIN callback .-> DLIN3
  DLIN3 -. event notify .-> SL3
```

### 이 프로젝트에서 레이어가 나뉘는 느낌

- 센서 해석은 `services/adc_module.c`, LIN protocol은 `services/lin_module.c`로 분리되어 있다.
- `app/app_slave2.c`는 두 service를 조합해서 정책을 만든다.
- tick hook과 LIN callback이 있어 단순해 보여도 역방향 흐름은 분명하다.

---

## 빠른 비교

- `S32K_Can_slave`
  - 가장 단순한 구조지만 `re_5`에서는 CAN RX callback 경로가 분명히 존재한다.

- `S32K_LinCan_master`
  - 레이어 수는 같지만 callback 종류가 가장 많아서 실제 구조는 가장 입체적이다.

- `S32K_Lin_slave`
  - ADC + LIN 두 축만 보면 되어서 구조를 읽기 가장 쉽다.
