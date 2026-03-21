# 실제 적용 순서

1. 네 기존 LIN 슬레이브 프로젝트에 `platform/`, `hal/`, `drivers/`, `services/`, `app/`, `main/` 디렉터리 추가
2. `platform/s32k_bindings.h`가 현재 generated 심볼과 맞는지 확인
3. `main.c`를 `main/main_lin_sensor_slave.c`로 교체
4. 컴파일 에러 정리
   - include path
   - GPIO_Type, status_t, lin_state_t 등 SDK 타입 확인
   - LPTMR ISR 이름 충돌 확인
5. LIN 슬레이브 단독 동작 검증
6. 이후 master project에 `app_master_node.*`, `drv_can.*`, `drv_uart.*`, `hal_s32k_can.*`, `hal_s32k_uart.*` 이식
7. 마지막으로 button slave project에 `app_can_button_slave.*` 이식

## 주의

- 지금 패키지의 CAN/UART 쪽은 실제 generated 이름이 없어서 템플릿 상태다.
- LIN 센서 슬레이브 쪽이 우선 적용 대상이다.
- master/button 으로 갈수록 protocol/service 문서부터 먼저 고정해야 한다.
