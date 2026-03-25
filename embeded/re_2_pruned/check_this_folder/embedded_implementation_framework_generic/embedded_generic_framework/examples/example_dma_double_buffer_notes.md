# DMA Double Buffer Pattern

DMA를 붙일 때 초급자가 가장 많이 놓치는 건 **버퍼 ownership**이다.

권장 흐름:

1. 버퍼 A/B 두 개를 둔다.
2. DMA가 현재 채우는 버퍼와 CPU가 처리하는 버퍼를 분리한다.
3. half/full complete 또는 ping-pong complete에서 active buffer만 교환한다.
4. ISR에서는 index 전환 + event post만 한다.
5. 실제 filter/convert/pack은 main/service 문맥에서 한다.

체크리스트:
- cache invalidate/clean 필요한가?
- alignment 제약 있는가?
- circular mode인가?
- drop 시 정책은 무엇인가?
- frame boundary를 DMA가 보장하는가?
