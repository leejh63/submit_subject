# re_3_prune

중요 기준:
- 실제 독립 프로젝트는 `projects/S32K_Can_slave`, `projects/S32K_Lin_slave`, `projects/S32K_LinCan_master` 입니다.
- 각 프로젝트 내부에 `app`, `runtime`, `core`, `drivers`, `services`, `platform`, `vendor`를 함께 넣어 두어 나중에 폴더 단위로 분리하기 쉽게 맞췄습니다.
- `platform/s32k_sdk`는 raw SDK 격리층입니다.
- `drivers`는 주변장치 어댑터 계층입니다.
- `services`는 재사용 가능한 기능/프로토콜 계층입니다.
- `runtime`과 `app`은 프로젝트별 조립 계층입니다.

주의:
- 루트에 남아 있는 `shared/`, `platform/`, `vendor/` 폴더는 재구성 과정의 staging 성격입니다.
- 이후 보드별 독립 빌드 기준으로 볼 때는 `projects/*` 아래 각 폴더를 기준 구조로 보면 됩니다.

현재 저장소에는 `sdk_project_config.h`와 일부 NXP SDK 기본 헤더가 없어서 즉시 컴파일되지는 않습니다.
자세한 내용은 `docs/STANDALONE_BUILD_NOTES.md`와 각 프로젝트 폴더의 `README.md`를 참고하세요.
