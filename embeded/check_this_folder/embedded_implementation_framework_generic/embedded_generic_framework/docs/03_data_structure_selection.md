# 03. Data Structure Selection

## 1) Ring Buffer
적합:
- UART RX/TX
- byte stream
- single producer / single consumer

장점:
- O(1)
- 고정 크기
- 구현 단순

주의:
- overwrite/drop 정책 명시
- 길이 기반 packet이면 framing 로직 별도 필요

## 2) Fixed-size Message Queue
적합:
- event/message 전달
- ISR -> task deferred work
- protocol frame queue

장점:
- 메시지 경계 보존
- 큐 꽉 참 처리 명확

주의:
- message size가 크면 메모리 낭비
- slot 수 부족 시 드롭 정책 필요

## 3) Fixed Pool / Slab
적합:
- variable ownership를 동적할당 없이 관리
- frame, transaction, request object 재사용

장점:
- malloc-free 회피
- 할당/반납 시간 예측 가능

주의:
- leak 탐지 카운터 필요
- double free 방지 규칙 필요

## 4) Bitmap
적합:
- resource allocation
- flag set
- ready masks
- channel availability

장점:
- 메모리 효율 높음

주의:
- 동시 수정 시 atomic 고려

## 5) Intrusive Singly Linked List
적합:
- free list
- deferred work list
- timer bucket chain

장점:
- 별도 노드 할당 불필요
- 오버헤드 낮음

주의:
- ownership 규칙이 흐려지기 쉬움
- 중복 연결 방지 필요

## 6) Static Vector
적합:
- 작은 고정 용량 테이블
- runtime registration
- command table
- filter table

장점:
- 배열 기반이라 캐시/분석 유리
- 구현 쉬움

주의:
- remove 시 압축 비용
- 용량 초과 처리 필요

## 7) Timer Wheel / Delta List
적합:
- 다수 timeout 관리
- protocol timer
- retry/cancel/expiry

장점:
- 주기 tick에서 효율적

주의:
- 해상도와 비용 trade-off
- tick jitter 문서화 필요

## 8) Priority Queue / Min-Heap
적합:
- deadline order dispatch
- scheduler
- timed jobs

장점:
- 가장 이른 deadline 탐색 용이

주의:
- 구현 복잡도 증가
- 일반 MCU 초급 프로젝트에 과할 수 있음

## 정석 선택 순서

초급자가 처음 잡을 때는 아래 순서가 가장 무난하다.

1. ring buffer
2. fixed message queue
3. fixed pool
4. bitmap
5. static vector
6. timer wheel
7. intrusive list

priority queue, tree, hash table은 정말 필요한 경우만 도입한다.
