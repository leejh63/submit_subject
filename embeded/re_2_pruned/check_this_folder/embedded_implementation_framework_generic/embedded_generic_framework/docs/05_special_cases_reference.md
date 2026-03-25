# 05. Special Cases Reference

이 문서는 일부러 템플릿을 제공하지 않는다.
아래 분야는 일반 임베디드 골격 위에 **별도 전문 규격**이 필요하다.

## 1) Motor Control / Power Electronics
- complementary PWM
- deadtime
- fault shutdown path
- current loop / speed loop / position loop
- ADC sampling phase alignment

## 2) Functional Safety / Certified Systems
- freedom from interference
- tool qualification
- traceability
- diagnostic coverage
- latent fault metrics

## 3) Bootloader / Secure Update / OTA
- A/B bank
- rollback
- image authentication
- fail-safe boot
- interrupted update recovery

## 4) High-speed Connectivity
- USB
- Ethernet
- BLE / Wi-Fi
- TCP/IP stack
- buffer ownership / zero-copy / cache coherence

## 5) Storage / Filesystem
- wear leveling
- journaling
- power fail recovery
- bad block handling

## 6) RTOS-heavy Multi-task Systems
- priority inversion
- lock hierarchy
- per-task stack sizing
- CPU load budget
- tracing / profiling

초급자는 위 분야를 처음부터 템플릿으로 복붙하려 하지 말고,
기본 골격 + 해당 분야 전문 자료를 결합해서 들어가는 편이 훨씬 안전하다.
