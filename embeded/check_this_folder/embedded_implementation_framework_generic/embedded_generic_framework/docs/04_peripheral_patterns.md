# 04. Peripheral Patterns

## GPIO
- 입력은 debounce 정책이 먼저다.
- 출력은 active high/low를 숨긴 래퍼를 둬라.
- 앱이 pin number를 직접 아는 구조를 피하라.

## ADC
- 샘플링보다 먼저 기준전압/분해능/샘플링 타임이 맞는지 확인한다.
- raw 값을 앱까지 그대로 올리지 말고 service에서 filter/convert/plausibility를 처리한다.
- polling보다 timer trigger + DMA + deferred processing 조합이 더 확장성이 좋다.

## TIMER / TIMEBASE
- `now - start` 방식 비교를 써라.
- wrap-around를 허용하는 비교 유틸을 반드시 둬라.
- 여러 모듈이 제각각 tick 계산을 만들지 못하게 공용 함수로 묶어라.

## PWM / FTM
- duty 변경은 가능한 한 shadow register / update point를 활용해 glitch를 줄여라.
- 0% / 100% 극단 동작의 하드웨어 의미를 먼저 확인하라.
- 모터/액추에이터 계열은 safe-duty와 startup/ramp 정책을 별도로 둬라.

## DMA
- DMA는 CPU 부담을 줄이지만 디버깅 난이도를 높인다.
- source/destination lifetime과 ownership을 명확히 하라.
- circular mode, half/full callback, cache coherence, alignment를 반드시 문서화하라.

## UART
- raw stream, line mode, framed packet을 섞지 마라.
- idle detection, timeout, delimiter, length field 중 무엇을 쓰는지 먼저 고정하라.
- RX overrun/ framing error/ noise error 카운터를 남겨라.

## SPI
- 버스 공유 시 chip-select와 transaction boundary를 service가 소유하게 하라.
- TX-only처럼 보여도 실제로는 dummy RX 처리 필요할 수 있다.
- DMA 전환 시 버퍼 정렬/수명 주기를 다시 검토하라.

## I2C
- stuck bus recovery 없으면 현장에서 한 번 꼬였을 때 답이 없다.
- device driver와 bus driver를 분리하라.
- timeout과 retry는 무한이 아니라 제한 횟수여야 한다.

## CAN
- ID 체계는 문서가 먼저다. 코드가 먼저면 망한다.
- mailbox/queue ownership을 분명히 하라.
- request-response, event, diagnostic traffic을 섞으면 복잡도가 급증한다.

## LIN
- role(master/slave), schedule table, frame ownership을 분명히 하라.
- sleep/wakeup, checksum, unconditional/sporadic/event-triggered frame 구분을 문서화하라.
- LIN은 “느리다”가 아니라 “스케줄이 본질”이다.

## FLASH / NVM
- 단일 구조체 덮어쓰기보다 version + crc + commit marker를 둬라.
- 전원 차단 중간 실패를 전제로 설계하라.
- erase cycle budget을 모르고 쓰면 유지보수 단계에서 바로 터진다.
