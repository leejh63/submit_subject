# re_2_pruned

원본 `re_2`는 그대로 두고, 노드별로 실제 사용하는 코드만 남긴 슬림 버전이다.

- `S32K_LinCan_master`: master coordinator 전용
- `S32K_Lin_slave`: LIN sensor slave 전용
- `S32K_Can_slave`: CAN reaction slave 전용

정리 원칙은 다음과 같다.

- 다른 노드 정책 파일(`app_master`, `app_slave1`, `app_slave2`)은 각 프로젝트에서 필요한 것만 남김
- `app_core`, `runtime`, `runtime_io`는 역할 전용으로 다시 작성해 no-op 분기 제거
- console, CAN, LIN, ADC, LED, UART 모듈은 해당 노드가 실제 사용하는 경우에만 복제
- 원본 프로젝트는 수정하지 않음

빌드 시스템 파일은 원본 저장소에도 보이지 않아 함께 옮기지 않았다.
따라서 이 폴더는 소스 구조 정리본이며, 실제 IDE/프로젝트 설정 연결은 별도로 맞춰야 한다.
