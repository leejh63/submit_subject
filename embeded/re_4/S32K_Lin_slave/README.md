# S32K_Lin_slave

이 폴더는 보드별 독립 빌드를 전제로 정리한 self-contained 프로젝트 폴더입니다.
나중에 이 폴더만 별도 저장소나 IDE 프로젝트로 옮겨도, 소스 구조 자체는 그대로 유지되도록 맞췄습니다.

## Directory Intent

- `app/`: LIN slave use-case와 상태 조립
- `runtime/`: 부팅 순서, task loop, project binding
- `core/`: queue, types, runtime tick 같은 공통 기반
- `drivers/`: board/lin/adc/tick/led adapter
- `services/`: LIN module, ADC module
- `platform/s32k_sdk/`: raw SDK 격리층
- `platform/generated/`: 보드별 generated config 입력 위치
- `vendor/drivers/inc/`: vendor header

## Required External Inputs

실제 빌드를 하려면 아래가 추가로 필요합니다.

- `platform/generated/sdk_project_config.h`
- 필요 시 SDK Config Tools가 생성한 관련 `.c` 파일
- NXP SDK base headers: `status.h`, `device_registers.h`, `osif.h`, `callbacks.h`

## Build Direction

권장 include path 기준:

- 프로젝트 루트
- `platform/generated`
- `platform/s32k_sdk`
- `vendor/drivers/inc`

권장 소스 묶음 기준:

- `main.c`
- `app/*.c`
- `runtime/*.c`
- `core/*.c`
- `drivers/*.c`
- `services/*.c`
- `platform/s32k_sdk/*.c`
- generated source if present
