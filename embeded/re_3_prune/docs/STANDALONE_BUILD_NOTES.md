# Standalone Build Notes

## Current State

`re_3_prune/projects/*` 아래의 세 프로젝트는 각각 독립 빌드를 염두에 두고 재배치되었습니다.
즉, 보드별 폴더를 따로 들고 가도 소스 구조 자체는 닫히도록 정리했습니다.

예시 구조:
- `app/`: 프로젝트별 use-case, 상태 조립
- `runtime/`: 부팅, task loop, 프로젝트 binding
- `core/`: queue, types, scheduler, tick runtime
- `drivers/`: can/uart/lin/adc/board/tick 어댑터
- `services/`: 프로토콜, 센서, console 서비스
- `platform/s32k_sdk/`: SDK generated 심볼과 raw SDK 격리층
- `platform/generated/`: 보드별 generated config 위치
- `vendor/drivers/inc/`: 일부 vendor driver header

## Which Folders Are Authoritative

독립 프로젝트 기준으로는 아래 3개 폴더가 실제 대상입니다.

- `projects/S32K_Can_slave`
- `projects/S32K_Lin_slave`
- `projects/S32K_LinCan_master`

루트에 남아 있는 `shared/`, `platform/`, `vendor/`는 재구성 작업 중 생성된 staging 성격이므로,
나중에 프로젝트를 개별로 떼어낼 때는 기준으로 삼지 않는 편이 좋습니다.

## What Still Blocks Real Compilation

현재 저장소에는 아래 파일이 없습니다.

Generated file:
- `sdk_project_config.h`

NXP SDK base headers:
- `status.h`
- `device_registers.h`
- `osif.h`
- `callbacks.h`

즉, 소스 계층은 정리됐지만 실제 컴파일을 위해서는 각 보드 프로젝트별 IDE/Config Tools 산출물과 SDK 기본 패키지가 필요합니다.

## Recommended Build Inputs Per Board

각 프로젝트 폴더 기준으로 아래를 준비하면 됩니다.

1. `platform/generated/sdk_project_config.h`
2. 필요 시 해당 generated `.c` 파일들
3. NXP SDK base include path
4. `vendor/drivers/inc` include path
5. `platform/s32k_sdk` include path
6. 프로젝트 루트 include path

## Recommended Source Groups Per Board

각 보드 프로젝트를 별도 IDE 프로젝트로 만들 때는 아래 묶음을 기준으로 가져가면 됩니다.

1. `main.c`
2. `app/*.c`
3. `runtime/*.c`
4. `core/*.c`
5. `drivers/*.c`
6. `services/*.c`
7. `platform/s32k_sdk/*.c`
8. generated source if present

## Architectural Intent

현재 구조의 의도는 아래와 같습니다.

- `runtime_io`는 더 이상 SDK를 직접 부르지 않습니다.
- raw SDK 호출은 `drivers/*` 아래 adapter와 `platform/s32k_sdk/*`에만 남깁니다.
- `app`과 `services`는 가능한 한 의미 중심 로직만 가지도록 유지합니다.
- 이후 프로젝트를 분리하더라도 레이어 의미가 그대로 유지되도록 물리 구조도 프로젝트 내부에 함께 넣었습니다.

이 구조는 작은 임베디드 포트폴리오에서 과분리 없이도 계층 의도를 설명하기 쉬운 형태를 목표로 합니다.
